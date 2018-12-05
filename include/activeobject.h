#ifndef ACTIVEOBJECT_H
#define ACTIVEOBJECT_H



#include "function_wrapper.h"
#include "safe_queue.h"
#include <log.h>
#include <future>
#include <thread>



class AbstractActive{

public:

    AbstractActive(): done(false){
        worker = std::unique_ptr<std::thread>(new std::thread(&AbstractActive::run, this));
    }

    ~AbstractActive(){
        interrupt();
        if(worker->joinable()){
            worker->join();
        }
    }
    template<typename Func>
    std::future<typename std::result_of<Func()>::type> submit(Func f){
        using result_type =typename std::result_of<Func()>::type;
        std::packaged_task<result_type()> task(std::move(f));
        std::future<result_type> result(task.get_future());
        LOG("ACTIVE OBJECT ", " submit()", " thread id : ", std::this_thread::get_id());
        work_queue.push(std::move(task));
        LOG("ACTIVE OBJECT ", " submit()", " thread id : ", std::this_thread::get_id());
        return result;
    }

    void run(){
        LOG("ACTIVE OBJECT worker started ", "thread id : ", std::this_thread::get_id());
        while(!done){
            FunctionWrapper task;
            work_queue.wait_and_pop(task);
            task();
            std::this_thread::yield();
        }
        LOG("ACTIVE OBJECT run terminated", "thread id : ", std::this_thread::get_id());
    }

    void interrupt(){
        work_queue.push([&]{
             done = true;
         });
    }
private:
    bool done;
    ThreadSafeQueue<FunctionWrapper> work_queue;
    std::unique_ptr<std::thread> worker;
};


#endif // ACTIVEOBJECT_H
