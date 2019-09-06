//
// Created by timafh on 06.09.2019.
//

#include "CpPrioThreadPool.h"

namespace Cp {
    namespace ThreadPool {
        template<typename F, typename Class, typename... Args>
        auto CpPrioThreadPool::executeClassMember(int prio, F function, Class cl, Args &&... args) {
            std::unique_lock<std::mutex> queue_lock(_task_mutex, std::defer_lock);
            std::packaged_task<std::invoke_result_t<F, Class, Args...>()> task_pkg(
                    std::bind(function, cl, args...)
            );
            std::future<std::invoke_result_t<F, Class, Args...>> future = task_pkg.get_future();

            std::shared_ptr<TaskContainer> t = std::make_shared<TaskContainer>();
            t->priority = prio;
            t->tc = allocate_task_container(std::move(task_pkg));

            queue_lock.lock();
            _queue.push(std::move(t));
            queue_lock.unlock();

            _task_cv.notify_one();

            return std::move(future);
        }

        template<typename F, typename Class, typename... Args>
        void CpPrioThreadPool::executeClassMemberNR(int prio, F function, Class cl, Args &&... args) {
            std::unique_lock<std::mutex> queue_lock(_task_mutex, std::defer_lock);
            std::packaged_task<std::invoke_result_t<F, Class, Args...>()> task_pkg(
                    std::bind(function, cl, args...)
            );
            std::future<std::invoke_result_t<F, Class, Args...>> future = task_pkg.get_future();

            std::shared_ptr<TaskContainer> t = std::make_shared<TaskContainer>();
            t->priority = prio;
            t->tc = allocate_task_container(std::move(task_pkg));

            queue_lock.lock();
            _queue.push(std::move(t));
            queue_lock.unlock();

            _task_cv.notify_one();
        }


        template<typename F, typename... Args>
        auto CpPrioThreadPool::execute(int prio, F function, Args &&... args) {
            std::unique_lock<std::mutex> queue_lock(_task_mutex, std::defer_lock);
            std::packaged_task<std::invoke_result_t<F, Args...>()> task_pkg(
                    std::bind(function, args...)
            );
            std::future<std::invoke_result_t<F, Args...>> future = task_pkg.get_future();

            std::shared_ptr<TaskContainer> t = std::make_shared<TaskContainer>();
            t->priority = prio;
            t->tc = allocate_task_container(std::move(task_pkg));

            queue_lock.lock();
            _queue.push(std::move(t));
            queue_lock.unlock();

            _task_cv.notify_one();

            return std::move(future);
        }



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
                        )
                );
            }


        }
    }
}
