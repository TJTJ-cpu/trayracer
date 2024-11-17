//#include "ThreadPool.h"
//
//void ThreadPool::SpawnThread() {
//    int Cores = std::thread::hardware_concurrency();
//    for (int i = 0; i < Cores; i++) {
//        Threads.emplace_back(&ThreadPool::ThreadLoop, this);
//    }
//}
//
//void ThreadPool::ThreadLoop() {
//    while (true) {
//        std::function<void()> job;
//        {
//            std::unique_lock<std::mutex> lock(QueueMutex);
//            MutexCondition.wait(lock, [this] {
//                return !Jobs.empty() || bShouldTerminate;
//                });
//            if (bShouldTerminate)
//                return;
//            job = Jobs.front();
//            Jobs.pop();
//        }
//        job();
//    }
//}
//
//bool ThreadPool::Busy() {
//    bool bPoolBusy;
//    {
//		std::unique_lock<std::mutex> lock(QueueMutex);
//        bPoolBusy = !Jobs.empty();
//    }
//    return bPoolBusy;
//}
//
//void ThreadPool::Stop() {
//    {
//        std::unique_lock<std::mutex> lock(QueueMutex);
//        bShouldTerminate = true;
//    }
//    MutexCondition.notify_all();
//    for (std::thread& ActiveThread : Threads) {
//        ActiveThread.join();
//    }
//    Threads.clear();
//}
