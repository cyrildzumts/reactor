#ifndef UTILS_H
#define UTILS_H

#include "common.h"
#include "circuitbreaker.h"
#include "generator.h"
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <optional>
#include <numeric>
#include <cmath>
#include <algorithm>


using namespace std::chrono;
using unit_ms = std::chrono::milliseconds;
using unit_us = std::chrono::microseconds;


#ifdef TIME_UNIT_MS
    using duration_t = duration<unit_ms>;
    using unit_t = unit_ms;
#else
    using duration_t = duration<unit_us>;
    using unit_t = unit_us;
#endif

#ifndef URL
    #define URL URL_2
#endif


struct result_t{
    int request;
    int errors;
    int success;
    int deadline;
    long duration;
    double percent;
};

struct data_t{
    size_t request_index;
    size_t deadline_list_index;
    int request;
    int errors;
    int success;
    int deadline;
    long duration;
    long trips ;
    double percent;
    double ratio_success;
    double ratio_trip;
};




class TestRunner{
private:
    std::shared_ptr<ThreadPool>pool;
    bool is_direct_service_run;
    std::vector<std::vector<int>> errors_list;
    std::vector<std::vector<int>> success_list;
    std::vector<std::vector<int>> durations_list;
    std::vector<std::vector<double>> ratio_success_list;
    std::vector<std::vector<double>> ratio_trip_list;

    std::vector<std::vector<int>> service_errors_list;
    std::vector<std::vector<int>> service_success_list;
    std::vector<std::vector<int>> service_durations_list;
    std::vector<std::vector<double>> service_ratio_success_list;
    std::vector<std::vector<double>> service_ratio_trip_list;


    std::vector<std::vector<int>> delays_list;
    std::vector<std::vector<int>> deadline_list;
    std::vector<double> avarage;
    std::vector<int> requests{1, 10, 50, 100, 10000};
    //std::vector<int> requests{1, 10, 50, 100, 500, 1000, 5000, 10000, 100000, 1000000};
    std::vector<double> percents{0.5, 0.75, 1, 1.25, 2};
public:
    TestRunner();
    ~TestRunner();
    void print_avg();
    void print_delays();
    void print_delays(size_t i);
    void print_deadline();
    void print_deadline(size_t i);
    void run_test(int percent_index);
    void run_test();
    long run_service_test(data_t &data);
    long run_cbreaker_test(data_t &data);
    void save_result();
    void save__service_result();
};

#endif //UTILS_H
