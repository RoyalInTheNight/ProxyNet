#include "../../include/thread_pool/pool.h"
#include <functional>
#include <mutex>
#include <vector>

__raw_pool::__raw_pool(uint32_t threads) {
    for (uint32_t i = 0; i < threads; ++i) {
        pool_.emplace_back([this] {
            while (true) {
                std::function<bool()> task;

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

__raw_pool::~__raw_pool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_pool_);
        stop_pool_ = true;
    }

    cv_pool_.notify_all();

    for (auto& thread: pool_)
        thread.join();
}

void __raw_pool::add_thread(std::function<bool()>& task) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_pool_);
        task_pool_.emplace(std::move(task));
    }

    cv_pool_.notify_one();
}

std::vector<bool> __raw_pool::pool_thread_result() const {
    return result_pool_;
}