//
// Created by timafh on 06.09.2019.
//

#ifndef CPTHREADPOOL_CPTHREADPOOL_H
#define CPTHREADPOOL_CPTHREADPOOL_H

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


        class CpThreadPool {
        public:
            CpThreadPool() {}

            CpThreadPool(size_t thread_count) : _threadCount(thread_count) {}

            void init();

            void init(size_t threadCount);

            ~CpThreadPool();

            template<typename F, typename ...Args>
            auto execute(F, Args &&...);

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

            std::vector<std::thread> _threads;
            std::queue<std::unique_ptr<_task_container_base>> _tasks;
            std::mutex _task_mutex;
            std::condition_variable _task_cv;
            bool _stop_threads = false;
        };
    }
}


#endif //CPTHREADPOOL_CPTHREADPOOL_H
