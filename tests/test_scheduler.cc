#include "lyslg.h"


lyslg::Logger::ptr g_logger = LYSLG_LOG_ROOT();


void test_fiber() {
    LYSLG_LOG_INFO(g_logger) << "test in fiber";
    static int s_count = 5;
    sleep(1);

    if(--s_count >= 0) {
        lyslg::Scheduler::GetThis()->schedule(&test_fiber,lyslg::GetThreadId());
    }
 
}

int main(int argc,char** argv) {
// lyslg::Scheduler sc(); // 被解释为函数声明
    LYSLG_LOG_INFO(g_logger) << "main";
    lyslg::Scheduler sc(3,false,"test");
    sc.start();
    LYSLG_LOG_INFO(g_logger) << "schedule";
    sc.schedule(&test_fiber);
    sc.stop();
    LYSLG_LOG_INFO(g_logger) << "over";
    return 0;
}