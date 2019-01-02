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

    Active(): done(false), is_working(false){
        worker = std::unique_ptr<std::thread>(new std::thread(&Active::run, this));
    }

    ~Active(){
        LOG("Active object quiting ...");
        LOG("Active object send end signal ...");
        LOG("Active Queue size before  : ", work_queue.size());
        //work_queue.clear();
        interrupt();
        LOG("Active object end signal sent ...");
        if(worker->joinable()){
            LOG("Active Object thread joinable");
            LOG("Active Queue size : ", work_queue.size());
            worker->join();
        }
        LOG("Active Object quitted ...");
    }

    template<typename Callable, typename... Args,typename = std::enable_if_t<std::is_move_constructible_v<Callable>>>
        std::future<std::invoke_result_t<Callable, Args...>> submit(Callable &&op, Args&&... args){
            using result_type =std::invoke_result_t<Callable, Args...>;
            std::packaged_task<result_type()> task(std::bind(std::forward<Callable>(op), std::forward<Args>(args)...));
            std::future<result_type> result(task.get_future());
            work_queue.push(std::move(task));
            return result;
        }

    void run(){
        LOG("Active thread", "thread id : ", std::this_thread::get_id());
        while(!done){
            FunctionWrapper task;
            work_queue.wait_and_pop(task);
            is_working = true;
            task();
            is_working = false;
            std::this_thread::yield();
        }
        LOG("Active object thread quitting ...", "Active thread", "thread id : ", std::this_thread::get_id());
    }

    void interrupt(){
        //work_queue.clear();
        submit([&]{
            done = true;
            LOG("Active Object Interrupt set", " now quitting");
        });

    }
private:
    //bool done;
    std::atomic_bool done;
    bool is_working;
    ThreadSafeQueue<FunctionWrapper> work_queue;
    std::unique_ptr<std::thread> worker;
};

}

#endif // ACTIVEOBJECT_H
