 //#define BOOST_TEST_DYN_LINK
 #define BOOST_TEST_MODULE "BaseClassModule"

#include <boost/test/unit_test.hpp>
#include <memory>
#include "circuitbreaker.h"

//class TestObject{
//private:


//    CircuitBreaker<decltype (job), int, int> cb /*{duration_ms_t(100),duration_ms_t(RETRY_TIMEOUT), FAILURES_THRESHOLD}*/;

//    int count;

//public:
//    TestObject():cb{duration_ms_t(static_cast<int>(lround((PROCESSING_DURATION * 0.25)))),duration_ms_t(RETRY_TIMEOUT), FAILURES_THRESHOLD}, count{0}{


//    }

//    ~TestObject(){

//    }

//    bool isopen() const{
//        int ret = 0;
//        return cb.isOpen();
//    }
//    bool isclosed(){
//        return cb.isClosed();
//    }
//    bool ishalfopen() const{
//        return cb.isHalfOpen();
//    }

//    int cb_compute_ok(int a, int b){
//        int ret = 0;
//        try{
//            auto res = cb.execute(std::forward<int>(a), std::forward<int>(b));
//            ret = res.get();
//        }
//        catch(...){
//            ret = -1;
//        }
//        return ret;
//    }
//};

//BOOST_AUTO_TEST_CASE(countTest){
//    TestObject t1;
//    int ret = 0;
//    int a = 10;
//    int b = 12;
//    ret = a + b;
//    int computed = 0;
//    BOOST_CHECK(t1.isclosed());
//    BOOST_CHECK(!t1.ishalfopen());
//    BOOST_CHECK(!t1.isopen());
//    computed = t1.cb_compute_ok(a,b);
//    BOOST_CHECK_EQUAL(computed,ret);
//    // Send bad request until the circuit breaker trip
//    // after FAILURES_THRESHOLD times, it will trips
//    for(int i = 0; i < FAILURES_THRESHOLD +1; i++){
//        computed = t1.cb_compute_ok(a, PROCESSING_DURATION + 1);
//    }
//    // Now the circuit breaker must have triped
//    BOOST_CHECK(t1.isopen());
//    // wait a RETRY_TIMEOUT time. After this time , the circuit breaker
//    // should change it state to half-open
//    std::this_thread::sleep_for(duration_ms_t(RETRY_TIMEOUT));
//    /* send a dummy request. it should but it would
//     * the circuit breaker to check for the time and will transition
//     * to the half open state
//     */
//    ret = t1.cb_compute_ok(a, b);

//    BOOST_CHECK(!t1.isopen());
//    /*
//     * ensure that the circuit breaker is now in half open state
//     */
//    BOOST_CHECK(t1.ishalfopen());

//    /* Now Send a request we now the service will, will put the circuit breaker
//     * back in the open state.
//     * Send a right request will make the service success,
//     * putting the circuit breaker in the closed state
//     */
//     computed = t1.cb_compute_ok(a, b);
//     BOOST_CHECK(t1.isclosed());

//     /* here because the callegation and intern operating system activities, it is difficult to
//      * predict the outcome the next call 80 as second parameters, since this will put the service
//      * into sleep for 80us and our circuit breaker deadline is set to 100us. It is very close.
//      */
//      b = 13;
//     computed = t1.cb_compute_ok(a, b);
//     BOOST_CHECK_EQUAL(computed, -1);


//}

