
#include <string>
#include <iostream>
#include <memory>
#include "circuitbreaker.h"
#include <thread>
int constexpr DELAY_500_MS  = 500;
int constexpr MAX_REQUEST =  50;
using namespace std;

int main(int argc, char const *argv[])
{
    std::cout << "Hello Reactor" << std::endl;
    int res = 0;
    std::shared_ptr<Service> service(new ConcreteService());
    CircuitBreaker cb{service};
    for(int i = 0; i < MAX_REQUEST; i++){
        try {
            res = cb.process_request(i);
            std::cout << "SERVICE::SUCCESS : " << res << "\n" << std::endl;
        } catch (ServiceError &e) {
            std::cout << e.what() << "\n";
        }
        std::this_thread::sleep_for(time_ms_t(DELAY_500_MS));
    }
    return 0;
}
