#ifndef THREADSAFE_QUEUE
#define THREADSAFE_QUEUE
#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>

/*********************************************************
 * Author: Cyrille Ngassam Nkwenga
 * 2019
 * Desciption : This class implements a thread safe queue
 * which can be used in a multhreading environment.
 * This Queue accept any movable object
********************************************************/


template <typename T>
    class ThreadSafeQueue
    {
    public:
        ThreadSafeQueue()
        {

        }

        ThreadSafeQueue( ThreadSafeQueue const& other)
        {
            std::lock_guard<std::mutex>locker(other.itemMutex);
            items = other.items;
        }

        ThreadSafeQueue(ThreadSafeQueue&& other)
        {
            std::lock_guard<std::mutex>locker(other.itemMutex);
            items = std::move(other.items);
            other.clear();
        }

        ThreadSafeQueue& operator =(ThreadSafeQueue&& other)
        {
            std::lock(itemMutex, other.itemMutex);
            std::lock_guard<std::mutex> this_lock(itemMutex,std::adopt_lock);
            std::lock_guard<std::mutex> other_lock(other.itemMutex,std::adopt_lock);
            items = std::move(other.items);
            other.clear();
            return *this;

        }

        ThreadSafeQueue& operator =(ThreadSafeQueue& other)
        {
            std::lock(itemMutex, other.itemMutex);
            std::lock_guard<std::mutex> this_lock(itemMutex,std::adopt_lock);
            std::lock_guard<std::mutex> other_lock(other.itemMutex,std::adopt_lock);
            items = std::move(other.items);
            return *this;

        }

        /**
         * @brief push pushes item into the queue
         * @param item the element to be pushed into the queue
         */
        void push(T item)
        {
            std::lock_guard<std::mutex>locker(itemMutex);
            items.push(std::move(item));
            itemCond.notify_one();
        }

        /**
         * @brief wait_and_pop takes the last insert item from the queue.
         *
         * @param item contains the removed element
         */
        void wait_and_pop(T& item)
        {
            std::unique_lock<std::mutex> locker(itemMutex);
            itemCond.wait(locker,[this]{return !items.empty();});
            item = std::move(items.front());
            items.pop();
        }

        /**
         * @brief wait_and_pop takes the last insert item from the queue.
         * @return handle to the removed element
         */
        std::shared_ptr<T> wait_and_pop()
        {
            std::unique_lock<std::mutex> locker(itemMutex);
            itemCond.wait(locker,[this]{return !items.empty();});
            std::shared_ptr<T> result (std::make_shared<T> (std::move(items.front())));
            items.pop();
            return result;
        }

        /**
         * @brief try_pop non blocking removing method. this method try to remove
         * the last inserted element from the queue.
         * @param item contains the removed inserted element.
         * @return true on success. False is returned when the queue is empty.
         */
        bool try_pop(T& item)
        {
            std::lock_guard<std::mutex> locker(itemMutex);
            if(items.empty())
                return false;
            item = std::move(items.front());
            items.pop();
            return true;
        }

        /**
         * @brief try_pop non blocking removing method. this method try to remove
         * the last inserted element from the queue.
         *
         * @return handle to the removed element when the queue is not empty.
         * returns an invalid pointer when the queue is empty.
         */
        std::shared_ptr<T> try_pop()
        {
            std::lock_guard<std::mutex> locker(itemMutex);
            if(items.empty())
                return std::shared_ptr<T>();
            std::shared_ptr<T> result (std::make_shared<T> (std::move(items.front())));
            items.pop();
            return result;
        }

        /**
         * @brief empty checks whether the queue is empty
         * @return true when the queue is empty, else false is returned.
         */
        bool empty()const
        {
            std::lock_guard<std::mutex> locker(itemMutex);
            return items.empty();
        }

        /**
         * @brief size get the current size of the queue.
         * @return the size of the queue
         */
        int size()const
        {
            return items.size();
        }

        /**
         * @brief clear clears the queue's content.
         */
        void clear()
        {
            items = {};
        }

    private:
        std::queue<T> items;

        mutable std::mutex itemMutex;
        std::condition_variable itemCond;
    };

#endif //THREADSAFE_QUEUE
