#ifndef UTILS_H
#define UTILS_H
#include "circuitbreaker.h"
#include "cbreaker.h"
#include "generator.h"
#include <chrono>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <optional>
#include <numeric>
#include <cmath>
#include <algorithm>

#define WAIT_10_PER_CENT 0.1
#define WAIT_25_PER_CENT 0.25
#define WAIT_50_PER_CENT 0.5
#define WAIT_75_PER_CENT 0.75
#define WAIT_100_PER_CENT 1

#define WAIT_125_PER_CENT 1.25
#define WAIT_150_PER_CENT 1.5
#define WAIT_175_PER_CENT 1.75
#define WAIT_200_PER_CENT 2

constexpr int PERCENT_COUNT = 2;
constexpr int REQUEST_COUNT = 3;

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
    double percent;
};




class TestRunner{
private:
    std::shared_ptr<concurrency::Active> active;
    ConcreteService service;
    std::vector<std::vector<int>> errors_list;
    std::vector<std::vector<int>> success_list;
    std::vector<std::vector<int>> durations_list;

    std::vector<std::vector<int>> service_errors_list;
    std::vector<std::vector<int>> service_success_list;
    std::vector<std::vector<int>> service_durations_list;


    std::vector<std::vector<int>> delays_list;
    std::vector<std::vector<int>> deadline_list;
    std::vector<double> avarage;
    std::vector<int> requests{1, 5, 10, 20, 50, 100, 200, 500, 1000/*, 2000, 5000, 10000, 100000, 1000000*/};
    std::vector<double> percents{0.1,0.25, 0.5, 0.75, 1, 1.25, 1.5/*, 1.75, 2*/};
    //static constexpr std::array<double, PERCENT_COUNT> percents{0.1, 0.25};
    //static constexpr std::array<int, REQUEST_COUNT> requests{1, 5, 10};
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
