

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
using namespace std;

int constexpr DELAY_500_MS  = 500;
int constexpr DELAY_50_MS  = 50;
int constexpr DELAY_100_MS  = 100;
int constexpr MAX_REQUEST =  4;


int print_task(int a){
    LOG ("TASK Run by ", std::this_thread::get_id());
    return a * 10;
}

int main(int argc, char const *argv[])
{
    std::cout << "Reactor: Circuit Breaker " << std::endl;
    int r = 0;
    LOG("Reactor Main entry point ", "thread id : ", std::this_thread::get_id());
    ActuatorController actuators;
    std::vector<std::future<int>> results(MAX_REQUEST);
    std::vector<std::future<int>> a_r(MAX_REQUEST);
    std::thread actuator_thread (&ActuatorController::run, &actuators);
    //LOG("Reactor Main : actuators thread id : ", actuator_thread.get_id());
    ThreadPool pool;
    AbstractActive active;
    //pool.submit(actuators);
    for(size_t i = 0; i < MAX_REQUEST; i++){
        results[i] = pool.submit(std::bind(print_task, i));
        a_r[i] = active.submit(std::bind(print_task, i));
    }
    //LOG("Reactor Main ", " sending signal to ActuatorController " " thread id : ", std::this_thread::get_id(), " controller : ", actuators.getActuator_controller_id());
    actuators.setQuit(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
/*     std::shared_ptr<Service> service(new ConcreteService());
    CircuitBreaker cb{service};
    for(int i = 0; i < MAX_REQUEST; i++){
        try {
            cb.process_request(i-5);
            std::this_thread::sleep_for(duration_ms_t(DELAY_500_MS));
        }
        catch(TimeoutError &e){
            LOG("MAIN -- Timeout ");
        }
        catch(ServiceError &e){
            LOG("MAIN -- Service Error :  ", e.what());
        }
        catch(std::future_error &e){
            LOG("MAIN Future Error : ", e.what());
        }
        
    } */
    for(size_t i = 0; i < MAX_REQUEST; i++){
        try {
            r = results[i].get();
            LOG("MAIN:: RESULTS ",i, " : ", r );
            r = a_r[i].get();
            LOG("MAIN:: Active RESULTS ",i, " : ", r );

        }
        catch(std::future_error &e){
            LOG("MAIN Future Results Error : ", e.what());
        }

        
    }
    if(actuator_thread.joinable()){
        actuator_thread.join();
    }
    return 0;
}
