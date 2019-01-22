#include "threadpool.h"
#include "circuitbreaker.h"
#include <log.h> // < -- Logger library
#include <iostream>
#include <memory> // <-- Smart pointers

#define WORKERS 4
#define FAILURE_THRESHOLD 3
#define DEALINE 50
#define RETRY_TIME 70
using namespace std;

int _main_(int argc, char const *argv[])
{
    std::shared_ptr<ThreadPool> pool = std::make_shared<ThreadPool>(WORKERS);
    CircuitBreaker<decltype(http_job),std::string> cb(duration_ms_t(DEALINE), duration_ms_t(RETRY_TIME),FAILURE_THRESHOLD);
    cb.setPool(pool);
    int a = 10;
    int b = 30;
    std::future<CURLcode> fut_res;
    int res = 0;
    /*
     * calling the same service through the circuit breaker
     */
    try {
        fut_res = cb.execute(URL_2);
        res = fut_res.get();
        LOG("Request Resutl : ", res);
    }
    catch(TimeoutError &e){
        LOG_ERROR("MAIN -- Timeout ", e.what());
    }
    catch(ServiceError &e){
        LOG_ERROR("MAIN -- Service Error :  ", e.what());
    }
    /*
     * calling the service directly
     */
    try {
       res =  http_job(URL_2);
    }
    catch(ServiceError &e){
        LOG_ERROR("MAIN -- Service Error :  ", e.what());
    }
    return 0;
}
