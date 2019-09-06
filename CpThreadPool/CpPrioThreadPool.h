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


            template<typename F, typename ...Args>
            auto execute(int, F, Args &&...);


            template<typename F, typename Class, typename ...Args>
            auto executeClassMember(int, F, Class, Args &&...);


            template<typename F, typename Class, typename ...Args>
            void executeClassMember(int, F, Class, Args &&...);

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
        };

    }
}

#endif //CPTHREADPOOL_CPPRIOTHREADPOOL_H
