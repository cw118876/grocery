#include <iostream>

#include "scheduler.hpp"

Task jobA(Scheduler& sch) {
    std::cout << "jobA: enterred\n";
    co_await sch.suspend();
    std::cout << "jobA: resume executing after suspend\n";
    co_await sch.suspend();
    std::cout << "JobA: work is done\n";
}


Task jobB(Scheduler& sch) {
    std::cout << "jobB: enterred\n";
    co_await sch.suspend();
    std::cout << "jobB: resume executing after suspend\n";
    co_await sch.suspend();
    std::cout << "JobB: work is done\n";
}


int main(int argc, char* argv[]) {
    Scheduler sch;
    sch.add_task(jobA(sch).get_handle());
    sch.add_task(jobB(sch).get_handle());
    sch.run();

    return 0;

}