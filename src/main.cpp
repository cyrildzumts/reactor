
#include <string>
#include <iostream>
#include <memory>
#include "circuitbreaker.h"
#include <thread>
int constexpr DELAY_500_MS  = 500;
int constexpr MAX_REQUEST =  30;
using namespace std;

int main(int argc, char const *argv[])
{
    std::cout << "Hello Reactor" << std::endl;

    std::shared_ptr<Service> service(new ConcreteService());
    CircuitBreaker cb{service};
    for(int i = 0; i < MAX_REQUEST; i++){
        cb.process_request(i-5);
        std::this_thread::sleep_for(time_ms_t(DELAY_500_MS));
    }

    
    return 0;
}
