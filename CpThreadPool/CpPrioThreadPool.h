//
// Created by timafh on 06.09.2019.
//

#ifndef CPTHREADPOOL_CPPRIOTHREADPOOL_H
#define CPTHREADPOOL_CPPRIOTHREADPOOL_H

#include <future>
#include <vector>
#include <functional>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <type_traits>

namespace Cp {
    namespace ThreadPool {


        class CpPrioThreadPool {
        public:
            CpPrioThreadPool() {}
            CpPrioThreadPool(size_t thread_count) : _threadCount(thread_count)  {}

            void init();
            void init(size_t threadCount);
            ~CpPrioThreadPool();

            void startTasks() {
                this->_tasks_paused = false;
            }

            void stopTasks() {
                this->_tasks_paused = true;
            }


            template<typename F, typename ...Args>
            auto execute(int prio, F function, Args &&... args) {
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


            template<typename F, typename Class, typename ...Args>
            auto executeClassMember(int prio, F function, Class cl, Args &&...args) {
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



            template<typename F, typename Class, typename ...Args>
            void executeClassMemberNR(int prio, F function, Class cl, Args &&... args) {
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

        private:

            size_t _threadCount;

            class _task_container_base {
            public:
                virtual ~_task_container_base() {};

                virtual void operator()() = 0;
            };

            template<typename F>
            class _task_container : public _task_container_base {
            public:
                _task_container(F &&func) : _f(std::forward<F>(func)) {};

                void operator()() override {
                    _f();
                };

            private:
                F _f;
            };



            template<typename _Func>
            static std::unique_ptr<_task_container_base> allocate_task_container(_Func &&f) {
                return std::unique_ptr<_task_container_base>(new _task_container<_Func>(std::forward<_Func>(f)));
            };

            class TaskContainer {
            public:
                ~TaskContainer() {
                    tc.release();
                }
                std::unique_ptr<_task_container_base> tc;
                int priority;
            };

            class QueueCompare {
            public:
                bool operator() (std::shared_ptr<TaskContainer> t1, std::shared_ptr<TaskContainer> t2) {
                    return t1->priority < t2->priority;
                }
            };

            class QueueCompareUnique {
            public:
                bool operator() (const std::unique_ptr<TaskContainer>& t1, const std::unique_ptr<TaskContainer>& t2) {
                    return t1->priority < t2->priority;
                }
            };

            std::priority_queue<std::shared_ptr<TaskContainer>, std::vector<std::shared_ptr<TaskContainer>>, QueueCompare> _queue;
            std::vector<std::thread> _threads;
            std::mutex _task_mutex;
            std::condition_variable _task_cv;
            bool _stop_threads = false;
            bool _tasks_paused = true;
        };

    }
}

#endif //CPTHREADPOOL_CPPRIOTHREADPOOL_H
