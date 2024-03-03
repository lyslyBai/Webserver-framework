#include "lyslg.h"
#include "hook.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

lyslg::Logger::ptr g_logger = LYSLG_LOG_ROOT();

void test_sleep() {
    lyslg::IoManager iom;
    iom.schedule([](){
        sleep(2);
        LYSLG_LOG_INFO(g_logger) << "sleep 2";
    });

    iom.schedule([](){
        sleep(3);
        LYSLG_LOG_INFO(g_logger) << "sleep 3";
    });
    LYSLG_LOG_INFO(g_logger) << "test sleep";
}

void test_sock() {
    int sock = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80); // 
    inet_pton(AF_INET,"39.156.66.10",&addr.sin_addr.s_addr);
    
    LYSLG_LOG_INFO(g_logger) << "begin connect";
    int rt = connect(sock,(const sockaddr*)&addr,sizeof(addr));
    LYSLG_LOG_INFO(g_logger) << "connect re=" << rt <<" errno=" << errno << " errstr=" << strerror(errno);

    if(rt) {
        return;
    }

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sock,data,sizeof(data),0);
    LYSLG_LOG_INFO(g_logger) << "send rt=" << rt << " errno=" << errno;
    
    if(rt<=0) {
        return;
    }

    std::string buff;
    buff.resize(4096);

    rt = recv(sock,&buff[0],buff.size(),0);
    LYSLG_LOG_INFO(g_logger) << "recv rt=" << rt << " errno=" << errno;

    if(rt <= 0) {
        return;
    }

    buff.resize(rt);
    LYSLG_LOG_INFO(g_logger) << buff;

}


int main(int argc,char** argv) {
    // test_sleep();
    // test_sock();
    lyslg::IoManager iom;
    iom.schedule(test_sock);
    return 0;
}