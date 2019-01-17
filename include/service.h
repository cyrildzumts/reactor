﻿
#ifndef SERVICE_H
#define SERVICE_H

#include "common.h"
#include "generator.h"
#include <log.h>
#include <optional>
#include <mutex>
#include <exception>
#include <random>
#include <iterator>
#include <algorithm>
#include <vector>



int job(int req, int delay);

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
    virtual int process_request(int request,int delay = PROCESSING_DURATION) = 0;
    virtual int operator()(int request, int delay = PROCESSING_DURATION) = 0;
};


class ConcreteService : public Service{
private:
    int service_resource_usage;
public:
    ConcreteService();
    // Service interface
public:
    /**
     * @brief process_request return the number from samples at the location indicated by request.
     * if that number is even, the service returns normally. If the number is odd, the
     * service throws a ServiceError Exception.
     * @param request : request >= 0 && request < samples.size()
     * @return the number located at the position indicated by request in samples.
     *   throws a ServiceError if request is invalide.
     *
     */
    virtual int process_request(int request, int delay = PROCESSING_DURATION);

    // Service interface
public:
    virtual int operator ()(int request, int delay = PROCESSING_DURATION) override;
};
#endif // SERVICE_H
