#include "circuitbreaker.h"
#include <log.h> // < -- Logger library
#include <iostream>
#include <memory> // <-- Smart pointers

using namespace std;
int constexpr REQUEST = 100; // dummy request

int main(int argc, char const *argv[])
{
    std::shared_ptr<concurrency::Active> active = std::make_shared<concurrency::Active>();
    std::unique_ptr<Service> service = std::make_unique<ConcreteService>();
    CircuitBreaker cb(duration_ms_t(120), duration_ms_t(100), 5 );
    cb.setActive(active);
    /*
     * calling the same service through the circuit breaker
     */
    try {
        cb.process_request(REQUEST, REQUEST);
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
        service->process_request(REQUEST, REQUEST);
    }
    catch(ServiceError &e){
        LOG_ERROR("MAIN -- Service Error :  ", e.what());
    }
    return 0;
}
