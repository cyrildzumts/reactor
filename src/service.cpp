#include "service.h"
#include <iostream>


ConcreteService::ConcreteService(): service_resource_usage{0}
{

}

int ConcreteService::service_1(int in)
{
    service_resource_usage++;
    std::cout << __PRETTY_FUNCTION__ << " usage : " << service_resource_usage << std::endl;
    return service_resource_usage;
}

int ConcreteService::service_2(int in)
{
    service_resource_usage++;
    std::cout << __PRETTY_FUNCTION__ << " usage : " << service_resource_usage << std::endl;
    int ret = service_1(in) + in;

    return service_resource_usage;
}

int ConcreteService::service_3(int in)
{
    service_resource_usage++;
    std::cout << __PRETTY_FUNCTION__ << " usage : " << service_resource_usage << std::endl;
    int ret = service_2(in) + in;

    return service_resource_usage;
}
