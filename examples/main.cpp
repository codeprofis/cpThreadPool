#include <iostream>
#include <vector>

#include "CpThreadPool/CpThreadPool.h"
#include "CpThreadPool/CpPrioThreadPool.h"

std::mutex coutMutex;

int multiply(int x, int y) {
    srand(x*7);
    int waiting = 5 + (std::rand() % (20 - 5 +1));
    coutMutex.lock();
    std::cout << "Thread (" << x << "x" << y << "): " << std::this_thread::get_id() << std::endl;
    std::cout << "Waiting for " << waiting << " Seconds" << std::endl;
    coutMutex.unlock();

    std::this_thread::sleep_for(std::chrono::seconds(waiting));

    coutMutex.lock();
    std::cout << "Thread (" << x << "x" << y << ") Result: " << x*y << std::endl;
    coutMutex.unlock();

    return x*y;
}

class Entry {
public:
    int t;
};

class T {
public:

    class Argument {
    public:
        Entry x;
        int y;
    };

    int m(int x, int y) {

        srand(x*7);
        int waiting = 5 + (std::rand() % (20 - 5 +1));
        coutMutex.lock();
        std::cout << "CLASS Thread (" << x << "x" << y << "): " << std::this_thread::get_id() << std::endl;
        std::cout << "CLASS Waiting for " << waiting << " Seconds" << std::endl;
        coutMutex.unlock();

        std::this_thread::sleep_for(std::chrono::seconds(waiting));

        coutMutex.lock();
        std::cout << "CLASS Thread (" << x << "x" << y << ") Result: " << x*y << std::endl;
        coutMutex.unlock();

        return x * y;
    }

    int m2(Argument t) {
        std::cout << t.x.t*t.y;
        return t.x.t*t.y;
    }

    void m4() {
        Cp::ThreadPool::CpPrioThreadPool pool(2);
        pool.init();

        std::cout << "Pool inited" << std::endl;

        /*pool.execute(1, multiply, 15, 2);
        pool.execute(1, multiply, 30, 2);
        pool.execute(10, multiply, 60, 2);
        pool.execute(40, multiply, 120, 2);*/
       //pool.executeClassMember(30, &T::m, &t, 140, 2);

        T::Argument a;
        a.x.t = 2;
        a.y = 2;
        pool.executeClassMember(30, &T::m2, this, a);
    }


};

int main() {
   /*  T t;

    Cp::ThreadPool::CpPrioThreadPool pool(2);
    pool.init();

    std::cout << "Pool inited" << std::endl;


    pool.executeClassMember(30, &T::m, &t, 140, 2);

    T::Argument a;
    a.x = 2;
    a.y = 2;
    pool.executeClassMember(30, &T::m2, &t, a);*/


   T t;
   t.m4();

    return 0;
}

