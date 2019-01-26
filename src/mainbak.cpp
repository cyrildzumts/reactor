#include "threadpool.h"
#include "circuitbreaker.h"
#include <log.h> // < -- Logger library
#include <iostream>
#include <memory> // <-- Smart pointers

#define WORKERS 4
#define FAILURE_THRESHOLD 3
#define DEADLINE 50
#define RETRY_TIME 70
using namespace std;

int _main_(int argc, char const *argv[])
{
    Command service {http_job};
    std::shared_ptr<ThreadPool> pool =
            std::make_shared<ThreadPool>(WORKERS);
    CircuitBreaker cb(
                service,duration_ms_t(DEADLINE),
                duration_ms_t(RETRY_TIME),FAILURE_THRESHOLD);
    cb.setPool(pool);
    std::future<CURLcode> fut_res;
    CURLcode code;
    /*
     * calling the same service through the circuit breaker
     */
    try {
        fut_res = cb.execute(URL_2);
        code = fut_res.get();
        LOG("Request Resutl : ", code);
    }
    catch(TimeoutError &e){
        LOG_ERROR("MAIN -- Timeout ", e.what());
    }
    catch(ServiceError &e){
        LOG_ERROR("MAIN -- Service Error : ", e.what());
    }
    /*
     * calling the service directly
     */
    try {
       code =  http_job(URL_2);
    }
    catch(ServiceError &e){
        LOG_ERROR("MAIN -- Service Error : ", e.what());
    }
    return 0;
}
