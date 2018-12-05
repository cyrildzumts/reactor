#ifndef CLIENT_H
#define CLIENT_H

#include "command.h"
#include "safe_queue.h"
#include <iostream>
#include <mutex>


template<typename Func, typename ...Arg>
class Client{
public:
    Client();
    ~Client();
    void call(Arg...);
private:
    Func func;
    std::mutex mux;
    ThreadSafeQueue<Func> tasks;

};


template<typename Func, typename ...Arg>
Client<Func, Arg...>::Client(){
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

template<typename Func, typename ...Arg>
Client<Func, Arg...>::~Client(){
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

template<typename Func, typename ...Arg>
void Client<Func, Arg...>::call(Arg...){
    std::lock_guard<std::mutex> guard(mux);
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

#endif // CLIENT_H
