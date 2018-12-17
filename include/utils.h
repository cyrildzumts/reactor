#ifndef UTILS_H
#define UTILS_H

#include "circuitbreaker.h"
#include "generator.h"
#include <chrono>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <numeric>
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


class TestRunner{
private:

    int average_time;
    std::vector<int> delays;
    std::vector<int> errors_services;
    std::vector<int> success_services;
    std::vector<int> errors_cbreaker;
    std::vector<int> success_cbreaker;
    std::vector<int> waiting_times;
    std::vector<int> requests{1, 5, 10, 20, 50/*, 100, 200, 500, 1000, 2000, 5000*/};
    std::vector<long> results_services;
    std::vector<long> results_circuitbreaker;
    std::vector<double> percents{0.1,0.25, 0.5/*, 0.75, 1, 1.25, 1.5, 1.75, 2*/};

public:
    TestRunner();
    ~TestRunner();
    void run_test();
    void run_service_test(int request, double percent);
    void run_cbreaker_test(int request, double percent);
    void save_result(double percent);
};

#endif //UTILS_H
