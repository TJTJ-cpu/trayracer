#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <functional>
#include <condition_variable>

class ThreadPool {
public:
    // Public interface
    void SpawnThread();
    void QueueJob(const std::function<void()>& job);
    bool Busy();
    void Stop();

private:
    // Internal function for thread loop
    void ThreadLoop();

    // Member variables
    std::vector<std::thread> Threads;
    std::queue<std::function<void()>> Jobs;
    std::condition_variable MutexCondition;
    std::mutex QueueMutex;
    std::atomic<bool> bShouldTerminate = false;
    //bool bShouldTerminate = false;
};

#endif // THREADPOOL_H