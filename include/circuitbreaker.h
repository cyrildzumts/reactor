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
#include <log.h> // my own developped Logger


void print(int arg, int delay);


class CircuitBreaker;

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


class FSM{
protected:
    std::vector<FunctionWrapper> listeners;
protected:
    /**
     * @brief change_state this method changes the circuit breaker current state.
     * @param cbr the context object.
     * @param state the new state to transition to.
     * when state is a nullptr, nothing is done.
     */
    virtual void change_state(CircuitBreaker *cbr, FSM *state);
public:
    virtual ~FSM(){}

    virtual CURLcode fetch(CircuitBreaker *cbr, const std::string &url) = 0;

    /**
     * @brief call_service this method executes the right operations depending the circuit breaker
     * current state
     * @param cbr the context into which the operations are executed.
     * @param request the first parameter the Service is awaiting for.
     * @param delay the second parameter the Service is awaiting for.
     * @return
     */
    virtual int call_service(CircuitBreaker *cbr,  int request, int delay) = 0;
    /**
     * @brief trip this method performs operations required when transitioning in OPEN state
     * @param cbr the context object in which the required operations are run.
     */
    virtual void trip(CircuitBreaker *cbr) = 0;
    /**
     * @brief reset this method performs operations required when transitioning in CLOSED state
     * @param cbrthe context object in which the required operations are run.
     */
    virtual void reset(CircuitBreaker *cbr) = 0;
    /**
     * @brief notify this method executes the registered callback
     * This method only runs the observers of the current state.
     */
    virtual void notify(){
        if(!listeners.empty()){
            std::for_each(listeners.begin(), listeners.end(), [&](FunctionWrapper &f){
                f();
            });
        }
    }

    /**
     * @brief addObservers this method adds a new observer to the current state
     * @param f the observer to add.
     */
    virtual void addObservers(FunctionWrapper f){
        listeners.push_back(std::move(f));
    }

    virtual const std::string getName() = 0;
};

class CircuitBreaker
{

private:
    double ratio;
    double ratio_trip;
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
     * @brief current_state the current circuit breaker state
     */
    FSM *current_state;

    std::shared_ptr<concurrency::Active> active;
    std::shared_ptr<ThreadPool> pool;
    friend class FSM;
    // private interface
private:
    template<typename Callable, typename... Args>
    std::future<std::invoke_result_t<std::decay_t<Callable>, std::decay_t<Args>...>>
    execute_intern(Callable &&op, Args&&... args);

private:
    /**
     * @brief change_State this method changes the circuit breaker's current state
     * @param fsm_state the new state to transition to.
     * if fsm_state is a nullptr, nothing happens. No transition, the circuit breaker stays in
     * in its current state.
     */
    void change_State(FSM *fsm_state);
public:

    /**
     * @brief CircuitBreaker construct the Circuit breaker with the settings provided by user.
     * @param service Service which  processes the request
     * @param deadline the time to wait for the response from the Service
     * @param time_to_retry  the time to wait before transition from OPEN state to HALF_OPEN
     * @param failure_threshold the number of allowed failures in row before transitioning from CLOSED state
     * to OPEN state.
     */
    explicit CircuitBreaker(duration_ms_t deadline, duration_ms_t time_to_retry, int failure_threshold);
    //CircuitBreaker(std::unique_ptr<FunctionWrapper> service, duration_ms_t deadline, duration_ms_t time_to_retry, int failure_threshold);
    ~CircuitBreaker();
    /**
     * @brief trip this method changes the Circuit breaker current state to OPEN state.
     * This method is called everytime the failure threshold is reached.
     * After calling this method, the circuit breaker is an OPEN state.
     */
    void trip();
    /**
     * @brief reset this method changes the circuit breaker current state to CLOSED state.
     * This method is executed when the service succeed.
     */
    void reset();
    /**
     * @brief failure_count this method count the number of time the service has already failed.
     * It is called everytime the Service failed a request.
     */
    void failure_count();
    // Service interface
public:
    /**
     * @brief process_request This the interface to the client using the service provided by Service.
     * The operation performed when calling this method depends on the circuit breaker current state.
     * @param request the first parameter the Service is awaiting for.
     * @param delay delay the second parameter the Service is awaiting for.
     * @return the returned value by the Service.
     * Note: if the Service threw an exception instead of returning a value, the same exception
     * rethrown by this method. So this method should be used in a try-catch block.
     */
    int process_request(int request, int delay = PROCESSING_DURATION); // Client Interface: call are delegated to FSM


    template<typename Callable, typename... Args>
    std::future<std::invoke_result_t<std::decay_t<Callable>, std::decay_t<Args>...>>
    execute(Callable &&op, Args&&... args);



    /**
     * @brief call helper method used internally by the State Machine. it delagate the request to the Service.
     * @param request the first parameter the Service is awaiting for.
     * @param delay the second parameter the Service is awaiting for.
     * @return the returned value by the Service.
     * Note: if the Service threw an exception instead of returning a value, the same exception
     * rethrown by this method. So this method should be used in a try-catch block.
     */
    int call(int request, int delay);

    /**
     * @brief getFailure_counter helper function which return the number of failure encountered until the time this method is called.
     * @return total number of failure before the circuit breaker trips.
     * Note : This number is the number of failure in a row, not the total number of failure since the programme started.
     */
    int getFailure_counter() const;

    /**
     * @brief getFailures return the failures count since the programm started
     * @return failures
     */
    int getFailures() const;

    /**
     * @brief getTime_to_retry helper function which returns the time the circuit breaker waits before changing
     * from OPEN state to HALF_OPEN state.
     * @return a duration object contains the time to wait before retry.
     */
    duration_ms_t getTime_to_retry() const;


    /**
     * @brief getFailure_time helper function which returns the last time point when the Service failed a request

     * @return a time point object contains the time point when the last failure occured.
     */
    time_point_ms_t getFailure_time() const;
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
    std::shared_ptr<concurrency::Active> getActive() const;
    /**
     * @brief setActive set the Active Object instance which will handler the task execution
     * @param value
     */
    void setActive(const std::shared_ptr<concurrency::Active> &value);
    /**
     * @brief getUsage accessor to the usage count of this circuit breaker
     * @return usage
     */
    int getUsage() const;
    void updateFailures();
    int getSuccesses() const;
    int getFailure_threshold() const;
    double getRatio() const;
    int getFailure_threshold_reached() const;
    double getRatio_trip() const;
    void updateRatio();
    bool isOpen() const;
    bool isClosed()const;
    bool isHalfOpen() const;
    void setPool(const std::shared_ptr<ThreadPool> &value);
    CURLcode fetch(const std::string &url);
    CURLcode fetch_sumbit(const std::string &url);

};


class CircuitBreakerOpen : public FSM{
private:
    static FSM *root;
    CircuitBreakerOpen(){}
    // FSM interface

public:
    virtual ~CircuitBreakerOpen(){}
    static FSM *instance();
    virtual int call_service(CircuitBreaker *cbr, int request, int delay) override;
    virtual void trip(CircuitBreaker *cbr) override;
    virtual void reset(CircuitBreaker *cbr) override;
    virtual const std::string getName();
    virtual CURLcode fetch(CircuitBreaker *cbr, const std::string &url);
};


class CircuitBreakerClosed : public FSM{

private:
    static FSM *root;
    CircuitBreakerClosed(){}
    // FSM interface

public:
    virtual ~CircuitBreakerClosed(){}
    static FSM *instance();
    virtual int call_service(CircuitBreaker *cbr, int request, int delay) override;
    virtual void trip(CircuitBreaker *cbr) override;
    virtual void reset(CircuitBreaker *cbr) override;
    virtual const std::string getName();
    virtual CURLcode fetch(CircuitBreaker *cbr, const std::string &url);
};


class CircuitBreakerHalfOpen : public FSM{

private:
    static FSM *root;
    CircuitBreakerHalfOpen(){}
    // FSM interface
public:
    virtual ~CircuitBreakerHalfOpen(){}
    static FSM *instance();
    virtual int call_service(CircuitBreaker *cbr, int request, int delay) override;
    virtual void trip(CircuitBreaker *cbr) override;
    virtual void reset(CircuitBreaker *cbr) override;
    virtual const std::string getName();
    virtual CURLcode fetch(CircuitBreaker *cbr, const std::string &url);
};

#endif // CIRCUITBREAKER_H
