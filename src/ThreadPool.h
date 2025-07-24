#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <future>
#include <functional>
#include <mutex>
#include <condition_variable>

class ThreadPool {
public:
    explicit ThreadPool(size_t threads);
    ~ThreadPool();

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type>;
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

#include "ThreadPool.tpp"
