#ifndef CBREAKEREXCEPTION
#define CBREAKEREXCEPTION
#include <stdexcept>

/**
 * @brief The TimeoutError class An Exception thrown when the Service
 * doesn't return a response in the deadline.
 */

namespace breaker {
class TimeoutError : public std::exception{
public:
    TimeoutError(): TimeoutError(nullptr){

    }

    TimeoutError(const char *what){
        if(what){
            what_string = std::string(what);
        }
        else{
            what_string = "TIMOUT";
        }
    }
    TimeoutError(const std::string &what){
        what_string = what;
    }
    // Exception Interface
    virtual const char* what() const noexcept{
        return what_string.c_str();
    }

private:
    std::string what_string;

};
}



#endif // CBREAKEREXCEPTION
