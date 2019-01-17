
#include "utils.h"
#include <log.h> // < -- Logger library
#include <iostream>
#include <memory> // <-- Smart pointers

using namespace std;


int main(int argc, char const *argv[])
{
    LOG("Reactor: Circuit Breaker ");
    TestRunner runner;
    runner.run_test();
    runner.save_result();
    //runner.save__service_result();

    return 0;
}
