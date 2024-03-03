#include "daemon.h"
#include "iomanager.h"
#include "log.h"

static lyslg::Logger::ptr g_logger = LYSLG_LOG_ROOT();

lyslg::Timer::ptr timer;
int server_main(int argc, char** argv) {
    LYSLG_LOG_INFO(g_logger) << lyslg::ProcessInfoMgr::GetInstance()->toString();
    lyslg::IoManager iom(1);
    timer = iom.addTimer(1000, [](){
            LYSLG_LOG_INFO(g_logger) << "onTimer";
            static int count = 0;
            if(++count > 10) {
                // timer->cancel();
                exit(0);
            }
    }, true);
    return 0;
}

int main(int argc, char** argv) {
    LYSLG_LOG_INFO(g_logger) << "argc=" << argc;
    return lyslg::start_daemon(argc, argv, server_main, argc != 1);
}
