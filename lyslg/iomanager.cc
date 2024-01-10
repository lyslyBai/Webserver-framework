#include "iomanager.h"
#include "macro.h"
#include "log.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

namespace lyslg{

static lyslg::Logger::ptr g_logger = LYSLG_LOG_NAME("system"); 

IoManager::IoManager(size_t threads, bool use_caller, const std::string& name)
    :Scheduler(threads,use_caller,name){
    m_epfd = epoll_create(500);
    LYSLG_ASSERT(m_epfd > 0);
/*On success, zero is returned.  On error, -1 is returned, and errno is set appropriately.*/
    int rt = pipe(m_tickleFds);
    LYSLG_ASSERT(rt == 0);

    epoll_event event;
    memset(&event,0,sizeof(epoll_event));
    event.events == EPOLLIN | EPOLLET;
    event.data.fd = m_tickleFds[0];

    // 获取原来的flag，加上非阻塞，从新设置,-1为error，0成功
    int flags = fcntl(m_tickleFds[0], F_GETFL, 0);
    flags |= O_NONBLOCK;  // 设置为非阻塞
    rt = fcntl(m_tickleFds[0], F_SETFL, flags);
    LYSLG_ASSERT(rt == 0);

    // 成功返回0
    rt = epoll_ctl(m_epfd,EPOLL_CTL_ADD,m_tickleFds[0],&event);
    LYSLG_ASSERT(rt == 0);

    m_fdContexts.resize(64);
    start();
}
IoManager::~IoManager(){
    stop();
    close(m_epfd);
    close(m_tickleFds[0]);
    close(m_tickleFds[1]);

    for(size_t i = 0;i<m_fdContexts.size();++i) {
        if(m_fdContexts[i]) {
            delete m_fdContexts[i];
        }
    }

}

// 1 success, 0 retry , -1 error
int IoManager::addEvent(int fd, Event event, std::function<void()> cb){



}
bool IoManager::delEvent(int fd, Event event){



}
bool IoManager::cancel(int fd, Event event){

}

bool IoManager::cancelAll(int fd){

}
IoManager* IoManager::GetThis(){

}

void IoManager::tickle(){


}
bool IoManager::stopping(){

}
void IoManager::idle(){

}


}