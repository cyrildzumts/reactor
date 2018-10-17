#ifndef SERVICE_H
#define SERVICE_H
#include <mutex>

class Service
{
public:
    virtual ~Service(){}

    virtual int service_1(int in) = 0;
    virtual int service_2(int in) = 0;
    virtual int service_3(int in) = 0;
};


class ConcreteService : public Service{
private:
    int service_resource_usage;
    //std::mutex service_mux;

public:
    ConcreteService();
    // Service interface
public:
    virtual int service_1(int in) override;
    virtual int service_2(int in) override;
    virtual int service_3(int in) override;
};
#endif // SERVICE_H
