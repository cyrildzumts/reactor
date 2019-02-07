#ifndef CIRCUITBREAKER_H
#define CIRCUITBREAKER_H
#include "common.h"
#include "function_wrapper.h"
#include "service.h"
#include "activeobject.h"
#include <threadpool.h>
#include <utility>
#include <functional>
#include <type_traits>
#include <optional>
#include <chrono>
#include <future>
#include <memory>

/******************************************************************
 * Author : Cyrille Ngassam Nkwenga
 * Year : 2019
 * This header contains the implementation of the circuit breaker
 * pattern.
 ****************************************************************/

/**
 * @brief The TimeoutError class An Exception thrown when the Service
 * doesn't return a response in the deadline.
 */
class TimeoutError : public std::runtime_error{
public:
    TimeoutError();
    // Exception Interface
    virtual const char* what() const noexcept;

private:
    std::string what_string;

};


template<typename R, typename...Args>
class Command{
public:
    using result_type = std::decay_t<R>;
    using function_type = R(*)(Args...);

    Command(function_type op) : fn{op}{

    }

    Command(const Command &other){
        this->fn = other.fn;
    }
    //Command &operator=(const Command& other) = default;

    Command(Command&& other){
        this->fn = other.fn;
    }

    Command &operator=(const Command& other){
        this->fn = other.fn;
    }

    Command &operator=(Command&& other){
        this->fn = other.fn;
    }

    result_type operator()(Args&&... args){
        return fn(std::forward<Args>(args)...);
    }

private:
    function_type fn;

};

enum class State {
    CLOSED, OPEN, HALF_OPEN
};

template<typename R, typename... Args>
class CircuitBreaker
{
public:
    /**
     * this template decuces the result type of the submited request.
     * It help us to built the right std::future by using calling
     * std::future<result_type> to return the result of the request.
     */
    //using R =std::invoke_result_t<std::decay_t<Callable>, std::decay_t<Args>...>;
    //using result_type = std::invoke_result_t<std::decay_t<Callable>, std::decay_t<Args>...>;
    //using task_type = std::invoke_result_t<std::decay_t<Callable>, std::decay_t<Args>...>(*)(std::decay_t<Args>...);


private:
    Command<R, Args...> task;
    /**
     * @brief state this variable describe the current circuit breaker state
     */
    State state;
    /**
     * @brief ratio represents the success ratio until now.
     * It holds a percentage value
     */
    double ratio;
    /**
     * @brief ratio_trip this variable reprensent ratio at which the circuit trips.
     * It holds a percentage value
     */
    double ratio_trip;
    /**
     * @brief failure_threshold_reached this variable represents the number of time
     * the circuit breaker has tripped.
     */
    int failure_threshold_reached;
    /**
     * @brief usage the number of call made through this circuit breaker instance.
     * this helps when you want to monitor the circuit breaker
     */
    int usage;

    /**
     * @brief failure_counter current failure count
     */
    int failure_counter;
    /**
     * @brief failures total failures count since the programme started
     */
    int failures;

    /**
     * @brief successes number of successfuly calls
     */
    int successes;
    /**
     * @brief failure_threshold limit of the failure number in a row
     */
    int failure_threshold;
    /**
     * @brief failure_time the last time a failure occured
     */
    time_point_ms_t failure_time;
    /**
     * @brief time_to_retry time to wait before transitioning from OPEN state to HALF_OPEN state
     */
    duration_ms_t time_to_retry;
    /**
     * @brief deadline time to wait for the service's reply
     */
    duration_ms_t deadline;

    /**
     * @brief closed_observers this is the list of observer (callback) to be call when the circuit
     * becomes closed
     */
    std::vector<FunctionWrapper> closed_observers;
    /**
     * @brief open_observers this is the list of observer (callback) to be call when the circuit
     * becomes open.
     */
    std::vector<FunctionWrapper> open_observers;
    /**
     * @brief listeners this is the list of observer (callback) to be call when the circuit
     * changes it state.
     */
    std::vector<FunctionWrapper> listeners;
    /**
     * @brief pool the Thread pool that is used to run request in decated threads.
     */
    std::shared_ptr<ThreadPool> pool;
    // private interface
private:
    /**
     * @brief onClosedState processes the request when the circuit in the closed state.
     * @param args The list of arguments that are to be forward to the service
     * @return a std::future containing the result of the request.
     */
    std::future<R> onClosedState(Args&&... args){
        std::promise<R> result_not_ready;

        std::future<R> result = result_not_ready.get_future();
        std::future<R> async_result = pool->submit(task, std::forward<Args>(args)...);
        std::future_status status = async_result.wait_for(deadline);
        if( status == std::future_status::ready){
            try {
                result_not_ready.set_value(async_result.get());
                reset();
            } catch (const ServiceError &e) {
                result_not_ready.set_exception(std::make_exception_ptr(e));
                failure_count();
                if(getFailure_counter() >= getFailure_threshold()){
                    ++failure_threshold_reached;
                    trip();
                }
            }
        }
        else if(status == std::future_status::timeout){
            result_not_ready.set_exception(std::make_exception_ptr(TimeoutError()));
            failure_count();
            if(getFailure_counter() >= getFailure_threshold()){
                ++failure_threshold_reached;
                trip();
            }

        }
        return result;
    }

    /**
     * @brief onOpenState processes the request when the circuit in the open state.
     * @param args The list of arguments that are to be forward to the service
     * @return a std::future containing the result of the request.
     */
    std::future<R> onOpenState(Args&&... args){
        auto tmp = std::chrono::system_clock::now();
        std::promise<R> error;
        updateFailures();
        auto elapsed_time_duration =std::chrono::duration_cast<duration_ms_t>(tmp - getFailure_time());
        if( elapsed_time_duration >= getTime_to_retry()){
            transition(State::HALF_OPEN);
        }
        error.set_exception(std::make_exception_ptr(ServiceError("SYSTEM NOW")));
        return error.get_future();
    }

    /**
     * @brief onHalfOpenState processes the request when the circuit in the half-open state.
     * @param args The list of arguments that are to be forward to the service
     * @return a std::future containing the result of the request.
     */
    std::future<R> onHalfOpenState(Args&&... args){
        std::promise<R> result_not_ready;
        std::future<R> result = result_not_ready.get_future();
        std::future<R> async_result = pool->submit(task, std::forward<Args>(args)...);
        std::future_status status = async_result.wait_for(deadline);
        if( status == std::future_status::ready){
            try {
                result_not_ready.set_value(async_result.get());
                reset();
            } catch (const ServiceError &e) {
                result_not_ready.set_exception(std::make_exception_ptr(e));
                updateFailures();
                trip();
            }
        }
        else if(status == std::future_status::timeout){
            result_not_ready.set_exception(std::make_exception_ptr(TimeoutError()));
            updateFailures();
            trip();
        }
        return result;
    }

    /**
     * @brief transition this method changes the circuit breaker current state
     * and notify the registered observers about this change. Nothing happens when
     * state is the current circuit state.
     * @param state the new circuit state.
     */
    void transition(State state){
        if(this->state != state){
            this->state = state;
            if(state == State::OPEN){
                std::for_each(open_observers.begin(), open_observers.end(), [](FunctionWrapper &observer){
                    observer();
                });
            }else if(state == State::CLOSED){
                std::for_each(closed_observers.begin(), closed_observers.end(), [](FunctionWrapper &observer){
                    observer();
                });
            }
        }
    }
public:

    explicit CircuitBreaker(Command<R, Args...> task, duration_ms_t deadline, duration_ms_t time_to_retry, int failure_threshold):
        task{task} ,failure_threshold{failure_threshold},time_to_retry{time_to_retry}, deadline{deadline}
    {

        ratio = 0.0;
        ratio_trip = 0.0;
        state = State::CLOSED;
        failure_counter = 0;
        failures = 0;
        successes = 0;
        usage = 0;
        failure_threshold_reached = 0;
    }

    /**
     * @brief CircuitBreaker construct the Circuit breaker with the settings provided by user.
     * @param service Service which  processes the request
     * @param deadline the time to wait for the response from the Service
     * @param time_to_retry  the time to wait before transition from OPEN state to HALF_OPEN
     * @param failure_threshold the number of allowed failures in row before transitioning from CLOSED state
     * to OPEN state.
     */

    ~CircuitBreaker(){
//        if(usage){
//            ratio =100 * (static_cast<double>(successes)/usage);
//            ratio_trip =100 * (static_cast<double>(failure_threshold_reached)/usage) * failure_threshold;
//        }
#ifdef DEBUG_ON
        LOG("usage summary :\tsuccess =\t", successes, ";\terror = ", failures,
            ";\tusage : ",usage, ";\tdeadline(",TIME_UNIT,"): ",deadline.count(),
            " Success RATIO(%):", ratio, " Number of trip :", failure_threshold_reached,
            " Trip Ratio(%) :", ratio_trip);
        LOG("CB THREAD POOL usage summary : ", "submitted tasks : ", pool->getSubmitted(), " finished tasks : ", pool->getFinishedTasks(), " still active : ", pool->getActiveTask());

#endif
    }

    /**
     * @brief trip this method changes the Circuit breaker current state to OPEN state.
     * This method is called everytime the failure threshold is reached.
     * After calling this method, the circuit breaker is in an OPEN state.
     * The ratio_trips is updated when this method is called.
     */
    void trip(){
        failure_time = std::chrono::system_clock::now();
        failure_counter = 0;
        transition(State::OPEN);
    }
    /**
     * @brief reset this method is called when the request succeed. AFter calling this
     * method the circuit breaker is in CLOSED state.
     *
     */
    void reset(){
        ++successes;
        failure_counter = 0;
        transition(State::CLOSED);
    }
    /**
     * @brief failure_count this method count the number of time the service has already failed.
     * It is called everytime the Service failed a request.
     */
    void failure_count(){
        ++failure_counter;
        ++failures;
    }
    // Service interface
public:

    /**
     * @brief execute the client uses this method to submit a new request to the service.
     *
     * @param args the arguments the service accepts to process the request.
     * @return a std::future containing the result of the request.
     * On success the future contains a the result.
     * On failure the future contains an exception indicating the error.
     * These exception might be ServiceError or TimeoutError.
     */
    std::future<R> execute(Args&&... args){
        ++usage;
        std::future<R> res;
        if(state == State::CLOSED){
            res = onClosedState(std::forward<Args>(args)...);
        }
        else if(state == State::OPEN){
            res = onOpenState(std::forward<Args>(args)...);
        }
        else if(state == State::HALF_OPEN){
            res = onHalfOpenState(std::forward<Args>(args)...);
        }
        updateRatio();
        return res ;
    }

    /**
     * @brief getFailure_counter helper function which return the number of failure encountered until the time this method is called.
     * @return total number of failure before the circuit breaker trips.
     * Note : This number is the number of failure in a row, not the total number of failure since the programme started.
     */
    int getFailure_counter() const{
        return failure_counter;
    }

    /**
     * @brief getFailures return the failures count since the programm started
     * @return failures
     */
    int getFailures() const{
        return failures;
    }

    /**
     * @brief getTime_to_retry helper function which returns the time the circuit breaker waits before changing
     * from OPEN state to HALF_OPEN state.
     * @return a duration object contains the time to wait before retry.
     */
    duration_ms_t getTime_to_retry() const{
        return time_to_retry;
    }


    /**
     * @brief getFailure_time helper function which returns the last time point when the Service failed a request

     * @return a time point object contains the time point when the last failure occured.
     */
    time_point_ms_t getFailure_time() const{
        return failure_time;
    }

    /**
     * @brief addOnCircuitBreakClosedObserver this method register observer which should be called everytime
     * the circuit breaker changes it current state to the CLOSED state.
     * @param observer the code to be executed the circuit breaker changes to CLOSED state.
     * Note I : The observer is just a callback function with the signature : void()
     * Note II: The observer is run on the caller thread. The observer should note last long.
     */
    void addOnCircuitBreakClosedObserver(FunctionWrapper observer){
        closed_observers.push_back(std::move(observer));
    }
    /**
     * @brief addOnCircuitBreakOpenObserver this method register observer which should be called everytime
     * the circuit breaker changes it current state to the OPEN state.
     * @param observer the code to be executed the circuit breaker changes to OPEN state.
     * Note I : The observer is just a callback function with the signature : void()
     * Note II: The observer is run on the caller thread. The observer should note last long.
     */
    void addOnCircuitBreakOpenObserver(FunctionWrapper observer){
        open_observers.push_back(std::move(observer));
    }

    /**
     * @brief addOnCircuitBreakHalfOpenObserver this method register observer which should be called everytime
     * the circuit breaker changes it current state to the HALF_OPEN state.
     * @param observer the code to be executed the circuit breaker changes to HALF_OPEN state.
     * Note I : The observer is just a callback function with the signature : void()
     * Note II: The observer is run on the caller thread. The observer should note last long.
     */
    void addOnCircuitBreakHalfOpenObserver(FunctionWrapper observer){

    }


    /**
     * @brief getUsage get the number of time the circuit is used until the
     * time this method is called.
     * @return usage
     */
    int getUsage() const{
        return usage;
    }

    /**
     * @brief updateFailures updates the number of time the serive has failed
     */
    void updateFailures(){
        ++failure_counter;
        ++failures;
        if(failure_counter >= failure_threshold){
            ++failure_threshold_reached;
            failure_counter = 0;
        }
    }

    /**
     * @brief getSuccesses get the number of time the service succeed until
     * the time this method is called
     * @return
     */
    int getSuccesses() const{
        return successes;
    }

    /**
     * @brief getFailure_threshold get the failure threshold currently set in the circuit.
     * @return
     */
    int getFailure_threshold() const{
        return failure_threshold;
    }

    /**
     * @brief getRatio get the current ratio of success. This value already converted in percent
     * @return ratio
     */
    double getRatio() const{
        return ratio;
    }

    /**
     * @brief getFailure_threshold_reached get the number of time the failure threshold
     * has been reached until this method is called.
     * @return failure_threshold_reached
     */
    int getFailure_threshold_reached() const{
        return failure_threshold_reached;
    }

    /**
     * @brief getRatio_trip get the ratio on how many time the circuit has tripped.
     * @return ratio_trip
     */
    double getRatio_trip() const{
        return ratio_trip;
    }

    /**
     * @brief updateRatio Update the success ratio and the trip ratio.
     */
    void updateRatio(){
        if(usage){
            ratio =100 * (static_cast<double>(successes)/usage);
            ratio_trip =100 * (static_cast<double>(failure_threshold_reached)/usage) * failure_threshold;
        }
    }

    /**
     * @brief isOpen check if the circuit is currently open
     * @return  true when the circuit is open. It returns false when the
     * circuit is not in the open state.
     */
    bool isOpen() const{
        return state == State::OPEN;
    }

    /**
     * @brief isClosed check if the circuit is currently closed
     * @return  true when the circuit is closed. It returns false when the
     * circuit is not in the closed state.
     */
    bool isClosed()const{
        return  state == State::CLOSED;
    }

    /**
     * @brief isHalfOpen check if the circuit is currently half-open
     * @return  true when the circuit is half-open. It returns false when the
     * circuit is not in the half-open state.
     */
    bool isHalfOpen() const{
        return  state == State::HALF_OPEN;
    }

    /**
     * @brief setPool set the thread pool used to run the submited requests
     * @param value
     */
    void setPool(const std::shared_ptr<ThreadPool> &pool){
        this->pool = pool;
    }
    duration_ms_t getDeadline() const{
        return deadline;
    }
};

#endif // CIRCUITBREAKER_H
