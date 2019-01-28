 //#define BOOST_TEST_DYN_LINK
 #define BOOST_TEST_MODULE "BaseClassModule"


#include "circuitbreaker.h"
#include "service.h"

#include <memory>
#include <boost/test/unit_test.hpp>

#define WORKERS 4
#define FAILURE_THRESHOLD 3
#define DEADLINE 50
#define RETRY_TIME 70




class TestObject{
public:
    std::shared_ptr<ThreadPool> pool;

    int count;

public:
    TestObject(){
        pool = std::make_shared<ThreadPool>(WORKERS);
    }

    ~TestObject(){
        pool->interrupt();
    }



    int cb_compute_ok(int a, int b){
        int ret = 0;

        return ret;
    }
};

BOOST_AUTO_TEST_CASE(CIRCUIBREAKERSTATES){

    TestObject t1;
    Command s(job);
    //Command2 s2(job);
    //CircuitBreaker cbs(s2,duration_ms_t(DEADLINE), duration_ms_t(RETRY_TIME),FAILURE_THRESHOLD);
    CircuitBreaker cbreaker(s,duration_ms_t(DEADLINE), duration_ms_t(RETRY_TIME),FAILURE_THRESHOLD);
    cbreaker.setPool(t1.pool);
    int ret = 0;
    int a = 10;
    int b = 12;
    ret = a + b;
    int computed = 0;
    BOOST_CHECK(cbreaker.isClosed());
    BOOST_CHECK(!cbreaker.isHalfOpen());
    BOOST_CHECK(!cbreaker.isOpen());
    //std::future<int> res;
    try {
       computed = cbreaker.execute(a, b).get();
    } catch (...) {
        computed = -1;
    }
    cbreaker.execute(a,b);
    BOOST_CHECK(cbreaker.isClosed());
    BOOST_CHECK_EQUAL(computed,ret);
    // Send bad request until the circuit breaker trip
    // after FAILURES_THRESHOLD times, it will trips
    for(int i = 0; i < FAILURE_THRESHOLD +1; i++){
        try {
           computed = cbreaker.execute(std::forward<int>(a), std::forward<int>(PROCESSING_DURATION + 1)).get();
        } catch (...) {
            computed = -1;
        }
    }
    // Now the circuit breaker must have triped
    BOOST_CHECK(cbreaker.isOpen());
    // wait a RETRY_TIMEOUT time. After this time , the circuit breaker
    // should change it state to half-open
    std::this_thread::sleep_for(duration_ms_t(RETRY_TIME));
    /* send a dummy request. it should but it would
     * the circuit breaker to check for the time and will transition
     * to the half open state
     */
    try {
       computed = cbreaker.execute(std::forward<int>(a), std::forward<int>(b)).get();
    } catch (...) {
        computed = -1;
    }

    BOOST_CHECK(!cbreaker.isOpen());
    /*
     * ensure that the circuit breaker is now in half open state
     */
    BOOST_CHECK(cbreaker.isHalfOpen());

    /* Now Send a request we now the service will, will put the circuit breaker
     * back in the open state.
     * Send a right request will make the service success,
     * putting the circuit breaker in the closed state
     */
    try {
       computed = cbreaker.execute(a, b).get();
    } catch (...) {
        computed = -1;
    }
     BOOST_CHECK(cbreaker.isClosed());
}

BOOST_AUTO_TEST_CASE(CIRCUITBREAKER_MONITORING){
    TestObject t1;
    Command s(job);
    CircuitBreaker cbreaker(s,duration_ms_t(DEADLINE), duration_ms_t(RETRY_TIME),FAILURE_THRESHOLD);
    cbreaker.setPool(t1.pool);
    int submitted = 0;
    int awaited_success = 0;
    int succees = 0;
    int awaited_submitted = 0;
    int failures = 0;
    int awaited_failures = 0;
    cbreaker.execute(10, 10);
    ++awaited_submitted;
    awaited_success = 1;
    submitted = cbreaker.getUsage();
    BOOST_CHECK_EQUAL(submitted, awaited_submitted);
    succees = cbreaker.getSuccesses();
    BOOST_CHECK_EQUAL(succees, awaited_success);
    cbreaker.execute(10, 80);
    ++awaited_submitted;
    submitted = cbreaker.getUsage();
    failures = cbreaker.getFailures();
    succees = cbreaker.getSuccesses();
    awaited_failures = 1;
    BOOST_CHECK_EQUAL(submitted, awaited_submitted);
    BOOST_CHECK_EQUAL(succees,awaited_success );
    BOOST_CHECK_EQUAL(failures,awaited_failures);

}

