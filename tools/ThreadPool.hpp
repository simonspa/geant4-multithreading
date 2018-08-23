#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

class ThreadPool {
public:
    /**
     * @brief Internal thread-safe queue
     */
    template <typename T> class SafeQueue {
    public:
        /**
         * @brief Default constructor, initializes empty queue
         */
        SafeQueue() = default;

        /**
         * @brief Erases the queue and release waiting threads on destruction
         */
        ~SafeQueue() { invalidate(); };

        /**
         * @brief Get the top value in the safe queue
         * @param out Reference where the value at the top of the queue will be written to
         * @param wait If the method should wait for new tasks or should exit (defaults to wait)
         * @param func Optional function to execute before releasing the queue mutex if pop was successful
         * @return True if a task was acquired or false if pop was exited for another reason
         */
        bool pop(T& out, bool wait = true, const std::function<void()>& func = nullptr) {
            // Lock the mutex
            std::unique_lock<std::mutex> lock{mutex_};
            if(wait) {
                // Wait for new item in the queue (unlocks the mutex while waiting)
                condition_.wait(lock, [this]() { return !queue_.empty() || !valid_; });
            }
            // Check for empty and valid queue
            if(queue_.empty() || !valid_) {
                return false;
            }

            // Pop the queue and optionally execute the mutex protected function
            out = std::move(queue_.front());
            queue_.pop();
            if(func != nullptr) {
                func();
            }
            return true;
        };

        /**
         * @brief Push a new value onto the safe queue
         * @param value Value to push to the queue
         */
        void push(T value) {
            std::lock_guard<std::mutex> lock{mutex_};
            queue_.push(std::move(value));
            condition_.notify_one();
        };

        /**
         * @brief Return if the queue is in a valid state
         * @return True if the queue is valid, false if \ref SafeQueue::invalidate has been called
         */
        bool isValid() const {
            std::lock_guard<std::mutex> lock{mutex_};
            return valid_;
        };

        /**
         * @brief Return if the queue is empty or not
         * @return True if the queue is empty, false otherwise
         */
        bool empty() const {
            std::lock_guard<std::mutex> lock{mutex_};
            return !valid_ || queue_.empty();
        };

        /**
         * @brief Invalidate the queue
         */
        void invalidate(){
            std::lock_guard<std::mutex> lock{mutex_};
            std::queue<T>().swap(queue_);
            valid_ = false;
            condition_.notify_all();
        };

    private:
        std::atomic_bool valid_{true};
        mutable std::mutex mutex_;
        std::queue<T> queue_;
        std::condition_variable condition_;
    };

private:
    class ThreadWorker {
    private:
        ThreadPool* pool_;

    public:
        ThreadWorker(ThreadPool* pool) : pool_(pool) {}

        void operator()() {
            std::function<void()> func;
            bool dequeued;
            while(!pool_->shutdown_) {
                {
                    std::unique_lock<std::mutex> lock(pool_->conditional_mutex_);
                    if(pool_->queue_.empty()) {
                        pool_->conditional_lock_.wait(lock);
                    }
                    dequeued = pool_->queue_.pop(func);
                }
                if(dequeued) {
                    func();
                }
            }
        }
    };

    bool shutdown_;
    ThreadPool::SafeQueue<std::function<void()>> queue_;
    std::vector<std::thread> threads_;
    std::mutex conditional_mutex_;
    std::condition_variable conditional_lock_;

public:
    ThreadPool(const unsigned int n_threads)
        : shutdown_(false), threads_(std::vector<std::thread>(n_threads)) {
        for(auto& thread : threads_) {
            thread = std::thread(ThreadWorker(this));
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;

    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    // Waits until threads finish their current task and shutdowns the pool
    void shutdown() {
        shutdown_ = true;
        conditional_lock_.notify_all();
        queue_.invalidate();

        for(auto& thrd : threads_) {
            if(thrd.joinable()) {
                thrd.join();
            }
        }
    }

    // Submit a function to be executed asynchronously by the pool
    template <typename F, typename... Args> auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        // Create a function with bounded parameters ready to execute
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        // Encapsulate it into a shared ptr in order to be able to copy construct / assign
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

        // Wrap packaged task into void function
        std::function<void()> wrapper_func = [task_ptr]() { (*task_ptr)(); };

        // Enqueue generic wrapper function
        queue_.push(wrapper_func);

        // Wake up one thread if its waiting
        conditional_lock_.notify_one();

        // Return future from promise
        return task_ptr->get_future();
    }
};

#endif
