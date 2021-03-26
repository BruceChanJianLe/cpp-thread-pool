#include <functional>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <iostream>
#include <chrono>


class threadPool
{
    public:
        explicit threadPool(std::size_t numThreads)
        : stopping_flag_(false)
        {
            // Reserve vector size for thread list
            thread_list_.reserve(numThreads);

            // Start all threads in thread_list
            start(numThreads);
        }

        void enqueue(std::function<void()> task)
        {
            // Lock mutex while adding new task
            {
                std::unique_lock<std::mutex> lock{event_var_mutex};
                // Move task to queue
                task_queue_.emplace(std::move(task));
            }

            // Notify one of the waiting thread
            event_var_.notify_one();
        }

    private:
        // A vector of thread
        std::vector<std::thread> thread_list_;

        // Condition variable (to save cpu resource)
        std::condition_variable event_var_;
        std::mutex event_var_mutex;
        bool stopping_flag_;

        // Queue for tasks
        std::queue<std::function<void()>> task_queue_;

        void start(std::size_t numThreads)
        {
            for(auto i = 0u; i < numThreads; ++i)
            {
                thread_list_.emplace_back(
                    [this]()
                    {
                        while(true)
                        {
                            // Lock and validation scope
                            {
                                std::function<void()> task_to_be_run;
                                {
                                    // Lock share mutex (event_var_mutex)
                                    std::unique_lock<std::mutex> lock{this->event_var_mutex};

                                    this->event_var_.wait(
                                        lock,
                                        [this]()
                                        {
                                            return this->stopping_flag_ || !this->task_queue_.empty();
                                        }
                                    );

                                    // Exit condition
                                    if(this->stopping_flag_ && this->task_queue_.empty())
                                        break;

                                    // Move queue task in this thread
                                    task_to_be_run = std::move(this->task_queue_.front());
                                    // Pop first task
                                    this->task_queue_.pop();
                                }

                                // Run task
                                task_to_be_run();
                            }
                        }
                    }
                );
            }
        }

        void stop() noexcept
        {
            // Set stopping flag to true scope
            {
                std::unique_lock<std::mutex> lock{event_var_mutex};
                stopping_flag_ = true;
            }

            // Notify all threads to stop
            event_var_.notify_all();

            // Wait for all threads to join before deconstruction
            for(auto & thread : thread_list_)
            {
                if(thread.joinable())
                    thread.join();
            }
        }
};

int main()
{
    threadPool pool(16);

    for(int i = 0; i < 32; ++i)
    {
        pool.enqueue(
            [=]()
            {
                std::cout << i << std::endl;
            }
        );
    }

    std::this_thread::sleep_for(std::chrono::seconds(10));

    return 0;
}