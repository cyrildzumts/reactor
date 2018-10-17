#include "circuitbreaker.h"
#include <iostream>
CircuitBreaker::CircuitBreaker(Service *service):service{service}
{
    if(service){
        state = CBSTATE::CLOSED;
    }
    else{
        state = CBSTATE::OPEN;
    }
}


int CircuitBreaker::service_1(int in)
{
    int ret = -1;
    if(state == CBSTATE::CLOSED){
        ret = service->service_1(in);
        if(ret > 10){
            state = CBSTATE::OPEN;
        }
    }
    return ret;
}

int CircuitBreaker::service_2(int in)
{
    int ret = -1;
    if(state == CBSTATE::CLOSED){
        ret = service->service_2(in);
        if(ret > 10){
            state = CBSTATE::OPEN;
        }
    }
    return ret;
}

int CircuitBreaker::service_3(int in)
{
    int ret = -1;
    if(state == CBSTATE::CLOSED){
        ret = service->service_3(in);
        if(ret > 10){
            state = CBSTATE::OPEN;
        }
    }
    return ret;
}
