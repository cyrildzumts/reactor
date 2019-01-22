#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "common.h"
#include "function_wrapper.h"
#include "safe_queue.h"
#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <future>

#define WORKERS_SIZE 3


/**
 * @brief The ThreadJoins class this class is a helper  class.
 * It job is to join the threads list it contains when it is destructed.
 */
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
    }
};


class ThreadPool{
private:
    /**
     * @brief submitted the number of submitted task since the thread pool
     * was created.
     */
    std::atomic_size_t submitted;

    /**
     * @brief finished_tasks the of finished task since the thread pool
     * was started.
     */
    std::atomic_size_t finished_tasks;

    /**
     * @brief active_task the number of active task being processed now.
     */
    std::atomic_size_t active_task;

    /**
     * @brief done flag indicating that a thread should terminate and quit.
     */
    std::atomic_bool done;

    /**
     * @brief workers_count current number of available threads used by this thread pool.
     */
    size_t workers_count;
    /**
     * @brief task_queue contains the tasks to be run.
     */
    ThreadSafeQueue<FunctionWrapper> task_queue;
    /**
     * @brief threads list of actual threads in use.
     */
    std::vector<std::thread> threads;
    //ThreadJoins joiner;
    /**
     * @brief run entry point for created thread.
     */
    void run(){
        while (!done) {
            FunctionWrapper task;
            task_queue.wait_and_pop(task);
            active_task.fetch_add(1, std::memory_order_relaxed);;
            task();
            active_task.fetch_sub(1, std::memory_order_relaxed);;
            finished_tasks.fetch_add(1, std::memory_order_relaxed);;
            std::this_thread::yield();
        }
    }

public:
    ThreadPool(std::optional<size_t> number_of_workers = std::nullopt): done{false}{

        submitted = 0;
        active_task = 0;
        finished_tasks = 0;
        if(number_of_workers && (*number_of_workers > 0)){
            workers_count = *number_of_workers;
        }
        else {
            workers_count = WORKERS_SIZE;
        }

        for(size_t i = 0; i < workers_count; ++i){
            threads.push_back(std::thread(&ThreadPool::run, this));
        }
    }

    ~ThreadPool(){
        interrupt();
        // Wait for every worker to finish before quitting.
        std::for_each(threads.begin(), threads.end(), [&](std::thread &t){
            if(t.joinable()){
                t.join();
            }
        });
    }
    /**
     * @brief submit place a new task in the task queue.  This new task will
     * be processed by a free thread.
     * When there is no free thread, the task stays in the task queue until a thread
     * becomes available to run it.
     * this threadpool can process any any task with any signature.
     */
    template<typename Callable, typename... Args,typename = std::enable_if_t<std::is_move_constructible_v<Callable>>>
    std::future<std::invoke_result_t<Callable, Args...>> submit(Callable &&op, Args&&... args){
        using result_type =std::invoke_result_t<Callable, Args...>;
        std::packaged_task<result_type()> task(std::bind(std::forward<Callable>(op), std::forward<Args>(args)...));
        std::future<result_type> result(task.get_future());
        task_queue.push(std::move(task));
        submitted.fetch_add(1, std::memory_order_relaxed);
        return result;
    }

    /**
     * @brief interrupt interrupts all currently created thread
     */
    void interrupt(){
        std::for_each(threads.begin(), threads.end(), [&](std::thread &t){
            if(t.joinable()){
                submit([&]{
                    done = true;
                });
            }
        });
    }
    size_t getSubmitted() const{
        return submitted;
    }
    size_t getFinishedTasks() const{
        return finished_tasks;
    }
    size_t getActiveTask() const{
        return active_task;
    }
};


#endif // THREADPOOL_H
