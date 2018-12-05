#ifndef CIRCUITBREAKER_H
#define CIRCUITBREAKER_H
#include "service.h"
#include <chrono>
#include <future>
#include <memory>
#include <log.h>

constexpr int OPEN_STATE = 1;
constexpr int CLOSED_STATE = 2;
constexpr int HALF_OPEN_STATE = 3;

template<class  Duration>
using sys_time = std::chrono::time_point<std::chrono::system_clock,Duration>;

using sys_milliseconds = sys_time<std::chrono::milliseconds>;

typedef std::chrono::time_point<std::
chrono::system_clock> time_point_ms_t;

typedef  std::chrono::milliseconds time_ms_t;

class CircuitBreaker
{
public:
    enum CBSTATE{
        OPEN        = OPEN_STATE,
        CLOSED      = CLOSED_STATE,
        HALF_CLOSED = HALF_OPEN_STATE
    };
private:
    int error_rate;
    bool first_call;
    time_point_ms_t last_call_time_point;
    time_point_ms_t failure_time;
    time_ms_t time_to_wait;
    time_ms_t time_to_try;
    CBSTATE state;
    //Service *service;
    std::shared_ptr<Service> service;
public:
    //CircuitBreaker(Service *service);
    CircuitBreaker(std::shared_ptr<Service> service);
    // Service interface
public:
    int process_request(int request);
};

#endif // CIRCUITBREAKER_H
