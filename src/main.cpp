

//#define LOG_LEVEL_1

#include "test_saver.h"
#include "activeobject.h"
#include "actuators.h"
#include <string>
#include <sstream>
#include <iostream>
#include <memory>
#include "circuitbreaker.h"
#include "client.h"
#include "threadpool.h"
#include <thread>
#include <log.h>
using namespace std;

int constexpr MAX_REQUEST = 500;


int print_task(int a){
    LOG ("TASK Run by ", std::this_thread::get_id());
    return a * 10;
}

int main(int argc, char const *argv[])
{
    std::cout << "Reactor: Circuit Breaker " << std::endl;
    std::string description;
    long duration_with_cb =0;
    long duration_without_cb =0;
    std::stringstream str_stream;
    std::vector<int> delays(MAX_REQUEST);
    for(size_t i = 0; i < MAX_REQUEST; i++){
        delays[i] = Generator<PROCESSING_DURATION>::instance()->generate();
    }
    LOG("Reactor Main entry point ", "thread id : ", std::this_thread::get_id());
    std::shared_ptr<Service> service{new ConcreteService()};
    CircuitBreaker cb{service};
    auto start_without_circuit_breaker = std::chrono::system_clock::now();
    for(size_t i = 0; i < MAX_REQUEST; i++){
        try {
            service->process_request(i, delays[i]);
            //LOG("working ...");
        }
        catch(ServiceError &e){
            //LOG("MAIN -- Service Error :  ", e.what());
        }
    }
    auto end_without_circuit_breaker = std::chrono::system_clock::now() - start_without_circuit_breaker;

    auto start_with_circuit_breaker = std::chrono::system_clock::now();
    for(int i = 0; i < MAX_REQUEST; i++){
        try {
            cb.process_request(i, delays[i]);
            //std::this_thread::sleep_for(duration_ms_t(DELAY_500_MS));
        }
        catch(TimeoutError &e){
            //LOG("MAIN -- Timeout ");
        }
        catch(ServiceError &e){
            //LOG("MAIN -- Service Error :  ", e.what());
        }
        catch(std::future_error &e){
            //LOG("MAIN Future Error : ", e.what());
        }
        
    }
    auto end_with_circuit_breaker = std::chrono::system_clock::now() - start_with_circuit_breaker;
    duration_without_cb = std::chrono::duration_cast<std::chrono::milliseconds>(end_without_circuit_breaker).count();
    duration_with_cb = std::chrono::duration_cast<std::chrono::milliseconds>(end_with_circuit_breaker).count();
    LOG("Service called without Circuit Breaker ", "-- Duration : ", duration_without_cb, "ms");
    LOG("Service called with Circuit Breaker ", "-- Duration : ",duration_with_cb , "ms");
    str_stream << "Test value with " << MAX_REQUEST << " " << "requests\n";
    str_stream << "Test run without Circuit Breaker took " << duration_without_cb << " ms\n";
    str_stream << "Test run with Circuit Breaker took " << duration_with_cb << " ms\n";
    //std::getline(str_stream, description);
    description = str_stream.str();
    Testsaver::instance()->save(delays,description);
    return 0;
}
