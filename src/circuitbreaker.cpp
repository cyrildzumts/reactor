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

std::shared_ptr<concurrency::Active> CircuitBreaker::getActive() const
{
    return active;
}

void CircuitBreaker::setActive(const std::shared_ptr<concurrency::Active> &value)
{
    active = value;
}

int CircuitBreaker::getUsage() const
{
    return usage;
}

void CircuitBreaker::updateFailures()
{
    ++failures;
}

int CircuitBreaker::getSuccesses() const
{
    return successes;
}

int CircuitBreaker::getFailure_threshold() const
{
    return failure_threshold;
}

double CircuitBreaker::getRatio() const
{
    return ratio;
}

int CircuitBreaker::getFailure_threshold_reached() const
{
    return failure_threshold_reached;
}

double CircuitBreaker::getRatio_trip() const
{
    return ratio_trip;
}

void CircuitBreaker::updateRatio()
{
    if(usage){
        ratio =100 * (static_cast<double>(successes)/usage);
        ratio_trip =100 * (static_cast<double>(failure_threshold_reached)/usage);
    }
}

bool CircuitBreaker::isOpen() const
{
    return current_state == CircuitBreakerOpen::instance();
}

bool CircuitBreaker::isClosed() const
{
    return current_state == CircuitBreakerClosed::instance();
}

bool CircuitBreaker::isHalfOpen() const
{
    return current_state == CircuitBreakerHalfOpen::instance();
}

void CircuitBreaker::setPool(const std::shared_ptr<ThreadPool> &value)
{
    pool = value;
}

CURLcode CircuitBreaker::fetch_sumbit(const std::string &url)
{
    CURLcode code;
    ++usage;
    updateRatio();
    //LOG("CB : ", current_state->getName(), "- DEADLINE : ", deadline.count(),TIME_UNIT, " - Request : ", request, " DELAY : ", delay, TIME_UNIT, " ratio :", ratio);
    code = current_state->fetch(this, url);
    return code;
}

CURLcode CircuitBreaker::fetch(const std::string &url)
{
    CURLcode code;
    //LOG("CB call : ", current_state->getName(), "- DEADLINE : ", deadline.count(),TIME_UNIT, " - Request : ", request, " DELAY : ", delay, TIME_UNIT, " ratio :", ratio);
#ifdef MTHREADING
    std::future<CURLcode> async_result = pool->submit(http_job, url);
#else
    std::future<CURLcode> async_result = active->submit(http_job, url);
#endif

    std::future_status status = async_result.wait_for(deadline);

    if( status == std::future_status::ready){
        try {
            code = async_result.get();
            failure_counter = 0;
            ++successes;
            updateRatio();
        } catch (...) {
            failure_time = std::chrono::system_clock::now();
            throw ;
        }
    }
    else if(status == std::future_status::timeout){
        failure_time = std::chrono::system_clock::now();
        //LOG("TIMEOUT DEADLINE : ", deadline.count(), " Delay : ", delay, " - Duration : ", duration.count(), " MAX PROCESSING : ", PROCESSING_DURATION, TIME_UNIT);
        throw  TimeoutError();
    }
    return code;
}


void CircuitBreaker::change_State(FSM *fsm_state)
{
    if(fsm_state){
        current_state = fsm_state;

/*
#ifdef DEBUG_ON
        if(current_state == CircuitBreakerClosed::instance())
            current_state->notify();
#endif
*/
    }
}

CircuitBreaker::CircuitBreaker(duration_ms_t deadline, duration_ms_t time_to_retry, int failure_threshold):
    failure_threshold{failure_threshold},time_to_retry{time_to_retry}, deadline{deadline}
{
    ratio = 0.0;
    ratio_trip = 0.0;
    current_state = CircuitBreakerClosed::instance();
    failure_counter = 0;
    failures = 0;
    successes = 0;
    usage = 0;
    failure_threshold_reached = 0;
}

CircuitBreaker::~CircuitBreaker()
{

    if(usage){
        ratio =100 * (static_cast<double>(successes)/usage);
        ratio_trip =100 * (static_cast<double>(failure_threshold_reached)/usage);
    }
#ifdef DEBUG_ON
    LOG("usage summary :\tsuccess =\t", successes, ";\terror = ", failures,
        ";\tusage : ",usage, ";\tdeadline(",TIME_UNIT,"): ",deadline.count(),
        " Success RATIO(%):", ratio, " Number of trip :", failure_threshold_reached,
        " Trip Ratio(%) :", ratio_trip);
#endif
}

void CircuitBreaker::trip()
{
    ++failure_threshold_reached;
    updateRatio();
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
    int ret = 0;
    ++usage;
    updateRatio();
    //LOG("CB : ", current_state->getName(), "- DEADLINE : ", deadline.count(),TIME_UNIT, " - Request : ", request, " DELAY : ", delay, TIME_UNIT, " ratio :", ratio);
    ret = current_state->call_service(this, request, delay);
    return ret;
}

int CircuitBreaker::call(int request, int delay)
{
    int ret = -1;
    //LOG("CB call : ", current_state->getName(), "- DEADLINE : ", deadline.count(),TIME_UNIT, " - Request : ", request, " DELAY : ", delay, TIME_UNIT, " ratio :", ratio);
#ifdef MTHREADING
    std::future<int> async_result = pool->submit(job, request, delay);
#else
    std::future<int> async_result = active->submit(job, request, delay);
#endif

    std::future_status status = async_result.wait_for(deadline);

    if( status == std::future_status::ready){
        try {
            ret = async_result.get();
            failure_counter = 0;
            ++successes;
            updateRatio();
        } catch (...) {
            failure_time = std::chrono::system_clock::now();
            throw ;
        }
    }
    else if(status == std::future_status::timeout){
        failure_time = std::chrono::system_clock::now();
        //LOG("TIMEOUT DEADLINE : ", deadline.count(), " Delay : ", delay, " - Duration : ", duration.count(), " MAX PROCESSING : ", PROCESSING_DURATION, TIME_UNIT);
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
    cbr->updateFailures();
    cbr->trip();
    auto elapsed_time_duration =std::chrono::duration_cast<duration_ms_t>(tmp - cbr->getFailure_time());
    if( elapsed_time_duration >= cbr->getTime_to_retry()){
        change_state(cbr, CircuitBreakerHalfOpen::instance());
    }
    throw  ServiceError("SYSTEM DOWN");
    return 0; // just to be conformant to the signature
}

void CircuitBreakerOpen::trip(CircuitBreaker *cbr)
{

}

void CircuitBreakerOpen::reset(CircuitBreaker *cbr)
{

}

const std::string CircuitBreakerOpen::getName()
{
    return "Open";
}

CURLcode CircuitBreakerOpen::fetch(CircuitBreaker *cbr, const std::string &url)
{
    auto tmp = std::chrono::system_clock::now();
    cbr->updateFailures();
    cbr->trip();
    auto elapsed_time_duration =std::chrono::duration_cast<duration_ms_t>(tmp - cbr->getFailure_time());
    if( elapsed_time_duration >= cbr->getTime_to_retry()){
        change_state(cbr, CircuitBreakerHalfOpen::instance());
    }
    throw  ServiceError("SYSTEM DOWN");
    return CURLE_COULDNT_CONNECT;
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
    //LOG("CB STATE : ", this->getName(), " - Request : ", request, " DELAY : ", delay, TIME_UNIT, " ratio :", cbr->getRatio());
    try {
        ret = cbr->call(request, delay);
        this->reset(cbr);
    } catch (...) {
        cbr->failure_count();
        if(cbr->getFailure_counter() > cbr->getFailure_threshold()){
            this->trip(cbr);
        }
        throw ; // rethrow to inform the caller about the error.
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

const std::string CircuitBreakerClosed::getName()
{
    return "Closed";
}

CURLcode CircuitBreakerClosed::fetch(CircuitBreaker *cbr, const std::string &url)
{
    CURLcode code;
    //LOG("CB STATE : ", this->getName(), " - Request : ", request, " DELAY : ", delay, TIME_UNIT, " ratio :", cbr->getRatio());
    try {
        code = cbr->fetch(url);
        this->reset(cbr);
    } catch (...) {
        cbr->failure_count();
        if(cbr->getFailure_counter() > cbr->getFailure_threshold()){
            this->trip(cbr);
        }
        throw ; // rethrow to inform the caller about the error.
    }
    return  code;
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
    } catch (...) {
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

const std::string CircuitBreakerHalfOpen::getName()
{
    return "Half Open";
}

CURLcode CircuitBreakerHalfOpen::fetch(CircuitBreaker *cbr, const std::string &url)
{
    CURLcode code;

    try {
        code = cbr->fetch(url);
        this->reset(cbr);
    } catch (...) {
        cbr->failure_count();
        this->trip(cbr);
        throw ;
    }
    return  code;
}

void FSM::change_state(CircuitBreaker *cbr, FSM *state)
{
    cbr->change_State(state);
}

