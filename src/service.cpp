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


ConcreteService::ConcreteService()

{
   service_resource_usage = 0;
}


int ConcreteService::process_request(int request, int delay)
{

    if(delay > PROCESSING_DURATION || delay < 0){
        LOG("Service DELAY ", delay);
        throw ServiceError("Service: Bad delay argument: " + std::to_string(delay));
    }
    // simulate the time required to process the request
    std::this_thread::sleep_for(duration_ms_t(delay));

    return delay;
}




int ConcreteService::operator ()(int request, int delay)
{
    // simulate the time required to process the request
    std::this_thread::sleep_for(duration_ms_t(delay));
    return delay;
}

int job(int req, int delay)
{
    /*
     * this function sleeps to simulate a processing duration
     */

    if(delay > PROCESSING_DURATION || delay < 0 ){
        throw ServiceError("Service: Bad delay argument:  " + std::to_string(delay));
    }
   //std::cout <<"Job Request : " << req<< " - delay :" << delay << TIME_UNIT <<std::endl;
    std::this_thread::sleep_for(duration_ms_t(delay));
    return delay + req;
}
