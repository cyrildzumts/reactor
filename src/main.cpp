#include "circuitbreaker.h"
#include <log.h> // < -- Logger library
#include <iostream>
#include <memory> // <-- Smart pointers

using namespace std;

int constexpr MAX_REQUEST = 5;




int main(int argc, char const *argv[])
{
    LOG("Reactor: Circuit Breaker ");
    //std::shared_ptr<Service> service{new ConcreteService()};
    std::unique_ptr<Service> service{new ConcreteService()};
    CircuitBreaker cb(duration_ms_t(30), duration_ms_t(100), 5 );

    /*
     * calling the same service through the circuit breaker
     */
    try {
        cb.process_request(MAX_REQUEST);
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
        service->process_request(MAX_REQUEST);
    }
    catch(ServiceError &e){
        LOG_ERROR("MAIN -- Service Error :  ", e.what());
    }

    return 0;
}
