#include "circuitbreaker.h"
#include <iostream>

FSM *CircuitBreakerClosed::root = nullptr;
FSM *CircuitBreakerOpen::root = nullptr;
FSM *CircuitBreakerHalfOpen::root = nullptr;


TimeoutError::TimeoutError():std::runtime_error("BAD REQUEST"){
    what_string = std::string("TIMEOUT REACHED");
}

const char* TimeoutError::what()const noexcept{
    return what_string.c_str();
}



int CircuitBreaker::getFailure_counter() const
{
    return failure_counter;
}

time_ms_t CircuitBreaker::getTime_to_try() const
{
    return time_to_try;
}

bool CircuitBreaker::getFailure_on_last_call() const
{
    return failure_on_last_call;
}

void CircuitBreaker::change_State(FSM *fsm_state)
{
    current_state = fsm_state;
}

CircuitBreaker::CircuitBreaker(std::shared_ptr<Service> service):first_call{true},time_to_try(50),time_to_wait(200)
{
    this->service = service;
    if(service){
        state = CBSTATE::CLOSED;
        current_state = CircuitBreakerClosed::instance();
    }
    else{
        state = CBSTATE::OPEN;
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
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    std::cout << "#########################################\n";
    int ret = -INT32_MAX;
    /*
    std::future_status status;
    auto tmp = std::chrono::system_clock::now();

        if(state == CBSTATE::CLOSED){
            std::future<int> ret_future = std::async(&Service::process_request, service, request);
            status = ret_future.wait_for(std::chrono::milliseconds(DEADLINE_TIME));
            if( status == std::future_status::ready){
                try {
                    ret = ret_future.get();
                    failure_counter = 0;
                } catch (ServiceError &e) {
                    failure_counter++;
                    state = CBSTATE::OPEN;
                    failure_time = std::chrono::system_clock::now();
                    // take some action about that error
                }
            }
            else if(status == std::future_status::timeout){
                failure_counter++;
                throw  new TimeoutError();
            }

        }
        else if( state == CBSTATE::OPEN){
            std::cout << "CIRCUIT BREAKER - Service Component is down\n"
                      << "CIRCUIT BREAKER - Please try again later \n";
            auto diff = tmp - failure_time;
            if(diff.count() >= time_to_try.count()){
                state = CBSTATE::HALF_CLOSED;
                std::cerr << "CIRCUIT BREAKER - Trying to close the circuit and let request be sent the service component\n";
            }
        }
        else if (state == CBSTATE::HALF_CLOSED){

        }
    last_call_time_point = std::chrono::system_clock::now();
    */
    ret = current_state->call_service(this, request);
    std::cout << "#########################################\n";
    return ret;
}

int CircuitBreaker::call(int request)
{
    int ret;
    std::future<int> ret_future = std::async(&Service::process_request, service, request);
    std::future_status status = ret_future.wait_for(std::chrono::milliseconds(DEADLINE_TIME));
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
        throw  ServiceError("Timeout");
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
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    //auto tmp = std::chrono::system_clock::now();
    if( cbr->getTime_to_try().count()){
        change_state(cbr, CircuitBreakerHalfOpen::instance());
    }

    throw  ServiceError("SYSTEM DOWN");
}

void CircuitBreakerOpen::trip(CircuitBreaker *cbr)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    //change_state(cbr, CircuitBreakerHalfOpen::instance());
}

void CircuitBreakerOpen::reset(CircuitBreaker *cbr)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
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
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    int ret;
    try {
        ret = cbr->call(request);
        this->reset(cbr);
    } catch (ServiceError &e) {
        cbr->failure_count();
        if(cbr->getFailure_counter() > FAILURE_LIMIT){
            std::cout << "FAILURE RATE LIMITE REACHED" << std::endl;
            this->trip(cbr);
        }
        throw e;
    }
    return  ret;
}

void CircuitBreakerClosed::trip(CircuitBreaker *cbr)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    cbr->trip();
    change_state(cbr, CircuitBreakerOpen::instance());
}

void CircuitBreakerClosed::reset(CircuitBreaker *cbr)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    cbr->reset();
    //change_state(cbr, this);
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
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    int ret;
    try {
        ret = cbr->call(request);
        this->reset(cbr);
    } catch (ServiceError &e) {
        cbr->failure_count();
        this->trip(cbr);
        throw e;
    }
    return  ret;
}

void CircuitBreakerHalfOpen::trip(CircuitBreaker *cbr)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    cbr->trip();
    change_state(cbr, CircuitBreakerOpen::instance());
}

void CircuitBreakerHalfOpen::reset(CircuitBreaker *cbr)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    cbr->reset();
    change_state(cbr, CircuitBreakerClosed::instance());
}

void FSM::change_state(CircuitBreaker *cbr, FSM *state)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    cbr->change_State(state);
}
