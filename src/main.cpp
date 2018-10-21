
#include <string>
#include <iostream>
#include <memory>
#include "circuitbreaker.h"
#include <thread>
using namespace std;

int main(int argc, char const *argv[])
{
    std::cout << "Hello Reactor" << std::endl;

    std::shared_ptr<Service> service(new ConcreteService());
    CircuitBreaker cb{service};
    for(int i = 0; i <30; i++){
        cb.process_request(i-5);
        std::this_thread::sleep_for(time_ms_t(500));
    }

    
    return 0;
}
