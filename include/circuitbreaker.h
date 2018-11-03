#ifndef CIRCUITBREAKER_H
#define CIRCUITBREAKER_H
#include "service.h"
#include <chrono>
#include <future> // used to allow asynchronous call with future
#include <memory> // make usage of smart pointer


constexpr int DEADLINE_TIME = 10;
constexpr int FAILURE_LIMIT = 2;
constexpr int TIMEOUT_FAILURE = 3;


typedef std::chrono::time_point<std::
chrono::system_clock> time_point_ms_t;

typedef  std::chrono::milliseconds time_ms_t;

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
public:
    enum CBSTATE{
        OPEN        = 1,
        CLOSED      = 2,
        HALF_CLOSED = 3
    };


private:
    int error_rate;
    bool failure_on_last_call;
    bool first_call;
    int failure_counter;
    time_point_ms_t last_call_time_point;
    time_point_ms_t failure_time;
    time_ms_t time_to_wait;
    time_ms_t time_to_try;
    CBSTATE state;
    FSM *current_state;
    std::shared_ptr<Service> service;
    friend class FSM;

private:
    void change_State(FSM *fsm_state);
public:
    CircuitBreaker(std::shared_ptr<Service> service);
    void trip();
    void reset();
    void failure_count();
    // Service interface
public:
    int process_request(int request);
    int call(int request);
    int getFailure_counter() const;
    time_ms_t getTime_to_try() const;
    bool getFailure_on_last_call() const;
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
