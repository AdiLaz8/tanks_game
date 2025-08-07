#include "ThreadPool.h"
#include <iostream>

ThreadPool::ThreadPool(size_t numThreads) : stop(false) {
    std::cout << "Creating ThreadPool with " << numThreads << " threads" << std::endl;
    
    for(size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] {
            for(;;) {
                std::function<void()> task;
                
                {
                    std::unique_lock<std::mutex> lock(this->queueMutex);
                    this->condition.wait(lock, [this]{ return this->stop || !this->tasks.empty(); });
                    
                    if(this->stop && this->tasks.empty()) {
                        return;
                    }
                    
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                
                try {
                    task();
                } catch (const std::exception& e) {
                    std::cerr << "ThreadPool task threw exception: " << e.what() << std::endl;
                } catch (...) {
                    std::cerr << "ThreadPool task threw unknown exception" << std::endl;
                }
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    
    for(std::thread &worker: workers) {
        if(worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::waitForCompletion() {
    std::unique_lock<std::mutex> lock(queueMutex);
    condition.wait(lock, [this]{ return tasks.empty(); });
}