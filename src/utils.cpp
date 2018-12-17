#include "utils.h"


TestRunner::TestRunner()
{
    LOG("TestRunner constructed ...");
}
TestRunner::~TestRunner(){
    LOG("TestRunner destructed ...");
}

void TestRunner::run_test()
{

    LOG("Running Test started ...");
    std::for_each(percents.begin(), percents.end(), [&](double percent){
        LOG("RUNNING TEST with percent : ", percent);
        std::for_each(requests.begin(), requests.end(), [&](int request){
            for(size_t i = 0; i < request; i++){
                delays.push_back(Generator<PROCESSING_DURATION>::instance()->generate());
            }
            int result = std::accumulate( delays.begin(), delays.end(), 0);

            double average_value = result / static_cast<double>(request);
            average_time = static_cast<int>(average_value);
            LOG("RUNNING TEST with percent : ", percent, " REQUEST : ", request);
            this->run_service_test(request, percent);
            this->run_cbreaker_test(request, percent);
            waiting_times.push_back(average_time);
            delays.clear();
        });
        LOG("RUNNING TEST with percent : ", percent, " finished.");
        LOG("Saving TEST with percent : ", percent, " ...");
        this->save_result(percent);
        LOG("Saving TEST with percent : ", percent, " done !");
    });
    LOG("Running Test finished");
}

void TestRunner::run_service_test(int request, double percent)
{
    int ret;
    int success_count = 0;
    int error_count = 0;
    long duration = 0;
    std::optional<int> wait_time(average_time);
    std::unique_ptr<Service> service{new ConcreteService(wait_time)};
    auto start = std::chrono::system_clock::now();

    for(size_t i = 0; i < request; i++){
        try {
            ret = service->process_request(i, delays[i]);
            if(ret < average_time) {
                success_count++;
            }
            else {
                error_count++;
            }
            //LOG("working ...");
        }
        catch(ServiceError &e){
            error_count++;
            //LOG("MAIN -- Service Error :  ", e.what());
        }
    }
    auto end = std::chrono::system_clock::now() - start;
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end).count();
    results_services.push_back(duration);
    errors_services.push_back(error_count);
    success_services.push_back(success_count);
}

void TestRunner::run_cbreaker_test(int request, double percent)
{
    int ret;
    int success_count = 0;
    int error_count = 0;
    long duration = 0;

    int waiting_time = static_cast<int>(average_time * percent);
    std::optional<int> wait_time(average_time);
    std::optional<int> cb_wait_time(waiting_time);
    std::shared_ptr<Service> service{new ConcreteService(wait_time)};
    CircuitBreaker cb(service, cb_wait_time);
    auto start = std::chrono::system_clock::now();

    for(size_t i = 0; i < request; i++){
        try {
            ret = cb.process_request(i, delays[i]);
            success_count++;
            //LOG(" Circuit Breaker working ...");
        }
        catch(ServiceError &e){
            error_count++;
            //LOG("MAIN -- Service Error :  ", e.what());
        }
        catch(TimeoutError &e){
            error_count++;
            //LOG("MAIN -- Timeout ");
        }
    }
    auto end = std::chrono::system_clock::now() - start;
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end).count();
    results_circuitbreaker.push_back(duration);
    errors_cbreaker.push_back(error_count);
    success_cbreaker.push_back(success_count);
}

void TestRunner::save_result(double percent)
{
    std::string f = "log/result_" + std::to_string(percent) + ".m";
    std::string description;
    std::stringstream str_stream;
    size_t i = 0;
    int request = 0;
    std::ios_base::openmode mode = std::fstream::app|std::fstream::in|std::fstream::out;
    std::fstream file(f, mode);
    if(!file.is_open()){
        LOG("TestRunner : ", " unable to open ", f);
        exit(EXIT_FAILURE);
    }

    file << "TestRunner run for PERCENTAGE :  " << percent * 100  << "% \n";
    file << "--------------------------------------------------------------\n";

    for(i = 0; i < results_services.size(); i++){
        request = requests[i];
        file << "TestRunner run with REQUEST = " << request << " : \n";
        file << "Average waiting time : " << waiting_times[i] << " ms \n";
        file << "Error count when calling the Service directly : " << errors_services[i] << " errors \n";
        file << "Error count when calling the CBreaker directly : " << errors_cbreaker[i] << " errors \n";
        file << "Success count when calling the Service directly : " << success_services[i] << " success \n";
        file << "Success count when calling the CBreaker directly : " << success_cbreaker[i] << " success \n";
        file << "###############################################################################\n";

    }
    file << "Client waiting time when using the Service directly : \n";
    file << "durations_services = [ ";
    std::for_each(results_services.begin(), results_services.end(), [&](long value){
        file << value << " ";
    });
    file << "];\n";
    file << "Client waiting time when using the Circuit Breaker : \n";
    file << "durations_cbreaker = [ ";
    std::for_each(results_circuitbreaker.begin(), results_circuitbreaker.end(), [&](long value){
        file << value << " ";
    });
    file << "];\n";
    file << "--------------------------------------------------------------\n";
    file.close();
    results_circuitbreaker.clear();
    results_services.clear();
    errors_cbreaker.clear();
    errors_services.clear();
    waiting_times.clear();
    delays.clear();

    results_circuitbreaker.resize(0);
    results_services.resize(0);
    errors_cbreaker.resize(0);
    errors_services.resize(0);
    waiting_times.resize(0);
    delays.resize(0);
    /*
    results_circuitbreaker = std::vector<long>();
    results_services = std::vector<long>();
    errors_cbreaker = std::vector<int>();
    errors_services = std::vector<int>();
    waiting_times = std::vector<int>();
    */

}




