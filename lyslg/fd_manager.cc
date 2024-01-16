#include "fd_manager.h"
#include "hook.h"
#include <sys/stat.h>
#include <iostream>

namespace lyslg{

FdCtx::FdCtx(int fd)
    :m_isInit(false)
    ,m_isSocket(false)
    ,m_sysNonblock(false)
    ,m_userNonblock(false)
    ,m_isClosed(false)
    ,m_fd(fd)
    ,m_recvTimeout(-1)
    ,m_sendTimeout(-1){
    init();
}
FdCtx::~FdCtx(){

}

bool FdCtx::init(){
    if(m_isInit) {
        return false;
    }
    m_recvTimeout = -1;
    m_sendTimeout = -1;

    struct stat fd_stat;
    if(-1 == fstat(m_fd,&fd_stat)) {
        m_isInit = false;
        m_isSocket = false;
    } else {
        m_isInit = true;
        m_isSocket = S_ISSOCK(fd_stat.st_mode);
    }

    if(m_isSocket) {
        int flags = fcntl(m_fd,F_GETFL,0);
        if(!(flags & O_NONBLOCK)) {
            fcntl_f(m_fd,F_SETFL,flags|O_NONBLOCK);
        }
        m_sysNonblock = true;
    } else {
        m_sysNonblock = false;
    }
    m_userNonblock = false;
    m_isClosed = false;
    return m_isInit;
}


void FdCtx::setTimerout(int type,uint64_t v){
    if(type == SO_RCVTIMEO) {
        m_recvTimeout = v;
    }else {
        m_sendTimeout = v;
    }
}

uint64_t FdCtx::getTimerout(int type){
    /// SO_RCVTIMEO 为接收数据的超时时间
    if(type == SO_RCVTIMEO) {
        return m_recvTimeout;
    }else{
        return m_sendTimeout;
    }
}

FdManager::FdManager(){
    m_datas.resize(64);
}

// 默认不创建fd，auto_create为true是创建
FdCtx::ptr FdManager::get(int  fd, bool auto_create){
    if(fd == -1) {
        return nullptr;
    }
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_datas.size() <= fd) {
        if(auto_create == false) {
            return nullptr;
        } 
    }else {
        if(m_datas[fd] || !auto_create) {
            // std::cout << "fd=" << fd << " m_datas.size()=" 
            //           << (int)m_datas.size() // << "isClose" // << m_datas[fd]->isClose()  m_datas[fd]可能是空智能指针
            //           << std::endl;
            return m_datas[fd];
        }
    }
    lock.unlock();

    RWMutexType::WriteLock lock2(m_mutex);
    FdCtx::ptr ctx(new FdCtx(fd));
    m_datas[fd]=ctx;
    return ctx;
}

void FdManager::del(int fd){
    RWMutexType::WriteLock lock(m_mutex);
    if((int)m_datas.size() <= fd) {
        return;
    }
    m_datas[fd].reset();
}

}

