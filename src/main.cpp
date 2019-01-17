#define DEBUG_ON 1
#include "utils.h"
#include "cbreaker.h"
#include <log.h> // < -- Logger library
#include <iostream>
#include <memory> // <-- Smart pointers

using namespace std;
using func_type = decltype (job);
using task_result_t = breaker::AbstractBreaker<decltype (job),int, int>::result_type;
int main(int argc, char const *argv[])
{
    LOG("Reactor: Circuit Breaker ");
    LOG("TESTING CBREAKER IMPLEMENTATION");
    std::shared_ptr<concurrency::Active>active = std::make_shared<concurrency::Active>();
    breaker::AbstractBreaker<decltype (job),int, int> cb(duration_ms_t(10), duration_ms_t(100), 5 );
    cb.setActive(active);
    std::future<task_result_t> rest;
    try {
        rest = cb.execute(10, 40);
    } catch (...) {
        LOG("EXCept thrown");
    }
//    auto res = cb.execute(10, 40);
//    int ret = -1;
//    try {
//        ret = res.get();
//        LOG("RESULT : ", ret);
//    } catch (std::exception &e) {

//        LOG("RESULT ERROR: ", e.what());
//    }
/*
    TestRunner runner;
    runner.run_test();
    runner.save_result();
    runner.save__service_result();
    */

    return 0;
}
