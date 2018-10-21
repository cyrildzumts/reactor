#ifndef SERVICE_H
#define SERVICE_H
#include <mutex>
#include <exception>
#include <random>
#include <iterator>
#include <algorithm>
#include <vector>



class ServiceError : public std::runtime_error{
public:
    enum Error {
        BAD_REQUEST = 1, UNAUTHORISED, SYSTEM, TOO_FAST_REQUEST
    };
    ServiceError(const char* what_args);
    ServiceError(const std::string& what_args);
    ServiceError();
    // Exception Interface
    virtual const char* what() const noexcept;

private:
    std::string what_string;

};

class Service
{
public:
    virtual ~Service(){}

    virtual int process_request(int request) = 0;
};


class ConcreteService : public Service{
private:
    int service_resource_usage;

    std::vector<int> samples;

    //std::mutex service_mux;

public:
    ConcreteService();
    // Service interface
public:
    virtual int process_request(int request);
};
#endif // SERVICE_H
