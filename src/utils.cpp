#include "utils.h"


TestRunner::TestRunner()
{
    LOG("TestRunner constructed ...");
    int request = 0;
    double avg = 0.0;
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
            deadline_list[i].push_back(static_cast<int>(lround((avg * percents[i]))));
            //deadline_list[i].push_back(static_cast<int>(lround((PROCESSING_DURATION * percents[i]))));
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

    LOG("Running Test started ...");
    data_t data;
    data.deadline_list_index = percent_index;
    long duration = 0;


    for(size_t i = 0; i < requests.size(); i++){
        data.request_index = i;
        duration = run_cbreaker_test(data);
        LOG("Circuit Breaker Test results : ", "duration : ", data.duration, "; errors : ", data.errors, "; success : ", data.success,
            " ; deadline : ", data.deadline, "; request : ", data.request, "; percent : ", data.percent);
        LOG("Durarion : ", duration);
        errors_list.at(percent_index).push_back(data.errors);
        success_list.at(percent_index).push_back(data.success);
        durations_list.at(percent_index).push_back(data.duration);
    }



    for(size_t i = 0; i < requests.size(); i++){
        data.request_index = i;
        duration = run_service_test(data);
        LOG("ervice Test results : ", "duration : ", data.duration, "; errors : ", data.errors, "; success : ", data.success,
            " ; deadline : ", data.deadline, "; request : ", data.request, "; percent : ", data.percent);
        LOG("Durarion : ", duration);
        service_errors_list.at(percent_index).push_back(data.errors);
        service_success_list.at(percent_index).push_back(data.success);
        service_durations_list.at(percent_index).push_back(data.duration);
    }

    LOG("Running Test finished");
}

void TestRunner::run_test()
{
    for(size_t i = 0; i < percents.size(); i++){
        errors_list.push_back(std::vector<int>());
        success_list.push_back(std::vector<int>());
        durations_list.push_back(std::vector<int>());

        service_errors_list.push_back(std::vector<int>());
        service_success_list.push_back(std::vector<int>());
        service_durations_list.push_back(std::vector<int>());

        run_test(i);

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
    std::unique_ptr<Service> service = std::make_unique<ConcreteService>();
    auto start = std::chrono::system_clock::now();
    for(size_t i = 0; i < request; i++){
        try {
            ret = service->process_request(request, delays_list[data.request_index][i] );
            if(ret <= deadline){
                success++;
            }
            else{
                errors++;
            }
        } catch (...) {
            errors++;
        }
    }
    auto end = std::chrono::system_clock::now() - start;
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end).count();
    data.errors = errors;
    data.success = success;
    data.duration = duration;
    data.deadline = deadline;
    data.request = request;
    data.percent = percents[data.deadline_list_index];
    LOG("run Service test finished ... : duration : ", data.duration, " duration 2 : ", duration);
    return duration;
}

long TestRunner::run_cbreaker_test(data_t &data)
{
    LOG("run cbreaker test started ...");
    int request = requests[data.request_index];
    int deadline = deadline_list[data.deadline_list_index][data.request_index];
    int errors = 0;
    int success = 0;
    long duration = 0;
    std::string header;
    CircuitBreaker cb(duration_ms_t(deadline +5), duration_ms_t(100), 5 );
    auto start = std::chrono::system_clock::now();
    for(size_t i = 0; i < request; i++){
        try {
            cb.process_request(request, delays_list[data.request_index][i] );
            success++;
        } catch (...) {
            errors++;
        }
    }
    auto end = std::chrono::system_clock::now() - start;
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end).count();
    data.errors = errors;
    data.success = success;
    data.duration = duration;
    data.deadline = deadline;
    data.request = request;
    data.percent = percents[data.deadline_list_index];
    LOG("run cbreaker test finished ... : duration : ", data.duration, " duration 2 : ", duration);
    return duration;
}



void TestRunner::save_result()
{
    std::string filename = "log/circuit_breaker.txt";
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
        file << "REQUEST \t Durations\t Errors\t Success\t\n";
        file << "--------------------------------------------------------------\n";
        for(size_t j = 0; j < requests.size(); j++){
            file << requests[j] <<"\t\t"<< durations_list[i][j] << "\t\t"
                 << errors_list[i][j] << "\t\t" << success_list[i][j] << "\t\n";
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

    }
    file << "--------------------------------------------------------------\n";
    file.close();
    LOG("Saving result finished ...");
}

void TestRunner::save__service_result()
{
    std::string filename = "log/service.txt";
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
        file << "REQUEST \t Durations\t Errors\t Success\t\n";
        file << "--------------------------------------------------------------\n";
        for(size_t j = 0; j < requests.size(); j++){
            file << requests[j] <<"\t\t"<< service_durations_list[i][j] << "\t\t"
                 << service_errors_list[i][j] << "\t\t" << service_success_list[i][j] << "\t\n";
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

    }
    file << "--------------------------------------------------------------\n";
    file.close();
    LOG("Saving result finished ...");
}
