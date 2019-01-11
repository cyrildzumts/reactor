#ifndef ACTIVEOBJECT_H
#define ACTIVEOBJECT_H



#include "function_wrapper.h"
#include "safe_queue.h"
#include <log.h>
#include <atomic>
#include <future>
#include <thread>


namespace concurrency {


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
        return result;
    }

    /**
     * @brief run the worker entry point.
     */
    void run(){
        while(!done){
            FunctionWrapper task;
            task_queue.wait_and_pop(task);
            task();
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
    }
private:
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
