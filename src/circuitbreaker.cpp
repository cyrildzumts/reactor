#include "circuitbreaker.h"
#include <iostream>

CircuitBreaker::CircuitBreaker(std::shared_ptr<Service> service):first_call{true},time_to_try(50),time_to_wait(200)
{
    this->service = service;
    if(service){
        state = CBSTATE::CLOSED;
    }
    else{
        state = CBSTATE::OPEN;
    }
}


int CircuitBreaker::process_request(int request)
{
    int ret = -INT32_MAX;
    std::future_status status;
    if(first_call){
        last_call_time_point = std::chrono::system_clock::now();
    }
    auto tmp = std::chrono::system_clock::now() - last_call_time_point;
    //std::cout << "CIRCUIT BREAKER - current time tmp : " << tmp.count() << '\n';
    //std::cout << "CIRCUIT BREAKER - current time to wait : " << time_to_wait.count() << '\n';
    if(tmp.count() >= time_to_wait.count()){

        if(state == CBSTATE::CLOSED){
            std::future<int> ret_future = std::async(&Service::process_request, service, request);
            status = ret_future.wait_for(std::chrono::milliseconds(100));
            if( status == std::future_status::ready){
                try {
                    ret = ret_future.get();
                    std::cout << "******************************************************************\n";
                    std::cerr << "CIRCUIT BREAKER - Service Component has processed your request\n";
                    std::cout << "******************************************************************\n";
                } catch (ServiceError &e) {
                    std::cout << "CIRCUIT BREAKER - Service Component could not process your request\n"
                              << "CIRCUIT BREAKER - Error : " << e.what() << '\n';
                    state = CBSTATE::OPEN;
                    failure_time = std::chrono::system_clock::now();
                }
            }

        }
        else if( state == CBSTATE::OPEN){
            std::cout << "CIRCUIT BREAKER - Service Component is down\n"
                      << "CIRCUIT BREAKER - Please try again later \n";
            auto tmp = std::chrono::system_clock::now() - failure_time;
            if(tmp.count() >= time_to_try.count()){
                state = CBSTATE::CLOSED;
                std::cerr << "CIRCUIT BREAKER - Trying to close the circuit and let request be sent the service component\n";
            }
        }
    }
    else{
        std::cout << "CIRCUIT BREAKER -  Client is sending request to fast. Please slow down\n";
    }
    last_call_time_point = std::chrono::system_clock::now();
    return ret;
}


