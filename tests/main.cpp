#ifndef BOOST_TEST_DYN_LINK
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE "CIRCUIBREAKER TEST"


#include "circuitbreaker.h"
#include "service.h"

#include <memory>
#include <boost/test/unit_test.hpp>

#define WORKERS 1
#define FAILURE_THRESHOLD 3
#define DEADLINE 50
#define RETRY_TIME 70

using CMD = Command<int,const int&, const int&>;
using CBreaker = CircuitBreaker<int, const int&, const int&>;


struct CircuitBreakerFixture{

    std::shared_ptr<ThreadPool> pool;
    std::shared_ptr<CBreaker> breaker;
    CircuitBreakerFixture(){
        pool = std::make_shared<ThreadPool>(WORKERS);
        Command s(job);

        breaker = std::make_shared<CBreaker> (s, duration_ms_t(DEADLINE), duration_ms_t(RETRY_TIME),FAILURE_THRESHOLD);
        breaker->setPool(pool);
    }

    ~CircuitBreakerFixture(){
        pool->interrupt();
    }
};

/**
 * @brief BOOST_FIXTURE_TEST_CASE This test case checks if the circuit breaker
 * is properly initialised.
 */
BOOST_FIXTURE_TEST_CASE(CIRCUITBREAKER_INIT, CircuitBreakerFixture){

    BOOST_CHECK(breaker->isClosed());
    BOOST_CHECK(!breaker->isOpen());
    BOOST_CHECK(!breaker->isHalfOpen());

    BOOST_CHECK_EQUAL(0, breaker->getFailures());
    BOOST_CHECK_EQUAL(0, breaker->getSuccesses());
    BOOST_CHECK_EQUAL(FAILURE_THRESHOLD, breaker->getFailure_threshold());
    BOOST_CHECK_EQUAL(RETRY_TIME, breaker->getTime_to_retry().count());
    BOOST_CHECK_EQUAL(DEADLINE, breaker->getDeadline().count());
    BOOST_CHECK_EQUAL(0, breaker->getUsage());
}


/**
 * @brief BOOST_FIXTURE_TEST_CASE This test case checks if the
 * the circuit breaker correctly return the result from the service.
 */
BOOST_FIXTURE_TEST_CASE(CIRCUITBREAKER_SERVICE_RESULTS_TEST, CircuitBreakerFixture){
    int res = 0;
    int a = 10;
    int b = 40;
    int usage = 0;
    int failures = 0;
    int successes = 0;
    std::vector<std::future<int>>results_fut;
    std::vector<int> a_args;
    std::vector<int> awaited_results;
    for(int i = 0; i < 10; i++){
        a_args.push_back(Generator<100>::instance()->generate());
    }
    int awaited_res = job(a, b);
    auto res_fut = breaker->execute(a,b);
    successes++;
    usage++;
    try {
        res = res_fut.get();
        BOOST_CHECK_EQUAL(awaited_res, res);
        BOOST_CHECK_EQUAL(failures, breaker->getFailures());
        BOOST_CHECK_EQUAL(successes, breaker->getSuccesses());
        BOOST_CHECK_EQUAL(usage, breaker->getUsage());
    } catch (std::exception &e) {
        res = 0;
    }

    std::for_each(a_args.begin(), a_args.end(),[&](int arg){
        awaited_results.push_back(job(arg, b));
        results_fut.push_back(breaker->execute(arg,b));
        usage++;
        successes++;
    });

    bool is_equal = std::equal(awaited_results.begin(), awaited_results.end(), results_fut.begin(),[](int a, std::future<int> &fut){
        bool f = false;
        try {
            f = a == fut.get();
        } catch (std::exception &) {
            f = false;
        }
        return f;
    });
    BOOST_CHECK(is_equal);
    BOOST_CHECK_EQUAL(failures, breaker->getFailures());
    BOOST_CHECK_EQUAL(successes, breaker->getSuccesses());
    BOOST_CHECK_EQUAL(usage, breaker->getUsage());
}


/**
 * @brief BOOST_FIXTURE_TEST_CASE This test case checks if the
 * the circuit breaker correctly return the result from the service.
 */
BOOST_FIXTURE_TEST_CASE(CIRCUITBREAKER_FAILURES_TEST, CircuitBreakerFixture){
    int res = 0;
    int a = 10;
    int b = 40;
    int usage = 0;
    int failures = 0;
    int successes = 0;
    std::vector<std::future<int>>results_fut;
    std::vector<int> args;
    for(int i = 0; i < FAILURE_THRESHOLD; i++){
        args.push_back(DEADLINE);
    }

    std::for_each(args.begin(), args.end(),[&](int arg){
        results_fut.push_back(breaker->execute(arg, arg));
        usage++;
        failures++;
    });
//    std::for_each(results_fut.begin(), results_fut.end(),[](std::future<int> &f){
//        f.wait();
//    });
    BOOST_CHECK_EQUAL(failures, breaker->getFailures());
    BOOST_CHECK_EQUAL(successes, breaker->getSuccesses());
    BOOST_CHECK_EQUAL(usage, breaker->getUsage());

    BOOST_CHECK(!breaker->isClosed());
    BOOST_CHECK(breaker->isOpen());
    BOOST_CHECK(!breaker->isHalfOpen());


}

/**
 * @brief BOOST_FIXTURE_TEST_CASE This test case checks if the circuit breaker
 * correctly forward the exception thrown from the service.
 * The service throws an ServiceError Exception when the delay
 * parameter has a value delay < 0 || delay > PROCESSING_DURATION
 */
BOOST_FIXTURE_TEST_CASE(CIRCUITBREAKER_TEST_SERVICE_EXCEPTION, CircuitBreakerFixture){
    int res = 0;
    int a = 10;
    int b = PROCESSING_DURATION + 1;
    auto future_res = breaker->execute(a, b);
    auto is_critical = [](const ServiceError &e){
        return false;
    };
    BOOST_CHECK_THROW(future_res.get(),ServiceError);

}

/**
 * @brief BOOST_FIXTURE_TEST_CASE This test case checks whether
 * a TiemoutError Exception is thrown when the deadline is reached.
 *
 */
BOOST_FIXTURE_TEST_CASE(CIRCUITBREAKER_TIMEOUT_EXCEPTION, CircuitBreakerFixture){
    int res = 0;
    int a = 10;
    int b =  DEADLINE + 1;
    auto future_res = breaker->execute(a, b);
    auto is_critical = [](const TimeoutError &e){
        return false;
    };
    BOOST_CHECK_THROW(future_res.get(), TimeoutError);
}

/**
 * @brief BOOST_FIXTURE_TEST_CASE This Unit test case checks
 * for conformance. The Circuit breaker is checked to see whether or not
 * it changes it state accordingly to the errors or success from the
 * service.
 */
BOOST_FIXTURE_TEST_CASE(CIRCUITBREAKER_TRANSITIONS_CLOSED_OPEN_HALF_CLOSED, CircuitBreakerFixture){
    int res = 0;
    int a = 10;
    int b =  DEADLINE + 1;
    int usage = 0;
    int failures = 0;
    int successes = 0;
    std::vector<std::future<int>>results_fut;
    std::vector<int> args;
    for(int i = 0; i < FAILURE_THRESHOLD; i++){
        args.push_back(DEADLINE);
    }

    auto callable =[&](int arg){
        results_fut.push_back(breaker->execute(arg, arg));
        usage++;
        failures++;
    };

    auto waiter_f = [](std::future<int> &f){
        f.wait();
    };

    BOOST_CHECK(breaker->isClosed());

    /**
     * Send a number of request which will all fail,
     * It is enough when sending a request equivalent to the failure threshold.
     */
    std::for_each(args.begin(), args.end(),callable);
    std::for_each(results_fut.begin(), results_fut.end(),waiter_f);


    BOOST_CHECK(breaker->isOpen());

    // after a duration of RETRY_TIME ms, the circuit breaker
    // transition to half-open state after it has received a request.
    std::this_thread::sleep_for(duration_ms_t(RETRY_TIME));

    

    // the circuit breaker must still be in Open state first.
    BOOST_CHECK(breaker->isOpen());
    /**
     * sending a request now will fail but the goal is make the circuit breaker 
     * notices that the time to retry timedout so that it transitions to the half-open
     * state.
     */ 
    auto dummy_fut = breaker->execute(a, b);
    BOOST_CHECK(breaker->isHalfOpen());

    /** Sending a request has 2 possible results:
     * On success, the circuit breaker transitions to Closed state.
     * On faillure, the circuit breaker transitions back to Open state.
     */ 
    b = 30;
    int awaited_result = job(a, b);
    auto success_fut = breaker->execute(a,b);
    try
    {
        res = success_fut.get();
    }
    catch(const std::exception& e)
    {
        res = 0;
    }
    BOOST_CHECK_EQUAL(awaited_result, res);
    BOOST_CHECK(breaker->isClosed());

}

BOOST_FIXTURE_TEST_CASE(CIRCUITBREAKER_TRANSITIONS_CLOSED_OPEN_HALF_OPEN, CircuitBreakerFixture){
    int res = 0;
    int a = 10;
    int b =  DEADLINE + 10;
    int usage = 0;
    int failures = 0;
    int successes = 0;
    std::vector<std::future<int>>results_fut;
    std::vector<int> args;
    for(int i = 0; i < FAILURE_THRESHOLD; i++){
        args.push_back(b);
    }

    auto callable =[&](int arg){
        results_fut.push_back(breaker->execute(arg, arg));
        usage++;
        failures++;
    };

    auto waiter_f = [](std::future<int> &f){
        f.wait();
    };
    BOOST_CHECK(breaker->isClosed());

    std::for_each(args.begin(), args.end(),callable);
    std::for_each(results_fut.begin(), results_fut.end(),waiter_f);

    BOOST_CHECK(breaker->isOpen());

    // after a duration of RETRY_TIME ms, the circuit breaker
    // transition to half-open state after it has received a request.
    std::this_thread::sleep_for(duration_ms_t(RETRY_TIME));

    // the circuit breaker must still be in Open state first.
    BOOST_CHECK(breaker->isOpen());
    /**
     * sending a request now will fail but the goal is make the circuit breaker 
     * notices that the time to retry timedout so that it transitions to the half-open
     * state.
     */ 
    auto dummy_fut = breaker->execute(a, b);
    BOOST_CHECK(breaker->isHalfOpen());

    /** Sending a request has 2 possible results:
     * On success, the circuit breaker transitions to Closed state.
     * On faillure, the circuit breaker transitions back to Open state.
     */ 
    auto success_fut = breaker->execute(a,b);

    BOOST_CHECK(breaker->isOpen());

}

