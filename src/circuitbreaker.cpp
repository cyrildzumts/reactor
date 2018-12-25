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

void CircuitBreaker::addOnCircuitBreakClosedObserver(FunctionWrapper observer)
{
    CircuitBreakerClosed::instance()->addObservers(std::move(observer));
}

void CircuitBreaker::addOnCircuitBreakOpenObserver(FunctionWrapper observer)
{
    CircuitBreakerOpen::instance()->addObservers(std::move(observer));
}

void CircuitBreaker::addOnCircuitBreakHalfOpenObserver(FunctionWrapper observer)
{
    CircuitBreakerHalfOpen::instance()->addObservers(std::move(observer));
}


int CircuitBreaker::getFailures() const
{
    return failures;
}

void CircuitBreaker::change_State(FSM *fsm_state)
{
    if(fsm_state){
        current_state = fsm_state;
#ifdef DEBUG_ON
        current_state->notify();
#endif
    }
    else{
        LOG_WARN("Circuit Breaker : change_state() called with nullptr. Nothing done");
    }
}

CircuitBreaker::CircuitBreaker():time_to_retry{1000us}
{
    service = std::make_shared<ConcreteService>();
    if(service){
        current_state = CircuitBreakerClosed::instance();
    }
    else{
        current_state = CircuitBreakerOpen::instance();
    }
    failure_counter = 0;
    failures = 0;
}

CircuitBreaker::CircuitBreaker(duration_ms_t deadline, duration_ms_t time_to_retry, int failure_threshold):
    failure_threshold{failure_threshold},time_to_retry{time_to_retry}, deadline{deadline}
{
    service = std::make_shared<ConcreteService>();
    current_state = CircuitBreakerClosed::instance();
    failure_counter = 0;
    failures = 0;
}

CircuitBreaker::~CircuitBreaker()
{
#ifdef DEBUG_ON
    LOG("Circuit Breaker Deleted");
    if(service){
        LOG("Unique pointer still valide");
    }else{
        LOG("Unique pointer invalide");
    }
#endif
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
    failures++;
}


int CircuitBreaker::process_request(int request, int delay)
{
    int ret = -INT32_MAX;
    ret = current_state->call_service(this, request, delay);
    return ret;
}

int CircuitBreaker::call(int request, int delay)
{
    int ret = -1;
    auto start = std::chrono::system_clock::now();
    std::future<int> ret_future = std::async(std::launch::async,&Service::process_request, service, request, delay);
    auto end = std::chrono::system_clock::now();
    std::future_status status = ret_future.wait_for(std::chrono::microseconds(deadline));
    //std::future_status status = ret_future.wait_for(std::chrono::microseconds(150us));
    auto end2 = std::chrono::system_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - end).count();
    ret = ret_future.get();
    LOG("CircuitBreaker::call() : async call : ", duration1, " deadline required : ", deadline.count(), " delay provided : ", delay);
    LOG("CircuitBreaker::call() : time to get future : ", duration2, " timeout : " , status == std::future_status::timeout);
    LOG("Future Result : ", ret);
    exit(-1);
    if( status == std::future_status::ready){
        try {
            ret = ret_future.get();
            failure_counter = 0;
        } catch (...) {
            failure_time = std::chrono::system_clock::now();
            throw ;
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
        root->addObservers([](){
            LOG("Circuit Breaker open ...");
        });
    }
    return root;
}

int CircuitBreakerOpen::call_service(CircuitBreaker *cbr, int request, int delay)
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
        root->addObservers([](){
            LOG("Circuit Breaker closed ...");
        });
    }
    return root;
}

int CircuitBreakerClosed::call_service(CircuitBreaker *cbr, int request, int delay)
{
    int ret;
    try {
        ret = cbr->call(request, delay);
        this->reset(cbr);
    } catch (ServiceError &e) {
        cbr->failure_count();
        if(cbr->getFailure_counter() > FAILURE_LIMIT){
            this->trip(cbr);
        }
        throw ; // rethrow to inform the caller about the error.
    }catch(TimeoutError &t){
        cbr->failure_count();
        if(cbr->getFailure_counter() > FAILURE_LIMIT){
            this->trip(cbr);
        }
        throw; // rethrow to inform the caller about the error.
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
        root->addObservers([](){
            LOG("Circuit Breaker Half-Open ...");
        });
    }
    return root;
}

int CircuitBreakerHalfOpen::call_service(CircuitBreaker *cbr, int request, int delay)
{
    int ret;
    try {
        ret = cbr->call(request, delay);
        this->reset(cbr);
    } catch (ServiceError &e) {
        cbr->failure_count();
        this->trip(cbr);
        throw ;
    }catch (TimeoutError &e) {
        cbr->failure_count();
        this->trip(cbr);
        throw ;
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
