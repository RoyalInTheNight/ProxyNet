#ifndef __POOL_THREADS
#define __POOL_THREADS

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class __raw_pool {
public:
    __raw_pool(uint32_t threads = std::thread::hardware_concurrency());

    void add_thread(std::function<bool()>&);

    std::vector<bool> pool_thread_result() const;

    ~__raw_pool();

private:
    std::vector<std::thread>               pool_;
    std::queue<std::function<bool()>> task_pool_;
    std::mutex                 queue_mutex_pool_;
    std::condition_variable             cv_pool_;
    std::vector<bool>               result_pool_;

    bool stop_pool_ = false;
};

#endif // __POOL_THREADS