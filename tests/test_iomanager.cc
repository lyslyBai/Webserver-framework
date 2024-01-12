#include "lyslg.h"
#include "iomanager.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>

lyslg::Logger::ptr g_logger = LYSLG_LOG_ROOT();

int sock = 0;

void test_fiber() {
    LYSLG_LOG_INFO(g_logger) << "test fiber sock=" << sock;


    sock = socket(AF_INET,SOCK_STREAM,0);
    fcntl(sock,F_SETFL,O_NONBLOCK);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET,"39.156.66.10",&addr.sin_addr.s_addr);

    if(!connect(sock,(const sockaddr*)&addr,sizeof(addr))){

    }else if(errno == EINPROGRESS) {
        LYSLG_LOG_INFO(g_logger) << "add event errno=" << errno << strerror(errno);

        lyslg::IoManager::GetThis()->addEvent(sock,lyslg::IoManager::WRITE,[](){
            LYSLG_LOG_INFO(g_logger) << "connected";
        });
    }else {
        LYSLG_LOG_INFO(g_logger) << "else errno=" << errno << strerror(errno);
    }

}

void test1() {
    lyslg::IoManager iom;
    iom.schedule(&test_fiber);
}

lyslg::Timer::ptr s_timer;
void test_timer() {
    lyslg::IoManager iom(2);
    s_timer = iom.addTimer(1000,[](){
        static int i = 0;
        LYSLG_LOG_INFO(g_logger) << "hello timer i=" << i;
        if(++i == 3) {
            s_timer->reset(2000,true);
            // s_timer->cancel();
        }
    },true);

}



int main(int argc, char** argv) {
    // test1();
    test_timer();
    return 0;
}