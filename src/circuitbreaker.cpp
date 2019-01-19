#include "circuitbreaker.h"
#include <iostream>




TimeoutError::TimeoutError():std::runtime_error("TIMEOUT"){
    //what_string = std::string("TIMEOUT REACHED");
}

const char* TimeoutError::what()const noexcept{
    return "TIMEOUT";
}


