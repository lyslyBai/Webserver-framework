#include "env.h"
#include <unistd.h>
#include <iostream>
#include <fstream>

struct A {
    A() {
        std::ifstream ifs("/proc/" + std::to_string(getpid()) + "/cmdline", std::ios::binary);
        std::string content;
        content.resize(4096);

        ifs.read(&content[0], content.size());
        content.resize(ifs.gcount());

        for(size_t i = 0; i < content.size(); ++i) {
            std::cout << i << " - " << content[i] << " - " << (int)content[i] << std::endl;
        }
    }
};

A a;

int main(int argc, char** argv) {
    std::cout << "argc=" << argc << std::endl;
    lyslg::EnvMgr::GetInstance()->addHelp("s", "start with the terminal");
    lyslg::EnvMgr::GetInstance()->addHelp("d", "run as daemon");
    lyslg::EnvMgr::GetInstance()->addHelp("p", "print help");

    if(!lyslg::EnvMgr::GetInstance()->init(argc, argv)) {
        lyslg::EnvMgr::GetInstance()->printHelp();
        return 0;
    }

    std::cout << "exe=" << lyslg::EnvMgr::GetInstance()->getExe() << std::endl;
    std::cout << "cwd=" << lyslg::EnvMgr::GetInstance()->getCwd() << std::endl;

    std::cout << "path=" << lyslg::EnvMgr::GetInstance()->getEnv("PATH", "xxx") << std::endl;
    std::cout << "test=" << lyslg::EnvMgr::GetInstance()->getEnv("TEST", "") << std::endl;
    std::cout << "set env " << lyslg::EnvMgr::GetInstance()->setEnv("TEST", "yy") << std::endl;
    std::cout << "test=" << lyslg::EnvMgr::GetInstance()->getEnv("TEST", "") << std::endl;
    if(lyslg::EnvMgr::GetInstance()->has("p")) {
        lyslg::EnvMgr::GetInstance()->printHelp();
    }
    
    return 0;
}
