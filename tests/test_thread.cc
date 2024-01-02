#include "lyslg.h"

lyslg::Logger::ptr g_logger = LYSLG_LOG_ROOT();

uint64_t count = 0;
lyslg::Mutex s_mutex;

void fun1() {
    LYSLG_LOG_INFO(g_logger) << "name:" << lyslg::Thread::GetName()
                             << " this.name:" << lyslg::Thread::GetThis()->getName()
                             << " id:" << lyslg::GetThreadId()
                             << " this.id:" << lyslg::Thread::GetThis()->getId();
    
    for(int i = 0;i<1000000000;i++) {
        lyslg::Mutex::Lock lock(s_mutex);
        ++count;
    }
}

void fun2() {
    while(true) {
        LYSLG_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    }
    
}

void fun3() {
    while(true) {
        LYSLG_LOG_INFO(g_logger) << "========================================";
    }
}

int main(int argc,char** argv){
    LYSLG_LOG_INFO(g_logger) << "thread test begin";
    std::vector<lyslg::Thread::ptr> thrs;
    YAML::Node root = YAML::LoadFile("/home/lyslg/lyslg_/bin/conf/log2.yml");
    lyslg::Config::LoadFromYaml(root);

    for(int i = 0;i<1000; ++i) {
        lyslg::Thread::ptr thr(new lyslg::Thread(&fun2, "name_" + std::to_string(i*2)));
        lyslg::Thread::ptr thr2(new lyslg::Thread(&fun3, "name_" + std::to_string(i*2+1)));
        thrs.push_back(thr);
        thrs.push_back(thr2);
    }

    for(size_t i = 0; i<thrs.size(); ++i) {
        thrs[i]->join();
    }

    LYSLG_LOG_INFO(g_logger) << "thread test end";
    LYSLG_LOG_INFO(g_logger) << "count=" << count;

    return 0;
}