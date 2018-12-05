
//#define LOG_LEVEL_1

#include "activeobject.h"
#include "actuators.h"
#include <string>
#include <iostream>
#include <memory>
#include "circuitbreaker.h"
#include "client.h"
#include "threadpool.h"
#include <thread>
#include <log.h>

std::thread client1;
int constexpr DELAY_500_MS  = 500;
int constexpr MAX_REQUEST =  3;
using namespace std;

int print_task(int a){
    LOG ("TASK Run by ", std::this_thread::get_id());
    return a * 10;
}

int main(int argc, char const *argv[])
{
    std::cout << "Reactor: Circuit Breaker " << std::endl;
    LOG("Reactor Main entry point ", "thread id : ", std::this_thread::get_id());
    //ActuatorController actuators;
    std::vector<std::future<int>> results(MAX_REQUEST*2);
    //std::thread actuator_thread (&ActuatorController::run, &actuators);
    //LOG("Reactor Main : actuators thread id : ", actuator_thread.get_id());
    ThreadPool pool;
    AbstractActive active;
    int r = 0;
    //pool.submit(actuators);
    for(size_t i = 0; i < MAX_REQUEST*2; i++){
        results[i] = pool.submit(std::bind(print_task, i));
        results[i+1] = active.submit(std::bind(print_task, i));
    }
    //LOG("Reactor Main ", " sending signal to ActuatorController " " thread id : ", std::this_thread::get_id(), " controller : ", actuators.getActuator_controller_id());
    //actuators.setQuit(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
//    std::shared_ptr<Service> service(new ConcreteService());
//    CircuitBreaker cb{service};
//    for(int i = 0; i < MAX_REQUEST; i++){
//        cb.process_request(i-5);
//        std::this_thread::sleep_for(time_ms_t(DELAY_500_MS));
//    }
    for(size_t i = 0; i < MAX_REQUEST*2; i++){
        r = results[i].get();
        LOG("MAIN:: RESULTS ",i, " : ", r );
    }
    return 0;
}
