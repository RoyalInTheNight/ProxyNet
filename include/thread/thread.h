#ifndef __THREAD__
#define __THREAD__

#include <condition_variable>
#include <functional>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>

template<class __Fn, class __Tp> class pool {
public:
    pool<__Fn, __Tp>
        (const uint32_t nthreads
             = std::thread::hardware_concurrency()) {
        for (uint32_t i = 0; i < nthreads; ++i) {
            pool_.emplace_back([this] {
                while (true) {
                    __Fn task;

                    {
                        std::unique_lock<std::mutex> lock(queue_mutex_pool_);

                        cv_pool_.wait(lock, [this] {
                            return !task_pool_.empty() || stop_pool_;
                        });

                        if (stop_pool_ && task_pool_.empty())
                            return false;

                        task = task_pool_.front();
                        task_pool_.pop();
                    }

                    result_pool_.push_back(task());
                }
            });
        }
    }

    void add_thread(__Fn task) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_pool_);
            task_pool_.emplace(task);
        }

        cv_pool_.notify_one();
    }

    void join() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_pool_);
            stop_pool_ = true;
        }

        cv_pool_.notify_all();

        for (auto& thread: pool_)
            thread.join();
    }

    std::vector<__Tp> pool_thread_result() const {
        return this->result_pool_;
    }

private:
    std::vector<std::thread>   pool_;
    std::queue<__Fn>      task_pool_;
    std::mutex     queue_mutex_pool_;
    std::condition_variable cv_pool_;
    std::vector<__Tp>   result_pool_;

    bool stop_pool_ = false;
};

#endif // __THREAD__