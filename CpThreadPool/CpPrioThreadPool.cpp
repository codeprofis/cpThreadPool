//
// Created by timafh on 06.09.2019.
//

#include "CpPrioThreadPool.h"

namespace Cp {
    namespace ThreadPool {

        CpPrioThreadPool::~CpPrioThreadPool() {
            _stop_threads = true;
            _task_cv.notify_all();

            for (std::thread &thread : _threads) {
                thread.join();
            }
        }


        void CpPrioThreadPool::init() {
            this->init(4);
        }

        void CpPrioThreadPool::init(size_t threadCount) {
            this->_threadCount = threadCount;

            for (size_t i = 0; i < this->_threadCount; i++) {
                _threads.emplace_back(
                        std::thread(
                                [&]() {
                                    std::unique_lock<std::mutex> queue_lock(_task_mutex, std::defer_lock);

                                    while (true) {
                                        if(!this->_tasks_paused) {
                                            std::this_thread::sleep_for(std::chrono::milliseconds(500));
                                            queue_lock.lock();
                                            _task_cv.wait(
                                                    queue_lock,
                                                    [&]() -> bool { return !_queue.empty() || _stop_threads; }
                                            );

                                            if (_stop_threads && _queue.empty()) return;

                                            //auto temp_task = std::move(_queue.top());
                                            std::shared_ptr<TaskContainer> temp_task = std::move(_queue.top());
                                            _queue.pop();
                                            queue_lock.unlock();

                                            (*temp_task->tc)();
                                        }
                                    }
                                }
                        )
                );
            }


        }
    }
}
