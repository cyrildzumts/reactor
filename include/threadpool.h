#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "function_wrapper.h"
#include "safe_queue.h"
#include <log.h>
#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <future>

class ThreadJoins{
private:
    std::vector<std::thread> &threads;

public:
    explicit ThreadJoins(std::vector<std::thread> &t):threads{t}{

    }

    ~ThreadJoins(){
        for(size_t i = 0; i< threads.size(); ++i){
            if(threads[i].joinable()){
                threads[i].join();
            }
        }
        LOG("THREADPOOL has quitted");
    }
};


class ThreadPool{
private:
    std::atomic_bool done;
    size_t workers_count;
    ThreadSafeQueue<FunctionWrapper> work_queue;
    std::vector<std::thread> threads;
    ThreadJoins joiner;
    void run(){
        while (!done) {
            FunctionWrapper task;
//            if(work_queue.try_pop(task)){
//                task();
//                std::this_thread::yield();
//            }
//            else{
//                std::this_thread::yield();
//            }
            LOG("THREADPOOL", " waiting for task to run from thread : ", std::this_thread::get_id() );
            work_queue.wait_and_pop(task);
            LOG("THREADPOOL", " got a task to run from thread : ", std::this_thread::get_id() );
            task();
            std::this_thread::yield();
        }
    }

public:
    ThreadPool(): done{false}, joiner{threads}{
        //workers_count = std::thread::hardware_concurrency();
        workers_count = 4;
        LOG("THREADPOOL ", "Current HARDWARE CONCURRENCY ", workers_count);
        try {
            for(unsigned i = 0; i < workers_count; ++i){
                threads.push_back(std::thread(&ThreadPool::run, this));
            }
        } catch (...) {
            done = true;
            throw;
        }
    }
    ~ThreadPool(){

        for(size_t i = 0; i < workers_count; i++){
            submit([&]{
                done = true;
            });
        }
        LOG("THREADPOOL is quitting ");
    }
    template<typename Func>
    std::future<typename std::result_of<Func()>::type> submit(Func f){
        using result_type =typename std::result_of<Func()>::type;
        std::packaged_task<result_type()> task(std::move(f));
        std::future<result_type> result(task.get_future());
        LOG("THREADPOOL ", " submit() entry", " thread id : ", std::this_thread::get_id());
        work_queue.push(std::move(task));
        LOG("THREADPOOL ", " submit() exit", " thread id : ", std::this_thread::get_id());
        return result;
    }

    void interrupt(){
        work_queue.push([&]{
             done = true;
         });
    }
};


#endif // THREADPOOL_H
