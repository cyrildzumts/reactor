#ifndef CIRCUITBREAKER_H
#define CIRCUITBREAKER_H
#include "function_wrapper.h"

#include <utility>
#include <functional>
#include <type_traits>

#include "service.h"
#include <chrono>
#include <future>
#include <memory>
#include <log.h>


using namespace std::chrono_literals;

constexpr int DEADLINE_TIME = 10;
constexpr int FAILURE_LIMIT = 2;
constexpr int TIMEOUT_FAILURE = 3;

constexpr int WAIT_TIME = 250;
typedef std::chrono::time_point<std::
chrono::system_clock> time_point_ms_t;


typedef  std::chrono::milliseconds duration_ms_t;


class CircuitBreaker;



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
    virtual void change_state(CircuitBreaker *cbr, FSM *state);
public:
    virtual ~FSM(){}
    virtual int call_service(CircuitBreaker *cbr,  int request, int delay) = 0;
    virtual void trip(CircuitBreaker *cbr) = 0;
    virtual void reset(CircuitBreaker *cbr) = 0;
    virtual void notify(){
        if(!listeners.empty()){
            std::for_each(listeners.begin(), listeners.end(), [&](FunctionWrapper &f){
                f();
            });
        }
    }
    virtual void addObservers(FunctionWrapper f){
        listeners.push_back(std::move(f));
    }
};

class CircuitBreaker
{

private:
    int failure_counter;
    int failure_threshold;
    time_point_ms_t failure_time;
    duration_ms_t time_to_retry;
    duration_ms_t deadline;
    FSM *current_state;
    std::shared_ptr<Service> service;
    std::unique_ptr<Service> serv;
    friend class FSM;

private:
    void change_State(FSM *fsm_state);
public:
    /**
     * @brief CircuitBreaker Construct a circuit breaker with default parameters
     * @param service Service which  processes the request
     */
    CircuitBreaker(std::shared_ptr<Service> service);
    CircuitBreaker(std::unique_ptr<Service> service, duration_ms_t deadline, duration_ms_t time_to_retry, int failure_threshold);
    void trip(); // Open the circuit when the failure rate are reached
    void reset(); // Close the circuit and reset the failure counter
    void failure_count();
    // Service interface
public:
    int process_request(int request, int delay = PROCESSING_DURATION); // Client Interface: call are delegated to FSM
    int call(int request, int delay);
    int getFailure_counter() const;
    duration_ms_t getTime_to_retry() const;
    time_point_ms_t getFailure_time() const;
    void addOnCircuitBreakClosedObserver(FunctionWrapper observer);
    void addOnCircuitBreakOpenObserver(FunctionWrapper observer);
    void addOnCircuitBreakHalfOpenObserver(FunctionWrapper observer);
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
};
#endif // CIRCUITBREAKER_H
