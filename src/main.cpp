

//#define LOG_LEVEL_1
#include "circuitbreaker.h"
#include <log.h>
#include <iostream>
#include <memory>

using namespace std;

int constexpr MAX_REQUEST = 5;

int main(int argc, char const *argv[])
{
    std::cout << "Reactor: Circuit Breaker " << std::endl;
    LOG("Reactor Main entry point ", "thread id : ", std::this_thread::get_id());
    std::shared_ptr<Service> service{new ConcreteService()};
    CircuitBreaker cb{service};
    try {
        service->process_request(MAX_REQUEST);
        LOG("service working ...");
    }
    catch(ServiceError &e){
        LOG_ERROR("MAIN -- Service Error :  ", e.what());
    }

    // calling the same service through the circuit breaker
    try {
        cb.process_request(MAX_REQUEST);
    }
    catch(TimeoutError &e){
        LOG_ERROR("MAIN -- Timeout ", e.what());
    }
    catch(ServiceError &e){
        LOG_ERROR("MAIN -- Service Error :  ", e.what());
    }
    return 0;
}
