#include "daemon.h"
#include "log.h"
#include <time.h>
#include "config.h"
#include "unistd.h"
#include <sys/wait.h>
#include "util.h"

namespace lyslg{


static lyslg::Logger::ptr g_logger = LYSLG_LOG_NAME("system");
static lyslg::ConfigVar<uint32_t>::ptr g_daemon_restart_interval
    = lyslg::Config::Lookup("daemon.restart_interval", (uint32_t)5, "daemon restart interval");

std::string ProcessInfo::toString() const {
    std::stringstream ss;
    ss << "[ProcessInfo parent_id=" << parent_id
       << " main_id=" << main_id
       << " parent_start_time=" << lyslg::Time2Str(parent_start_time)
       << " main_start_time=" << lyslg::Time2Str(main_start_time)
       << " restart_count=" << restart_count << "]";
    return ss.str();
}

static int real_start(int argc,char** argv, std::function<int(int argc, char** argv)> main_cb){
    ProcessInfoMgr::GetInstance()->main_id = getpid();
    ProcessInfoMgr::GetInstance()->main_start_time = time(0);
    return main_cb(argc,argv);
}
//// daemon(int nochdir, int noclose);
// nochdir: 0表示让守护进程的工作目录切换到根目录，非0表示保持当前工作目录不变
// noclose: 0表示将标准输入、标准输出、标准错误重定向到 /dev/null，非0表示保持标准输入、标准输出、标准错误不变
static int real_daemon(int argc,char** argv, std::function<int(int argc, char** argv)> main_cb){
    daemon(1, 0);
    ProcessInfoMgr::GetInstance()->parent_id = getpid();
    ProcessInfoMgr::GetInstance()->parent_start_time = time(0);
    while(true) {
        pid_t pid = fork();
        if(pid == 0) {
            //子进程返回
            // ProcessInfoMgr::GetInstance()->main_id = getpid();
            // ProcessInfoMgr::GetInstance()->main_start_time  = time(0);
            LYSLG_LOG_INFO(g_logger) << "process start pid=" << getpid();
            return real_start(argc, argv, main_cb);
        } else if(pid < 0) {
            LYSLG_LOG_ERROR(g_logger) << "fork fail return=" << pid
                << " errno=" << errno << " errstr=" << strerror(errno);
            return -1;
        } else {
            //父进程返回
            int status = 0;
            waitpid(pid, &status, 0);
            if(status) {
                if(status == 9) {
                    LYSLG_LOG_INFO(g_logger) << "killed";
                    break;
                } else {
                    LYSLG_LOG_ERROR(g_logger) << "child crash pid=" << pid
                        << " status=" << status;
                }
            } else {
                LYSLG_LOG_INFO(g_logger) << "child finished pid=" << pid;
                break;
            }
            ProcessInfoMgr::GetInstance()->restart_count += 1;
            sleep(g_daemon_restart_interval->getValue());
        }
    }
    return 0;
}

int start_daemon(int argc,char** argv, std::function<int(int argc, char** argv)> main_cb
                    , bool is_daemon){

    if(!is_daemon) {
        LYSLG_LOG_INFO(g_logger) << "start_daemon";
        ProcessInfoMgr::GetInstance()->parent_id = getpid();
        ProcessInfoMgr::GetInstance()->parent_start_time = time(0);
        return real_start(argc,argv,main_cb);;
    }
    return real_daemon(argc,argv,main_cb);
}



}