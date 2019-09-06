//
// Created by timafh on 06.09.2019.
//

#include "CpThreadPool.h"

namespace Cp {
    namespace ThreadPool {


        template<typename F, typename... Args>
        auto CpThreadPool::execute(F function, Args &&... args) {
            std::unique_lock<std::mutex> queue_lock(_task_mutex, std::defer_lock);
            std::packaged_task<std::invoke_result_t<F, Args...>()> task_pkg(
                    std::bind(function, args...)
            );
            std::future<std::invoke_result_t<F, Args...>> future = task_pkg.get_future();

            queue_lock.lock();
            _tasks.emplace(
                    //allocate_task_container([task(std::move(task_pkg))]()mutable { task(); })
                    allocate_task_container(std::move(task_pkg))
            );
            queue_lock.unlock();

            _task_cv.notify_one();

            return std::move(future);
        }


        CpThreadPool::~CpThreadPool() {
            _stop_threads = true;
            _task_cv.notify_all();

            for (std::thread &thread : _threads) {
                thread.join();
            }
        }

        void CpThreadPool::init() {
            this->init(4);
        }

        void CpThreadPool::init(size_t threadCount) {
            this->_threadCount = threadCount;

            for (size_t i = 0; i < this->_threadCount; i++) {
                _threads.emplace_back(
                        std::thread(
                                [&]() {
                                    std::unique_lock<std::mutex> queue_lock(_task_mutex, std::defer_lock);

                                    while (true) {
                                        queue_lock.lock();
                                        _task_cv.wait(
                                                queue_lock,
                                                [&]() -> bool { return !_tasks.empty() || _stop_threads; }
                                        );

                                        if (_stop_threads && _tasks.empty()) return;

                                        auto temp_task = std::move(_tasks.front());

                                        _tasks.pop();
                                        queue_lock.unlock();

                                        (*temp_task)();
                                    }
                                }
                        )
                );
            }
        }
    }
}
