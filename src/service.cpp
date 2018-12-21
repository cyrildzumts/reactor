#include "service.h"
#include <time.h>
#include <iostream>
#include <thread>
#include <chrono>

ServiceError::ServiceError():std::runtime_error("BAD REQUEST"){
    what_string = std::string("BAD REQUEST");
}

ServiceError::ServiceError(const char* what_args):std::runtime_error(what_args){
    what_string = std::string(what_args);
}

ServiceError::ServiceError(const std::string& what_args):std::runtime_error(what_args){
    what_string = std::string(what_args);
}

const char* ServiceError::what()const noexcept{
    return what_string.c_str();
}



<<<<<<< HEAD
ConcreteService::ConcreteService(std::optional<int> wait_time): duration{0}
=======
ConcreteService::ConcreteService()
>>>>>>> trunk
{
    average_duration = wait_time ? *wait_time : 20;
    LOG("Service component average time : ", average_duration , " ms");

}


int ConcreteService::process_request(int request, int delay)
{
<<<<<<< HEAD
    //duration = Generator<20>::instance()->generate();
    int total = duration + delay;
    //LOG("Service DURATION : ", total, "ms");
    //service_resource_usage++;
=======
>>>>>>> trunk
    // simulate the time required to process the request
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
//    if((total) > average_duration){
//        throw ServiceError("SYSTEM::ERROR: " + std::to_string(total));
//    }
    return total ;
}


