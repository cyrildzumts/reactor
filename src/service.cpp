#include "service.h"
#include <time.h>
#include <iostream>


ServiceError::ServiceError():std::runtime_error("BAD REQUEST"){
    what_string = std::string("BAD REQUEST");
}

ServiceError::ServiceError(const char* what_args):std::runtime_error(what_args){
    what_string = std::string(what_args);
}

ServiceError::ServiceError(const std::string& what_args):std::runtime_error(what_args){
    what_string = std::string(what_args);
}

const char* ServiceError::what()const noexcept{
    return what_string.c_str();
}



ConcreteService::ConcreteService(): service_resource_usage{0}, samples(100)
{
    std::vector<int> sources_sample(200);
    //std::fill_n(samples.begin(), 25, 0);
    std::random_device seeder;
    std::cout << " Seeder Entropy : " << seeder.entropy() << '\n';
    const auto seed = seeder.entropy() ? seeder() : time(nullptr);
    std::cout << " Seed : " << seed << '\n';
    std::default_random_engine gen (static_cast<std::default_random_engine::result_type>(seed));
    std::iota(sources_sample.begin(), sources_sample.end(),1);
    sample(sources_sample.cbegin(), sources_sample.cend(), samples.begin(),
                100, gen);


    std::cout << " Source Sample : ";
    std::copy(sources_sample.cbegin(), sources_sample.cend(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << '\n';

    std::cout << " Samples : ";
    std::copy(samples.cbegin(), samples.cend(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << '\n';

}

int ConcreteService::process_request(int request)
{
    service_resource_usage++;
    int value = 0;
    if(!((request  >= 0) && (request < samples.size()))){
        throw ServiceError("BAD REQUEST : Index out of Bound");
    }

    value = samples.at(request);

    if(!((value % 2) == 0)){
        throw ServiceError("SYSTEM ERROR: service return value : " + std::to_string(value));
    }
    std::cout << __PRETTY_FUNCTION__ << " usage : " << service_resource_usage << std::endl;
    return value;
}


