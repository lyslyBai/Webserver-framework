#include "iomanager.h"
#include "macro.h"
#include "log.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

namespace lyslg{

static lyslg::Logger::ptr g_logger = LYSLG_LOG_NAME("system"); 

IoManager::FdContext::EventContext& IoManager::FdContext::getContext(IoManager::Event event){
    switch(event) {
        case IoManager::READ:
            return read;
        case IoManager::WRITE:
            return write;
        default:
            LYSLG_ASSERT2(false,"getContext");
    }
}

void IoManager::FdContext::resetContext(FdContext::EventContext& ctx){
    ctx.scheduler = nullptr;
    ctx.fiber.reset();
    ctx.cb = nullptr;
}

void IoManager::FdContext::triggerEvent(IoManager::Event event){
    LYSLG_ASSERT(event & events);
    events = (Event)(events  & ~event);
    EventContext& ctx = getContext(event);
    if(ctx.cb) {
        ctx.scheduler->schedule(&ctx.cb);
    } else {
        ctx.scheduler->schedule(&ctx.fiber);
    }
    ctx.scheduler = nullptr;
    return ;
}



IoManager::IoManager(size_t threads, bool use_caller, const std::string& name)
    :Scheduler(threads,use_caller,name){

    m_epfd = epoll_create(500);
    LYSLG_ASSERT(m_epfd > 0);
/*On success, zero is returned.  On error, -1 is returned, and errno is set appropriately.*/
    int rt = pipe(m_tickleFds);
    LYSLG_ASSERT(rt == 0);

    epoll_event event;
    memset(&event,0,sizeof(epoll_event));
    event.events = EPOLLIN|EPOLLET;
    event.data.fd = m_tickleFds[0];

    // 获取原来的flag，加上非阻塞，从新设置,-1为error，0成功
    int flags = fcntl(m_tickleFds[0], F_GETFL, 0);
    flags |= O_NONBLOCK;  // 设置为非阻塞
    rt = fcntl(m_tickleFds[0], F_SETFL, flags);
    LYSLG_ASSERT(rt == 0);

    // 成功返回0
    rt = epoll_ctl(m_epfd,EPOLL_CTL_ADD,m_tickleFds[0],&event);
    LYSLG_ASSERT(rt == 0);

    contextResize(32);
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

void IoManager::contextResize(size_t size){
    m_fdContexts.resize(size);

    for(size_t i = 0;i<m_fdContexts.size();++i) {
        if(!m_fdContexts[i]) {
            m_fdContexts[i] = new FdContext;
            m_fdContexts[i]->fd = i;
        }
    }

}

// 1 success, 0 retry , -1 error
int IoManager::addEvent(int fd, Event event, std::function<void()> cb){
    FdContext* fd_ctx = nullptr;
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() > fd) {
        fd_ctx = m_fdContexts[fd];
        lock.unlock();
    } else{
        lock.unlock();
        RWMutexType::WriteLock lock2(m_mutex);
        contextResize(fd * 1.5);
        fd_ctx = m_fdContexts[fd];
    }

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(fd_ctx->events & event) {
        LYSLG_LOG_ERROR(g_logger) << "addEvent assert fd" << fd
                        << " event=" << event   
                        << " fd_ctx.event=" << fd_ctx->events;
        LYSLG_ASSERT(!(fd_ctx->events & event));
    }

    int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    epoll_event epevent;
    epevent.events = EPOLLET | fd_ctx->events | event;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op ,fd, &epevent);
    if(rt) {
        LYSLG_LOG_ERROR(g_logger) << "epoll_ctl" << m_epfd << ", "
            << op << ","<< fd << "," << epevent.events << "):"
            << rt << " ("<< errno << ") (" << strerror(errno) << ")";
        return -1; 
    }

    ++m_pendingEventCount;
    fd_ctx->events = (Event)(fd_ctx->events | event);
    FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    LYSLG_ASSERT(!event_ctx.scheduler
                && !event_ctx.fiber
                && !event_ctx.cb);
    
    event_ctx.scheduler = Scheduler::GetThis();
    if(cb) {
        event_ctx.cb.swap(cb);
    }else {
        // 当前线程的协程，处于执行状态，这一点怎么理解？
        event_ctx.fiber = Fiber::GetThis();
        LYSLG_ASSERT(event_ctx.fiber->getState() == Fiber::EXEC);
    }
    return 0;
}

bool IoManager::delEvent(int fd, Event event){
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() <= fd) {
        return false;
    }

    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(!(fd_ctx->events & event)) {
        return false;
    }

    Event new_events = (Event)(fd_ctx->events & ~event);
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | new_events;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd,op,fd,&epevent);
    if(rt) {
        LYSLG_LOG_ERROR(g_logger) << "epoll_ctl" << m_epfd << ", "
            << op << ","<< fd << "," << epevent.events << "):"
            << rt << " ("<< errno << ") (" << strerror(errno) << ")";
        return false; 
    }

    --m_pendingEventCount;
    fd_ctx->events = new_events;
    FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    fd_ctx->resetContext(event_ctx);
    return true;
}

bool IoManager::cancelEvent(int fd, Event event){
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() <= fd) {
        return false;
    }

    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(!(fd_ctx->events & event)) {
        return false;
    }

    Event new_events = (Event)(fd_ctx->events & ~event);
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | new_events;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd,op,fd,&epevent);
    if(rt) {
        LYSLG_LOG_ERROR(g_logger) << "epoll_ctl" << m_epfd << ", "
            << op << ","<< fd << "," << epevent.events << "):"
            << rt << " ("<< errno << ") (" << strerror(errno) << ")";
        return false; 
    }
    // 这里为什么要trigger
    fd_ctx->triggerEvent(event);
    --m_pendingEventCount;
    return true;
}

bool IoManager::cancelAll(int fd){
    IoManager::RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() <= fd) {
        return false;
    }

    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(!fd_ctx->events) {
        return false;
    }

    int op = EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = 0;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd,op,fd,&epevent);
    if(rt) {
        LYSLG_LOG_ERROR(g_logger) << "epoll_ctl" << m_epfd << ", "
            << op << ","<< fd << "," << epevent.events << "):"
            << rt << " ("<< errno << ") (" << strerror(errno) << ")";
        return false; 
    }

    if(fd_ctx->events & READ){
        fd_ctx->triggerEvent(READ);
        --m_pendingEventCount;
    }
    if(fd_ctx->events & WRITE){
        fd_ctx->triggerEvent(WRITE);
        --m_pendingEventCount;
    }
    LYSLG_ASSERT(fd_ctx->events == 0);
    return true;
}
IoManager* IoManager::GetThis(){
    return dynamic_cast<IoManager*>(Scheduler::GetThis());
}

void IoManager::tickle(){
    if(!hasIdleThreads()) {
        return;
    }
    // 此处想管道中写入数据
    int rt = write(m_tickleFds[1],"T",1);
    LYSLG_ASSERT(rt == 1);
}

bool IoManager::stopping(uint64_t& timeout) {
    timeout = getNextTimer();
    return timeout == ~0ull
        && m_pendingEventCount == 0
        && Scheduler::stopping();
}

bool IoManager::stopping(){
    uint64_t timeout = 0;
    return stopping(timeout);
}
void IoManager::idle(){
    /*这段代码使用了Linux中的epoll系统调用，动态分配了一个包含64个epoll_event结构的数组，
    并使用std::shared_ptr来管理这些事件的内存，提供了一个自定义的删除器，
    确保在不再需要shared_ptr时正确释放数组。*/

    //这行代码使用new运算符动态分配了一个包含64个epoll_event结构的数组，
    // 并用()进行值初始化，将数组中的每个元素初始化为零。
    epoll_event* events = new epoll_event[64]();
    std::shared_ptr<epoll_event> shared_events(events,[](epoll_event*ptr){
        delete[] ptr;
    });

    while(true) {
        uint64_t next_timeout = 0;
        if(stopping(next_timeout)) {
            LYSLG_LOG_INFO(g_logger) << "name="<<getName() << " idle stopping exit";
            break;
        }

        int rt = 0;
        do{ // 毫秒
            // 这里使用下个定时器的执行时间间隔与MAX_TIMEOUT中的小值作为超时时间，保证定时器的按时执行
            static const int MAX_TIMEOUT = 3000;
            if(next_timeout != ~0ull) {
                next_timeout = (int)next_timeout > MAX_TIMEOUT
                              ? MAX_TIMEOUT : next_timeout;
            }else {
                next_timeout = MAX_TIMEOUT;
            }
            // 超时函数将返回0。
            rt = epoll_wait(m_epfd, events,64,(int)next_timeout);

            if(rt<0 && errno == EINTR){
                // 中断，继续执行
            }else{
                break;
            }
        }while(true);

        // tickle() 中pipe触发读事件，epoll_wait返回，于是执行Timer中过期函数，（当处在定时器插入到定时器队列首时执行一次）
        std::vector<std::function<void()> > cbs;
        listExpiredCb(cbs);
        if(!cbs.empty()) {
            schedule(cbs.begin(),cbs.end());
            cbs.clear();
        }

        for(int i = 0;i<rt;++i) {
            epoll_event& event = events[i];
            if(event.data.fd == m_tickleFds[0]) {
                uint8_t dummy;
                while(read(m_tickleFds[0],&dummy,1) == 1);
                continue;
            }

            FdContext* fd_ctx = (FdContext*)event.data.ptr;
            FdContext::MutexType::Lock lock(fd_ctx->mutex);
            /*EPOLLERR (错误条件):

            当文件描述符上发生错误时，例如底层的socket发生错误或连接被重置，
            EPOLLERR 事件被触发。
            这通常表示有一些错误发生在与文件描述符相关联的 I/O 操作上，
            但具体的错误信息可能需要通过检查 errno 变量来获取。
            EPOLLHUP (挂起条件):
            当与文件描述符关联的对端关闭连接时，
            或者发生了一些其他导致连接挂起或被断开的情况时，EPOLLHUP 事件被触发。
            这通常用于检测对端关闭连接的情况。*/
            if(event.events & (EPOLLERR | EPOLLHUP)) {
                event.events |= EPOLLIN | EPOLLOUT;
            }
            
            int real_events = NONE;
            if (event.events & EPOLLIN) {
                real_events |= READ;
            }
            if(event.events & EPOLLOUT) {
                real_events |= WRITE;
            }

            if((fd_ctx->events & real_events) == NONE) {
                continue;
            }

            // 执行过后剩余的事件
            int left_events = (fd_ctx->events & ~real_events);
            int op = left_events? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            event.events = EPOLLET | left_events;

            int rt2 = epoll_ctl(m_epfd,op,fd_ctx->fd,&event);
            if(rt2) {
                LYSLG_LOG_ERROR(g_logger) << "epoll_ctl" << m_epfd << ", "
                    << op << ","<< fd_ctx->fd << "," << event.events << "):"
                    << rt2 << " ("<< errno << ") (" << strerror(errno) << ")";
                continue; 
            }

            if(real_events & READ) {
                fd_ctx->triggerEvent(READ);
                --m_pendingEventCount;
            }

            if(real_events & WRITE) {
                fd_ctx->triggerEvent(WRITE);
                --m_pendingEventCount;
            }
        }

        Fiber::ptr cur = Fiber::GetThis();
        auto raw_ptr = cur.get();
        cur.reset();

        raw_ptr->swapOut();

    }
}

void IoManager::onTimerInsertedAtFront() {
    tickle();
}


}