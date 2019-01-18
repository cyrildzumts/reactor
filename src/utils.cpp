#include "utils.h"


TestRunner::TestRunner()
{
    LOG("TestRunner constructed ...");
    int request = 0;
    double avg = 0.0;
#ifdef MTHREADING

        pool = std::make_shared<ThreadPool>();
#else
    active = std::make_shared<concurrency::Active>();
#endif

    for(size_t i = 0; i < requests.size(); i++){
        request = requests[i];
        delays_list.push_back(std::vector<int>());
        for(int j = 0; j < request; j++){
            delays_list[i].push_back(Generator<PROCESSING_DURATION>::instance()->generate());
        }
        avg = std::accumulate(delays_list[i].begin(), delays_list[i].end(), 0) / static_cast<double>(request);
        avarage.push_back(avg);
    }

    for(size_t i = 0; i < percents.size(); i++){
        deadline_list.push_back(std::vector<int>());
        std::for_each(avarage.begin(), avarage.end(), [&](double avg){
            //deadline_list[i].push_back(static_cast<int>(lround((avg * percents[i]))));
            deadline_list[i].push_back(static_cast<int>(lround((PROCESSING_DURATION * percents[i]))));
        });
    }

}
TestRunner::~TestRunner(){
    LOG("TestRunner destructed ...");
}

void TestRunner::print_avg()
{
    LOG("AVARAGE LIST : ");
    std::cout << "[";
    std::for_each(avarage.begin(), avarage.end(), [](double avg){
        std::cout << avg << " ";
    });
    std::cout << "]\n";
}

void TestRunner::print_delays()
{
    LOG("DELAYS for every request :");
    for(size_t i = 0; i < delays_list.size(); i++){
        print_delays(i);
    }
}

void TestRunner::print_delays(size_t i)
{
    LOG("DELAYS LIST  for request = : ", requests[i]);
    std::cout << "[ ";
    std::for_each(delays_list[i].begin(), delays_list[i].end(), [](int delay){
        std::cout << delay << " ";
    });
    std::cout << "]\n";
}

void TestRunner::print_deadline()
{
    LOG("DEADLINES for every percents :");
    for(size_t i = 0; i < deadline_list.size(); i++){
        print_deadline(i);
    }
}

void TestRunner::print_deadline(size_t i)
{
    LOG("DEADLINE LIST  for percent = : ", percents[i]);
    std::cout << "[ ";
    std::for_each(deadline_list[i].begin(), deadline_list[i].end(), [](int deadline){
        std::cout << deadline << " ";
    });
    std::cout << "]\n";
}

void TestRunner::run_test(int percent_index)
{
    data_t data;
    data.deadline_list_index = percent_index;
    long duration = 0;


    for(size_t i = 0; i < requests.size(); i++){
        data.request_index = i;
        duration = run_cbreaker_test(data);
        errors_list.at(percent_index).push_back(data.errors);
        success_list.at(percent_index).push_back(data.success);
        durations_list.at(percent_index).push_back(data.duration);
        ratio_success_list.at(percent_index).push_back(data.ratio_success);
        ratio_trip_list.at(percent_index).push_back(data.ratio_trip);
        //std::this_thread::sleep_for(duration_ms_t(1));
    }



    for(size_t i = 0; i < requests.size(); i++){
        data.request_index = i;
        duration = run_service_test(data);
        //LOG("ervice Test results : ", "duration : ", data.duration, "; errors : ", data.errors, "; success : ", data.success,
        //    " ; deadline : ", data.deadline, "; request : ", data.request, "; percent : ", data.percent);
        //LOG("Durarion : ", duration);
        service_errors_list.at(percent_index).push_back(data.errors);
        service_success_list.at(percent_index).push_back(data.success);
        service_durations_list.at(percent_index).push_back(data.duration);
        service_ratio_success_list.at(percent_index).push_back(data.ratio_success);
        service_ratio_trip_list.at(percent_index).push_back(data.ratio_trip);
    }

    //LOG("TEST with percent ", percents.at(percent_index), " ratio success : ", data.ratio_success, " ratio trip : ", data.ratio_trip);
}

void TestRunner::run_test()
{
    LOG("Running Test started ...");
    for(size_t i = 0; i < percents.size(); i++){
        errors_list.push_back(std::vector<int>());
        success_list.push_back(std::vector<int>());
        durations_list.push_back(std::vector<int>());
        ratio_success_list.push_back(std::vector<double>());
        ratio_trip_list.push_back(std::vector<double>());

        service_errors_list.push_back(std::vector<int>());
        service_success_list.push_back(std::vector<int>());
        service_durations_list.push_back(std::vector<int>());
        service_ratio_success_list.push_back(std::vector<double>());
        service_ratio_trip_list.push_back(std::vector<double>());

        run_test(i);
        LOG("Running Test finished");
    }


}

long TestRunner::run_service_test(data_t &data)
{
    LOG("run Service test started ...");
    int request = requests[data.request_index];
    int deadline = deadline_list[data.deadline_list_index][data.request_index];
    int errors = 0;
    int success = 0;
    long duration = 0;
    int ret = 0;
    std::future<CURLcode> code ;
    CURLcode res_code;
    std::unique_ptr<Service> service = std::make_unique<ConcreteService>();
    auto start = std::chrono::system_clock::now();
    for(size_t i = 0; i < request; i++){
        try {
            #ifdef MTHREADING
                code = pool->submit(http_job,URL_2);
            #else
                code = active->submit(http_job,URL_2); // set the Active Object which provides a thread execution unit
            #endif


             res_code = code.get();
            //ret = service->process_request(request, delays_list[data.request_index][i] );
            ++success;

        } catch (...) {
            errors++;
        }
    }
    auto end = std::chrono::system_clock::now() - start;
    duration = std::chrono::duration_cast<duration_ms_t>(end).count();
    data.errors = errors;
    data.success = success;
    data.duration = duration;
    data.deadline = deadline;
    data.request = request;
    data.ratio_success = static_cast<double>(success)/request;
    data.ratio_trip = 0.0;
    data.percent = percents[data.deadline_list_index];
    //LOG("run Service test finished ... : duration : ", data.duration, " duration 2 : ", duration);
    return duration;
}

long TestRunner::run_cbreaker_test(data_t &data)
{
    //LOG("run cbreaker test started ...");
    int request = requests[data.request_index];
    int deadline = deadline_list[data.deadline_list_index][data.request_index];
    int errors = 0;
    int success = 0;
    long duration = 0;
    int failures_threshold = 0;
    int retry_time = 0;
    CURLcode code;
#ifdef FAILURES_THRESHOLD
    failures_threshold = FAILURES_THRESHOLD;
#else
    failures_threshold = 3;
#endif

#ifdef RETRY_TIMEOUT
    retry_time = RETRY_TIMEOUT;
#else
    retry_time = 100;
#endif
    CircuitBreaker cb(duration_ms_t(deadline), duration_ms_t(retry_time),failures_threshold);
#ifdef MTHREADING
    cb.setPool(pool);
#else
    cb.setActive(active); // set the Active Object which provides a thread execution unit
#endif
    auto start = std::chrono::system_clock::now();
    for(size_t i = 0; i < request; i++){
        try {
#ifdef USE_REMOTE_SERVICE
          code = cb.fetch_sumbit(URL_2);
#else
          cb.process_request(request, delays_list.at(data.request_index).at(i) );
#endif
            success++;
        } catch (...) {
            errors++;
        }
    }
    auto end = std::chrono::system_clock::now() - start;
    duration = std::chrono::duration_cast<duration_ms_t>(end).count();
    data.errors = errors;
    data.success = success;
    data.duration = duration;
    data.deadline = deadline;
    data.request = request;
    data.trips = cb.getFailure_threshold_reached();
    data.ratio_success = cb.getRatio();
    data.ratio_trip = cb.getRatio_trip();
    data.percent = percents[data.deadline_list_index];
    //LOG("TEST with percent ", data.percent, " ratio success : ", data.ratio_success, " ratio trip : ", data.ratio_trip, " trips : ", data.trips);
    return duration;
}



void TestRunner::save_result()
{


#ifdef LOG_FILENAME
    std::string filename = std::string(LOG_FILENAME);
#else
    std::string filename = "log/circuit_breaker.txt";
#endif
    std::ios_base::openmode mode = std::fstream::app|std::fstream::in|std::fstream::out;
    std::fstream file(filename, mode);
    if(!file.is_open()){
        LOG("TestRunner : ", " unable to open ", filename);
        exit(EXIT_FAILURE);
    }
    LOG("Saving test results into ", filename);
    file << "System Infos : \n"
         << "Operating System \t: Ubuntu 18.04.1 LTS\n"
         << "RAM              \t: 12GB\n"
         << "PROCESSOR        \t: Intel Core i7-8550U @1.80Hz * 8\n"
         << "GRAPHICS 1       \t: Intel UHD 620\n"
         << "GRAPHICS 2       \t: NVidia GEFORCE 150\n"
         << "ARCHITECTURE     \t: 64-bit\n"
         << "DESKTOP          \t: GNOME 3.28.2\n"
         << "DEVICE           \t: HP ENVY x360 15-bp150\n";
    for(size_t i = 0; i < percents.size(); i++){
        file << "TestRunner Circuit Breaker run for PERCENTAGE :  " << percents[i] * 100  << "% \n";
        file << "--------------------------------------------------------------\n";
        file << "REQUEST \t Durations\t Errors\t Success\t Success Ratio(%)\t Trip Ratio(%)\n";
        file << "--------------------------------------------------------------\n";
        for(size_t j = 0; j < requests.size(); j++){
            file << requests[j] <<"\t\t"<< durations_list[i][j] << "\t\t"
                 << errors_list[i][j] << "\t\t" << success_list[i][j]<<"\t\t"<<ratio_success_list[i][j]<<"\t\t" <<ratio_trip_list[i][j] << "\n";
        }
        file << "--------------------------------------------------------------\n";
        file << "Summary : \n";
        file << "request   \t= [ ";
        std::for_each(requests.begin(), requests.end(), [&](int element){
            file << element << " ";
        });
        file << " ]\n";
        file << "durations \t= [ ";

        std::for_each(durations_list[i].begin(), durations_list[i].end(), [&](long element){
            file << element << " ";
        });
        file << " ]\n";
        file << "errors    \t= [ ";
        std::for_each(errors_list[i].begin(), errors_list[i].end(), [&](int element){
            file << element << " ";
        });
        file << " ]\n";
        file << "success   \t= [ ";
        std::for_each(success_list[i].begin(), success_list[i].end(), [&](int element){
            file << element << " ";
        });
        file << " ]\n";
        file << "ratio_success   \t= [ ";
        std::for_each(ratio_success_list[i].begin(), ratio_success_list[i].end(), [&](int element){
            file << element << " ";
        });
        file << " ]\n";

        file << "ratio_trip   \t= [ ";
        std::for_each(ratio_trip_list[i].begin(), ratio_trip_list[i].end(), [&](int element){
            file << element << " ";
        });
        file << " ]\n";

    }
    file << "--------------------------------------------------------------\n";
    file.close();
    LOG("Saving result finished ...");
}

void TestRunner::save__service_result()
{
#ifdef LOG_FILENAME
    std::string filename = std::string(LOG_FILENAME_SERVICE);
#else
    std::string filename = "log/service.txt";
#endif
    std::ios_base::openmode mode = std::fstream::app|std::fstream::in|std::fstream::out;
    std::fstream file(filename, mode);
    if(!file.is_open()){
        LOG("TestRunner : ", " unable to open ", filename);
        exit(EXIT_FAILURE);
    }
    LOG("Saving Service test results into ", filename);
    file << "System Infos : \n"
         << "Operating System \t: Ubuntu 18.04.1 LTS\n"
         << "RAM              \t: 12GB\n"
         << "PROCESSOR        \t: Intel Core i7-8550U @1.80Hz * 8\n"
         << "GRAPHICS 1       \t: Intel UHD 620\n"
         << "GRAPHICS 2       \t: NVidia GEFORCE 150\n"
         << "ARCHITECTURE     \t: 64-bit\n"
         << "DESKTOP          \t: GNOME 3.28.2\n"
         << "DEVICE           \t: HP ENVY x360 15-bp150\n";
    for(size_t i = 0; i < percents.size(); i++){
        file << "TestRunner Service run for PERCENTAGE :  " << percents[i] * 100  << "% \n";
        file << "--------------------------------------------------------------\n";
        file << "REQUEST \t Durations\t Errors\t Success\t Success Ratio(%)\t Trip Ratio(%)\n";
        file << "--------------------------------------------------------------\n";
        for(size_t j = 0; j < requests.size(); j++){
            file << requests[j] <<"\t\t"<< service_durations_list[i][j] << "\t\t"
                 << service_errors_list[i][j] << "\t\t" << service_success_list[i][j]<<"\t\t"<<service_ratio_success_list[i][j]<<"\t\t" <<service_ratio_trip_list[i][j] << "\n";
        }
        file << "--------------------------------------------------------------\n";
        file << "Summary : \n";
        file << "request   \t= [ ";
        std::for_each(requests.begin(), requests.end(), [&](int element){
            file << element << " ";
        });
        file << " ]\n";
        file << "durations \t= [ ";

        std::for_each(service_durations_list[i].begin(), service_durations_list[i].end(), [&](long element){
            file << element << " ";
        });
        file << " ]\n";
        file << "errors    \t= [ ";
        std::for_each(service_errors_list[i].begin(), service_errors_list[i].end(), [&](int element){
            file << element << " ";
        });
        file << " ]\n";
        file << "success   \t= [ ";
        std::for_each(service_success_list[i].begin(), service_success_list[i].end(), [&](int element){
            file << element << " ";
        });
        file << " ]\n";

        file << "ratio_success   \t= [ ";
        std::for_each(service_ratio_success_list[i].begin(), service_ratio_success_list[i].end(), [&](int element){
            file << element << " ";
        });
        file << " ]\n";

        file << "ratio_trip   \t= [ ";
        std::for_each(service_ratio_trip_list[i].begin(), service_ratio_trip_list[i].end(), [&](int element){
            file << element << " ";
        });
        file << " ]\n";

    }
    file << "--------------------------------------------------------------\n";
    file.close();
    LOG("Saving result finished ...");
}
