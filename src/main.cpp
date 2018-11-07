#include <string>
#include <iostream>
#include <memory>
#include "circuitbreaker.h"
#include <thread>

using namespace std;

int constexpr DELAY_500_MS  = 500;
int constexpr DELAY_50_MS  = 50;
int constexpr DELAY_100_MS  = 100;
int constexpr MAX_REQUEST =  50;


int main(int argc, char const *argv[])
{
    int res = 0;
    std::shared_ptr<Service> service(new ConcreteService());
    CircuitBreaker cb{service};
    for(int i = 0; i < MAX_REQUEST; i++){
        try {
            res = cb.process_request(i);
            std::cout << "SERVICE::SUCCESS : " << res << "\n" << std::endl;
        } catch (ServiceError &e) {
            std::cout << e.what() << "\n";
        } catch(TimeoutError &t){
            std::cout << t.what() << "\n";
        }
        std::this_thread::sleep_for(duration_ms_t(DELAY_100_MS));
    }
    return 0;
}
