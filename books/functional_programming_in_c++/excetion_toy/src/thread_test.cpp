#include "execution/thread.hpp"
#include "execution/execution.hpp"


#include <iostream>


int main(int argc, const char* argv[]) {
    ex::thread_context th;
    auto sched = th.get_scheduler();
    auto s5 = then(schedule(sched), [](auto){return 42;});
    auto s6 = then(s5, [](auto i) {return i + 24;});
    auto val7 = ex::sync_wait(s6);
    std::cout << std::get<1>(val7) << std::endl;
    th.finish();
    th.join();
    return 0;
}