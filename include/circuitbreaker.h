#ifndef CIRCUITBREAKER_H
#define CIRCUITBREAKER_H
#include "service.h"
#include <chrono>
#include <future> // used to allow asynchronous call with future
#include <memory> // make usage of smart pointer

using namespace std::chrono_literals;

constexpr int DEADLINE_TIME = 10;
constexpr int FAILURE_LIMIT = 2;
constexpr int TIMEOUT_FAILURE = 3;


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
    virtual void change_state(CircuitBreaker *cbr, FSM *state);
public:
    virtual ~FSM(){}
    virtual int call_service(CircuitBreaker *cbr,  int request) = 0;
    virtual void trip(CircuitBreaker *cbr) = 0;
    virtual void reset(CircuitBreaker *cbr) = 0;
};

class CircuitBreaker
{
private:
    int failure_counter;
    time_point_ms_t failure_time;
    duration_ms_t time_to_retry;
    FSM *current_state;
    std::shared_ptr<Service> service;
    friend class FSM;

private:
    void change_State(FSM *fsm_state);
public:
    CircuitBreaker(std::shared_ptr<Service> service);
    void trip(); // Open the circuit when the failure rate are reached
    void reset(); // Close the circuit and reset the failure counter
    void failure_count();
    // Service interface
public:
    int process_request(int request); // Client Interface: call are delegated to FSM
    int call(int request);
    int getFailure_counter() const;
    duration_ms_t getTime_to_retry() const;
    time_point_ms_t getFailure_time() const;
};


class CircuitBreakerOpen : public FSM{
private:
    static FSM *root;
    CircuitBreakerOpen(){}
    // FSM interface

public:
    virtual ~CircuitBreakerOpen(){}
    static FSM *instance();
    virtual int call_service(CircuitBreaker *cbr, int request) override;
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
    virtual int call_service(CircuitBreaker *cbr, int request) override;
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
    virtual int call_service(CircuitBreaker *cbr, int request) override;
    virtual void trip(CircuitBreaker *cbr) override;
    virtual void reset(CircuitBreaker *cbr) override;
};
#endif // CIRCUITBREAKER_H
