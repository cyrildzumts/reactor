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


ConcreteService::ConcreteService(std::optional<int> wait_time): duration{0}

{
    average_duration = wait_time ? *wait_time : 20;
    LOG("Service component average time : ", average_duration , " ms");

}


int ConcreteService::process_request(int request, int delay)
{
    // simulate the time required to process the request
    std::this_thread::sleep_for(std::chrono::microseconds(delay));
//    if(delay > average_duration){
//        throw ServiceError("SYSTEM::ERROR: " + std::to_string(delay));
//    }
    return delay;
}


