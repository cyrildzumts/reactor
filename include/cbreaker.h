#ifndef CBREAKER_H
#define CBREAKER_H

#include "cbexeptions.h"
#include "function_wrapper.h"
#include "service.h"
#include "activeobject.h"
#include <utility>
#include <functional>
#include <type_traits>
#include <optional>
#include <chrono>
#include <future>
#include <memory>
#include <log.h> // my own developped Logger


using namespace std::chrono_literals;

typedef std::chrono::time_point<std::chrono::system_clock> time_point_ms_t;
typedef  std::chrono::microseconds duration_ms_t;

namespace breaker {

template<typename Callable, typename... Args>
struct typeof_t;

template<typename Callable, typename... Args>
struct typeof_t{
    using result_type = std::invoke_result_t<std::decay_t<Callable>, std::decay_t<Args>...>;
    using op_type = Callable(*)(Args...);
};


template<typename Callable, typename... Args>
class AbstractBreaker
{
public:
    using result_type = std::invoke_result_t<std::decay_t<Callable>, std::decay_t<Args>...>;
    enum State {
        CLOSED, OPEN, HALF_OPEN
    };

private:

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


    std::vector<FunctionWrapper> closed_observers;
    std::vector<FunctionWrapper> open_observers;
    std::vector<FunctionWrapper> listeners;
    std::shared_ptr<concurrency::Active> active;

    /**
     * @brief current_state the current circuit breaker state
     */
    State state;
    Callable *op;

private:
    /**
     * @brief change_State this method changes the circuit breaker's current state
     * @param fsm_state the new state to transition to.
     * if fsm_state is a nullptr, nothing happens. No transition, the circuit breaker stays in
     * in its current state.
     */
    void change_State(){

    }
    std::future<result_type> onClosedState(Args&&... args){
        std::promise<result_type> result_not_ready;
        std::future<result_type> result = result_not_ready.get_future();
        std::future async_result = active->submit(op, std::forward<Args>(args)...);
        std::future_status status = async_result.wait_for(std::chrono::microseconds(deadline));
        if( status == std::future_status::ready){
            try {
                result_not_ready.set_value(async_result.get());
                reset();
                ++successes;
            } catch (const ServiceError &e) {
                result_not_ready.set_exception(std::make_exception_ptr(e));

                failure_count();
                if(getFailure_counter() > getFailure_threshold()){
                    trip();
                    transition(OPEN);
                }
            }
        }
        else if(status == std::future_status::timeout){
            result_not_ready.set_exception(std::make_exception_ptr(breaker::TimeoutError()));
            failure_count();
            if(getFailure_counter() > getFailure_threshold()){
                trip();
                transition(OPEN);
            }
        }
        return result;
    }
    std::future<result_type> onOpenState(Args&&... args){
        auto tmp = std::chrono::system_clock::now();
        std::promise<result_type> error;
        updateFailures();
        auto elapsed_time_duration =std::chrono::duration_cast<duration_ms_t>(tmp - getFailure_time());
        if( elapsed_time_duration >= getTime_to_retry()){
            transition(HALF_OPEN);
        }
        error.set_exception(std::make_exception_ptr(ServiceError("SYSTEM NOW")));
        return error.get_future();
    }


    std::future<result_type> onHalfOpenState(Args&&... args){
        std::promise<result_type> result_not_ready;
        std::future<result_type> result = result_not_ready.get_future();
        std::future async_result = active->submit(op, std::forward<Args>(args)...);
        std::future_status status = async_result.wait_for(std::chrono::microseconds(deadline));
        if( status == std::future_status::ready){
            try {
                result_not_ready.set_value(async_result.get());
                reset();
                transition(CLOSED);
                ++successes;
            } catch (const ServiceError &e) {
                result_not_ready.set_exception(std::make_exception_ptr(e));
                failure_count();
                trip();
                transition(OPEN);
            }
        }
        else if(status == std::future_status::timeout){
            result_not_ready.set_exception(std::make_exception_ptr(breaker::TimeoutError()));
            failure_count();
            trip();
            transition(OPEN);
        }
        return result;
    }



    void transition(State state){
        this->state = state;
        if(state == OPEN){
            std::for_each(open_observers.begin(), open_observers.end(), [](FunctionWrapper &observer){
                observer();
            });
        }else if(state == CLOSED){
            std::for_each(closed_observers.begin(), closed_observers.end(), [](FunctionWrapper &observer){
                observer();
            });
        }
    }
public:

    /**
     * @brief CircuitBreaker construct the Circuit breaker with the settings provided by user.
     * @param service Service which  processes the request
     * @param deadline the time to wait for the response from the Service
     * @param time_to_retry  the time to wait before transition from OPEN state to HALF_OPEN
     * @param failure_threshold the number of allowed failures in row before transitioning from CLOSED state
     * to OPEN state.
     */

    explicit AbstractBreaker(duration_ms_t deadline, duration_ms_t time_to_retry, int failure_threshold);

    virtual ~AbstractBreaker(){

    }
    /**
     * @brief trip this method changes the Circuit breaker current state to OPEN state.
     * This method is called everytime the failure threshold is reached.
     * After calling this method, the circuit breaker is an OPEN state.
     */
    void trip(){
        failure_time = std::chrono::system_clock::now();
        state = OPEN;
    }
    /**
     * @brief reset this method changes the circuit breaker current state to CLOSED state.
     * This method is executed when the service succeed.
     */
    void reset(){
        failure_counter = 0;
        state = CLOSED;
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
     * @brief executeThis the interface to the client using the service provided by Service.
     * The operation performed when calling this method depends on the circuit breaker current state.
     * @param request the first parameter the Service is awaiting for.
     * @param delay delay the second parameter the Service is awaiting for.
     * @return the returned value by the Service.
     * Note: if the Service threw an exception instead of returning a value, the same exception
     * rethrown by this method. So this method should be used in a try-catch block.
     */

    // Client Interface: call are delegated to FSM

    std::future<result_type> execute(Args&&... args) noexcept {
        ++usage;
        std::future<result_type> res;
        if(state == CLOSED){
            res = onClosedState(std::forward<Args>(args)...);
        }
        else if(state == OPEN){
            res = onOpenState(std::forward<Args>(args)...);
        }
        else if(state == HALF_OPEN){
            res = onHalfOpenState(std::forward<Args>(args)...);
        }
        return res ;
    }
    /**
     * @brief call helper method used internally by the State Machine. it delagate the request to the Service.
     * @param request the first parameter the Service is awaiting for.
     * @param delay the second parameter the Service is awaiting for.
     * @return the returned value by the Service.
     * Note: if the Service threw an exception instead of returning a value, the same exception
     * rethrown by this method. So this method should be used in a try-catch block.
     */


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
     * Note that the observer is just a callback function with the signature :
     * void()
     */
    void addOnCircuitBreakClosedObserver(FunctionWrapper observer);
    /**
     * @brief addOnCircuitBreakOpenObserver this method register observer which should be called everytime
     * the circuit breaker changes it current state to the OPEN state.
     * @param observer the code to be executed the circuit breaker changes to OPEN state.
     * Note that the observer is just a callback function with the signature :
     * void()
     */
    void addOnCircuitBreakOpenObserver(FunctionWrapper observer);

    /**
     * @brief addOnCircuitBreakHalfOpenObserver this method register observer which should be called everytime
     * the circuit breaker changes it current state to the HALF_OPEN state.
     * @param observer the code to be executed the circuit breaker changes to HALF_OPEN state.
     * Note that the observer is just a callback function with the signature :
     * void()
     */
    void addOnCircuitBreakHalfOpenObserver(FunctionWrapper observer);

    /**
     * @brief getActive accessor to the active object of this circuit breaker.
     * @return the active object instance used by this circuit breaker
     */
    std::shared_ptr<concurrency::Active> getActive() const{
        return active;
    }
    /**
     * @brief setActive set the Active Object instance which will handler the task execution
     * @param value
     */
    void setActive(const std::shared_ptr<concurrency::Active> &value){
        active = value;
    }
    /**
     * @brief getUsage accessor to the usage count of this circuit breaker
     * @return usage
     */
    int getUsage() const{
        return usage;
    }
    void updateFailures(){
        ++failures;
    }
    int getSuccesses() const{
        return successes;
    }

    int getFailure_threshold() const
    {
    return failure_threshold;
    }

    void setFailure_threshold(int value)
    {
    failure_threshold = value;
    }
};

template<typename Callable, typename... Args>
class CBreaker : AbstractBreaker<Callable, Args...>{


};


template<typename Callable, typename... Args>
AbstractBreaker<Callable, Args...>::AbstractBreaker(duration_ms_t deadline, duration_ms_t time_to_retry, int failure_threshold):
    failure_threshold{failure_threshold},time_to_retry{time_to_retry}, deadline{deadline}
{
    state = CLOSED;
    failure_counter = 0;
    failures = 0;
    successes = 0;
    usage = 0;
    op = job;
}

template<typename Callable, typename... Args>
void AbstractBreaker<Callable, Args...>::addOnCircuitBreakClosedObserver(FunctionWrapper observer){
    closed_observers.push_back(std::move(observer));
}

template<typename Callable, typename... Args>
void AbstractBreaker<Callable, Args...>::addOnCircuitBreakHalfOpenObserver(FunctionWrapper observer){
    //half_open_observers.push_back(std::move(observer));
}

template<typename Callable, typename... Args>
void AbstractBreaker<Callable, Args...>::addOnCircuitBreakOpenObserver(FunctionWrapper observer){
    open_observers.push_back(std::move(observer));
}



}
#endif // CBREAKER_H


