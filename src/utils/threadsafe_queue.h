
#ifndef __M2E_BRIDGE_THREADSAFEQUEUE_H__
#define __M2E_BRIDGE_THREADSAFEQUEUE_H__


#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;

    // Add an element to the queue
    void enqueue(T value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(value));
        cond_var_.notify_one();  // Notify one waiting thread (if any)
    }

    // Remove and return an element from the queue
    // If the queue is empty, wait until an element is available
    T dequeue() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_var_.wait(lock, [this]{ return !queue_.empty(); });
        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }

    // Check if the queue is empty
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

private:
    mutable std::mutex mutex_;  // Mutex to protect access to the queue
    std::queue<T> queue_;       // The underlying queue
    std::condition_variable cond_var_; // Condition variable to manage waiting
};

#endif