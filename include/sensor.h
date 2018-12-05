#include "log.h"
#include <algorithm>
#include <random>

constexpr double TEMP_LIMIT = 30.5;

template<typename T>
class abstractFilter{
public:
    virtual bool filter(T value)= 0;
    virtual ~abstractFilter(){

    }
};


class TemperatureFilter : abstractFilter<double>{

public:
    bool filter(double temp){
        return temp < TEMP_LIMIT;
    }
};

template<typename T>
class abstractSensor{
public:
    virtual ~abstractSensor(){}
    virtual void init_setup() = 0;
    virtual double read_value()= 0;
    virtual void reset()=0;
};



template<typename Filter>
class TempSensor : abstractSensor<double>{
private:
    double last_temp;
    Filter filter;
    std::uniform_real_distribution<> dis;
    std::mt19937 gen;
    // abstractSensor interface
public:
    TempSensor();
    void init_setup();
    double read_value();
    void reset();
};

class abstractADConverter{

public:
    virtual void init() = 0;
    virtual void reset() = 0;
    virtual int start_convertion() = 0;
    virtual bool conversion_finished() = 0;
    virtual double get_last_conversion() = 0;
    virtual ~abstractADConverter();
};


using ISendor = abstractSensor<double>;
class ADConverter : abstractADConverter{
    // abstractADConverter interface
    ISendor *sensor;
    double last_temp;

public:
    explicit ADConverter(ISendor *sensor);
    void init();
    void reset();
    int start_convertion();
    bool conversion_finished();
    double get_last_conversion();
};

template<typename Filter>
TempSensor<Filter>::TempSensor(): gen{std::random_device()()}, dis{0.0, 30.5}{

}

template<typename Filter>
void TempSensor<Filter>::init_setup(){
    last_temp = 0;
}

template<typename Filter>
double TempSensor<Filter>::read_value(){
    return dis(gen);
}

template<typename Filter>
void TempSensor<Filter>::reset(){
    gen = std::random_device();
}


// AD
