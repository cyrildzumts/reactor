#ifndef CIRCUITBREAKER_H
#define CIRCUITBREAKER_H
#include "service.h"
#include <chrono>
#include <future>

template<class  Duration>
using sys_time = std::chrono::time_point<std::chrono::system_clock,Duration>;

using sys_milliseconds = sys_time<std::chrono::milliseconds>;

typedef std::chrono::time_point<std::
chrono::system_clock> time_point_ms_t;

typedef  std::chrono::milliseconds time_ms_t;
/**
 * Func must be a callable object that
 *
 */
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
    bool first_call;
    time_point_ms_t last_call_time_point;
    time_point_ms_t failure_time;
    time_ms_t time_to_wait;
    time_ms_t time_to_try;
    CBSTATE state;
    Service *service;
public:
    CircuitBreaker(Service *service);

    // Service interface
public:
    int process_request(int request);
};

#endif // CIRCUITBREAKER_H
