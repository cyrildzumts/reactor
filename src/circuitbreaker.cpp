#include "circuitbreaker.h"
#include <iostream>

FSM *CircuitBreakerClosed::root = nullptr;
FSM *CircuitBreakerOpen::root = nullptr;
FSM *CircuitBreakerHalfOpen::root = nullptr;


TimeoutError::TimeoutError():std::runtime_error("TIMEOUT"){
    //what_string = std::string("TIMEOUT REACHED");
}

const char* TimeoutError::what()const noexcept{
    return "TIMEOUT";
}



int CircuitBreaker::getFailure_counter() const
{
    return failure_counter;
}

duration_ms_t CircuitBreaker::getTime_to_retry() const
{
    return time_to_retry;
}


time_point_ms_t CircuitBreaker::getFailure_time() const
{
    return failure_time;
}

void CircuitBreaker::change_State(FSM *fsm_state)
{
    current_state = fsm_state;
}

CircuitBreaker::CircuitBreaker(std::shared_ptr<Service> service):time_to_retry{1000ms}
{
    this->service = service;
    if(service){
        current_state = CircuitBreakerClosed::instance();
    }
    else{
        current_state = CircuitBreakerOpen::instance();
    }
    failure_counter = 0;
}

void CircuitBreaker::trip()
{

}

void CircuitBreaker::reset()
{
    failure_counter = 0;
}

void CircuitBreaker::failure_count()
{
    failure_counter++;
}


int CircuitBreaker::process_request(int request)
{
    int ret = -INT32_MAX;
    ret = current_state->call_service(this, request);
    return ret;
}

int CircuitBreaker::call(int request)
{
    int ret;
    std::future<int> ret_future = std::async(&Service::process_request, service, request);
    std::future_status status = ret_future.wait_for(std::chrono::microseconds(DEADLINE_TIME));
    if( status == std::future_status::ready){
        try {
            ret = ret_future.get();
            failure_counter = 0;
        } catch (ServiceError &e) {
            failure_time = std::chrono::system_clock::now();
            throw  e;
        }
        
    }
    else if(status == std::future_status::timeout){
        failure_time = std::chrono::system_clock::now();
        throw  TimeoutError();
    }
    return ret;
}



FSM *CircuitBreakerOpen::instance()
{
    if(!root){
        root = new CircuitBreakerOpen;
    }
    return root;
}

int CircuitBreakerOpen::call_service(CircuitBreaker *cbr, int request)
{
    auto tmp = std::chrono::system_clock::now();
    auto elapsed_time_duration =std::chrono::duration_cast<duration_ms_t>(tmp - cbr->getFailure_time());
    auto left_time = cbr->getTime_to_retry() - elapsed_time_duration;
    if( left_time >= 0ms){
        change_state(cbr, CircuitBreakerHalfOpen::instance());
    }
    throw  ServiceError("SYSTEM DOWN");
}

void CircuitBreakerOpen::trip(CircuitBreaker *cbr)
{
    //change_state(cbr, CircuitBreakerHalfOpen::instance());
}

void CircuitBreakerOpen::reset(CircuitBreaker *cbr)
{
    //change_state(cbr, CircuitBreakerClosed::instance());
}



FSM *CircuitBreakerClosed::instance()
{
    if(!root){
        root = new CircuitBreakerClosed;
    }
    return root;
}

int CircuitBreakerClosed::call_service(CircuitBreaker *cbr, int request)
{
    int ret;
    try {
        ret = cbr->call(request);
        this->reset(cbr);
    } catch (ServiceError &e) {
        cbr->failure_count();
        if(cbr->getFailure_counter() > FAILURE_LIMIT){
            this->trip(cbr);
        }
        throw e; // rethrow to inform the caller about the error.
    }catch(TimeoutError &t){
        cbr->failure_count();
        if(cbr->getFailure_counter() > FAILURE_LIMIT){
            this->trip(cbr);
        }
        throw t;// rethrow to inform the caller about the error.
    }
    return  ret;
}

void CircuitBreakerClosed::trip(CircuitBreaker *cbr)
{
    cbr->trip();
    change_state(cbr, CircuitBreakerOpen::instance());
}

void CircuitBreakerClosed::reset(CircuitBreaker *cbr)
{
    cbr->reset();
}


FSM *CircuitBreakerHalfOpen::instance()
{
    if(!root){
        root = new CircuitBreakerHalfOpen;
    }
    return root;
}

int CircuitBreakerHalfOpen::call_service(CircuitBreaker *cbr, int request)
{
    int ret;
    try {
        ret = cbr->call(request);
        this->reset(cbr);
    } catch (ServiceError &e) {
        cbr->failure_count();
        this->trip(cbr);
        throw e;
    }catch (TimeoutError &e) {
        cbr->failure_count();
        this->trip(cbr);
        throw e;
    }
    return  ret;
}

void CircuitBreakerHalfOpen::trip(CircuitBreaker *cbr)
{
    cbr->trip();
    change_state(cbr, CircuitBreakerOpen::instance());
}

void CircuitBreakerHalfOpen::reset(CircuitBreaker *cbr)
{
    cbr->reset();
    change_state(cbr, CircuitBreakerClosed::instance());
}

void FSM::change_state(CircuitBreaker *cbr, FSM *state)
{
    cbr->change_State(state);
}
