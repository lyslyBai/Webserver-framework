#include "lyslg.h"

lyslg::Logger::ptr g_logger = LYSLG_LOG_ROOT();

void run_in_fiber()
{
    LYSLG_LOG_INFO(g_logger) << "run_in_fiber begin";
    lyslg::Fiber::YiedldToHold();
    LYSLG_LOG_INFO(g_logger) << "run_in_fiber end";
    lyslg::Fiber::YiedldToHold();
}
 
void test_fiber() {
    LYSLG_LOG_INFO(g_logger) << "main begin() -1";
    {    
        lyslg::Fiber::GetThis();
        LYSLG_LOG_INFO(g_logger) << "main begin()";
        lyslg::Fiber::ptr fiber(new lyslg::Fiber(run_in_fiber));
        fiber->swapIn();
        LYSLG_LOG_INFO(g_logger) << "main after swapIn";
        fiber->swapIn();
        LYSLG_LOG_INFO(g_logger) << "main after end";
        fiber->swapIn();
    }
    LYSLG_LOG_INFO(g_logger) << "main after end2";
}
int main()
{
    lyslg::Thread::SetName("main");
    std::vector<lyslg::Thread::ptr> thrs;
    for(int i = 0; i<3;i++) {
        thrs.push_back(lyslg::Thread::ptr(
                new lyslg::Thread(&test_fiber,"name_" + std::to_string(i))
        ));
    }
    for(auto i:thrs) {
        i->join();
    }
    return 0;
}
