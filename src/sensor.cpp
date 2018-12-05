#include "sensor.h"

abstractADConverter::~abstractADConverter(){

}


ADConverter::ADConverter(ISendor *sensor): sensor{sensor}{

}

void ADConverter::init(){
    last_temp = 0.0;
}

void ADConverter::reset(){
    sensor->reset();
}

int ADConverter::start_convertion(){
    if(sensor){
        last_temp = sensor->read_value();
        return 0;
    }
    return  -1;
}


bool ADConverter::conversion_finished(){
    return true;
}

double ADConverter::get_last_conversion()
{
    return last_temp;
}
