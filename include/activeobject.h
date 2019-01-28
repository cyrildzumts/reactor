#ifndef ACTIVEOBJECT_H
#define ACTIVEOBJECT_H



#include "function_wrapper.h"
#include "safe_queue.h"
#include <log.h>
#include <atomic>
#include <future>
#include <thread>

/***********************************************************************
 * Author : Cyrille Ngassam Nkwenga
 * 2019
 * Implementation of the Active Object Patterns
 * *********************************************************************/



namespace concurrency {

/**
 * @brief The Active class implements the Active Object pattern.
 * The Active Object pattern decouple the method invocation from the method execution.
 * Every submitted tasks are run by only one thread. If you want to use the more threads
 * considere using the Thread Pool Pattern.
 */
class Active{

public:

    Active(): done(false){
        worker = std::unique_ptr<std::thread>(new std::thread(&Active::run, this));
    }

    ~Active(){
        interrupt();
        if(worker->joinable()){
            worker->join();
        }
    }

    /**
     * a template function which accept any callable object taking an arbitrary type of parameters.
     * This method is only define for movable objects.
     */
    template<typename Callable, typename... Args>
    std::future<std::invoke_result_t<std::decay_t<Callable>, std::decay_t<Args>...>>
    submit(Callable &&op, Args&&... args){
        using result_type =std::invoke_result_t<std::decay_t<Callable>, std::decay_t<Args>...>;
        std::packaged_task<result_type()> task(std::bind(std::forward<Callable>(op), std::forward<Args>(args)...));
        std::future<result_type> result(task.get_future());
        task_queue.push(std::move(task));
        ++submitted;
        return result;
    }

    /**
     * @brief run the worker entry point.
     */
    void run(){
        while(!done){
            FunctionWrapper task;
            task_queue.wait_and_pop(task);
            active = true;
            task();
            active = false;
            ++finished_tasks;
            std::this_thread::yield();
        }
    }


    /**
     * @brief interrupt sends an end signal to running thread.
     */
    void interrupt(){
        submit([&]{
            done = true;
        });
        interrupted = true;
    }
private:
    /**
     * @brief active the state of the worker thread.
     *
     */
    bool active;
    /**
     * @brief interrupted flag incating if the worker thread is interrupted
     */
    bool interrupted;
    /**
     * @brief submitted the number of submktted task until now.
     */
    size_t submitted;

    /**
     * @brief finished_tasks the number of finished task.
     */
    size_t finished_tasks;
    /**
     * @brief done flag to signal the worker thread that he must terminate now.
     */
    std::atomic_bool done;
    /**
     * @brief task_queue a task queue containing the submited tasks.
     */
    ThreadSafeQueue<FunctionWrapper> task_queue;
    /**
     * @brief worker a thread that when started executes the run() method.
     */
    std::unique_ptr<std::thread> worker;
};

}

#endif // ACTIVEOBJECT_H
