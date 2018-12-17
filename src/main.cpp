

//#define LOG_LEVEL_1

#include "test_saver.h"
#include "utils.h"
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
#include <experimental/string_view>
#include <numeric>
#include <algorithm>



using namespace std;

int constexpr MAX_REQUEST = 5000;


int print_task(int a){
    LOG ("TASK Run by ", std::this_thread::get_id());
    return a * 10;
}



int main(int argc, char const *argv[])
{
    std::cout << "Reactor: Circuit Breaker " << std::endl;
    /*
    std::vector<double>percentages {0.1, 0.25, 0.5, 0.75, 1, 1.25, 1.5, 1.75, 2};
    std::vector<int> requests {1,5,10,20,50,100,200,500,1000,2000,5000};
    std::vector<int> times_with_cb;
    std::vector<int> times_without_cb;
    int error_count = 0;
    int success_count = 0;
    int error_count_wcb = 0;
    int success_count_wcb = 0;
    int ret = 0;
    int computed_waiting_time = 0;
    long result = 0L;
    double average_value;
    long duration_with_cb =0;
    long duration_without_cb =0;
    std::string filename;
    std::string description;
    std::stringstream str_stream;
    std::vector<int> delays(MAX_REQUEST);
    for(size_t i = 0; i < MAX_REQUEST; i++){
        delays[i] = Generator<PROCESSING_DURATION>::instance()->generate();
    }
    result = std::accumulate( delays.begin(), delays.end(), 0);

    average_value = result / MAX_REQUEST;
    int av = static_cast<int>(average_value);
    computed_waiting_time = static_cast<int>(average_value * WAIT_200_PER_CENT);
    std::optional<int> circuit_breaker_wait_time (computed_waiting_time);
    std::optional<int>service_wait_time(av);
    std::shared_ptr<Service> service{new ConcreteService(service_wait_time)};
    CircuitBreaker cb(service, circuit_breaker_wait_time);
    //CircuitBreaker cb(service, std::nullopt);
    auto start_without_circuit_breaker = std::chrono::system_clock::now();
    for(size_t i = 0; i < MAX_REQUEST; i++){
        try {
            ret = service->process_request(i, delays[i]);
            if(ret < computed_waiting_time) {
                success_count++;
            }
            else {
                error_count++;
            }
            LOG("working ...");
        }
        catch(ServiceError &e){
            error_count++;
            LOG("MAIN -- Service Error :  ", e.what());
        }
    }
    auto end_without_circuit_breaker = std::chrono::system_clock::now() - start_without_circuit_breaker;

    auto start_with_circuit_breaker = std::chrono::system_clock::now();
    for(int i = 0; i < MAX_REQUEST; i++){
        try {
            cb.process_request(i, delays[i]);
            success_count_wcb++;
        }
        catch(TimeoutError &e){
            error_count_wcb++;
            LOG("MAIN -- Timeout ");
        }
        catch(ServiceError &e){
            error_count_wcb++;
            //LOG("MAIN -- Service Error :  ", e.what());
        }
        catch(std::future_error &e){
            error_count_wcb++;
            //LOG("MAIN Future Error : ", e.what());
        }
        
    }

    auto end_with_circuit_breaker = std::chrono::system_clock::now() - start_with_circuit_breaker;
    duration_without_cb = std::chrono::duration_cast<std::chrono::milliseconds>(end_without_circuit_breaker).count();
    duration_with_cb = std::chrono::duration_cast<std::chrono::milliseconds>(end_with_circuit_breaker).count();
    LOG("Service called without Circuit Breaker ", "-- Duration : ", duration_without_cb, "ms");
    LOG("Service called with Circuit Breaker ", "-- Duration : ",duration_with_cb , "ms");
    str_stream << "Test value with " << MAX_REQUEST << " " << "requests\n";
    str_stream << "Client WAIT_TIME " << computed_waiting_time << " ms\n" ;
    str_stream << "Average Processing time : " << average_value << "ms\n";
    str_stream << "Error Count  " << error_count << "\n" ;
    str_stream << "Success Count " << success_count << "\n" ;
    str_stream << "Error Count with Circuit Breaker " << error_count_wcb << "\n" ;
    str_stream << "Success Count with Circuit Breaker " << success_count_wcb << "\n" ;
    str_stream << "Service Max Processing Time " << PROCESSING_DURATION << " ms\n" ;
    str_stream << "Test run without Circuit Breaker took " << duration_without_cb << " ms\n";
    str_stream << "Test run with Circuit Breaker took " << duration_with_cb << " ms\n";
    description = str_stream.str();
    Testsaver::instance()->save(delays,description);
    */
    TestRunner runner;
    runner.run_test();
    return 0;
}
