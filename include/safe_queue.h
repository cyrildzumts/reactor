#ifndef THREADSAFE_QUEUE
#define THREADSAFE_QUEUE
#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>

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

        void push(T item)
        {
            std::lock_guard<std::mutex>locker(itemMutex);
            items.push(std::move(item));
            itemCond.notify_one();
        }

        void wait_and_pop(T& item)
        {
            std::unique_lock<std::mutex> locker(itemMutex);
            itemCond.wait(locker,[this]{return !items.empty();});
            item = std::move(items.front());
            items.pop();
        }

        std::shared_ptr<T> wait_and_pop()
        {
            std::unique_lock<std::mutex> locker(itemMutex);
            itemCond.wait(locker,[this]{return !items.empty();});
            std::shared_ptr<T> result (std::make_shared<T> (std::move(items.front())));
            items.pop();
            return result;
        }

        bool try_pop(T& item)
        {
            std::lock_guard<std::mutex> locker(itemMutex);
            if(items.empty())
                return false;
            item = std::move(items.front());
            items.pop();
            return true;
        }

        std::shared_ptr<T> try_pop()
        {
            std::lock_guard<std::mutex> locker(itemMutex);
            if(items.empty())
                return std::shared_ptr<T>();
            std::shared_ptr<T> result (std::make_shared<T> (std::move(items.front())));
            items.pop();
            return result;
        }

        bool empty()const
        {
            std::lock_guard<std::mutex> locker(itemMutex);
            return items.empty();
        }

        int size()const
        {
            return items.size();
        }

        void clear()
        {
            items = {};
        }
        std::queue<T> getContainer()
        {
            return items;
        }

    private:
        std::queue<T> items;

        mutable std::mutex itemMutex;
        std::condition_variable itemCond;
    };

#endif //THREADSAFE_QUEUE
