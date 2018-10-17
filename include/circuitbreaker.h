#ifndef CIRCUITBREAKER_H
#define CIRCUITBREAKER_H
#include "service.h"

class CircuitBreaker : public Service
{
public:
    enum CBSTATE{
        OPEN        = 1,
        CLOSED      = 2,
        HALF_CLOSED = 3
    };
private:
    CBSTATE state;
    Service *service;
public:
    CircuitBreaker(Service *service);

    // Service interface
public:
    virtual int service_1(int in) override;
    virtual int service_2(int in) override;
    virtual int service_3(int in) override;
};

#endif // CIRCUITBREAKER_H
