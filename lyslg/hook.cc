#include "hook.h"
#include "fiber.h"
#include "iomanager.h"
#include <dlfcn.h>
#include "log.h"
#include "fd_manager.h"
#include "config.h"

static lyslg::Logger::ptr g_logger = LYSLG_LOG_NAME("system");

namespace lyslg{

static lyslg::ConfigVar<int>::ptr g_tcp_connect_timeout = 
    lyslg::Config::Lookup("tcp.connect.timeout",5000,"tcp connect timeout");


static thread_local bool t_hook_enable = false;

#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep) \
    XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg) \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg) \
    XX(close) \
    XX(fcntl) \
    XX(ioctl) \
    XX(getsockopt) \
    XX(setsockopt) 

void hook_init() {
    static bool is_inited = false;
    if(is_inited) {
        return;
    }

/*动态库装载是由一些列动态库提供的API来完成的，
准确来说就是打开动态库(dlopen)、查找符号(dlsym)、
关闭动态库(dlcose)、错误处理(dlerror)四个函数。
我们可能会链接多个动态库，不同的动态库可能都会有symbol这个函数名，
那么使用RTLD_NEXT参数后dlsym返回的就是第一个遇到(匹配上)symbol这个符号的函数的函数地址。*/
/*
当宏XX(name)被展开时，它将创建一个名为name_f的函数指针变量，
该变量的类型为name_fun，并使用dlsym从动态符号表中获取与name
相对应的函数地址。(name ## _fun) 类型转换确保使用正确的函数指针类型。

这样的代码结构常常用于创建系统调用的函数指针变量，并动态地对它们进行挂钩。
在这里，HOOK_FUN(XX);的目的是为一系列系统调用生成对应的函数指针变量。*/
// 总结这个宏定义的意义是将name_f的地址定位到动态库中的系统调用的函数地址
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX);
#undef XX
 
}
static uint64_t s_connect_timeout = -1;
struct _HookIniter{
    _HookIniter() {
        hook_init();
        s_connect_timeout = g_tcp_connect_timeout->getValue();

         g_tcp_connect_timeout->addListener([](const int& old_value,const int& new_value){
            LYSLG_LOG_INFO(g_logger) << "tcp connect time changed from"
                                     << old_value << "to" << new_value;
            s_connect_timeout = new_value;
        });
    }
};

static _HookIniter s_hook_initer;

bool is_hook_enable() {
    return t_hook_enable;
}

void set_hook_enable(bool flag) {
    t_hook_enable = flag;
}

struct timer_info{
    int cancelled = 0; 
};
/*ssize_t 是函数的返回类型，表示有符号的整数型。*/

/*typename ... Args 模板中的可变参数模板（Variadic Templates）语法*/
template<typename OriginFun, typename ... Args>
static ssize_t do_io(int fd, OriginFun fun,const char* hook_fun_name,
                    uint32_t event, int timeout_so, Args&&... args) {
/*std::forward<Args>(args)... 是 C++ 中的完美转发（Perfect Forwarding）的语法。让我们逐步解释这个表达式：

std::forward 是 C++ 标准库中的一个模板函数，它用于进行完美转发。

<Args> 是模板参数，表示 std::forward 的模板参数。这里的 Args 实际上是 do_io 函数模板的模板参数包。

(args) 是函数 do_io 的参数包。这是 do_io 函数接收的可变数量和类型的参数。

... 是 C++11 中引入的展开操作符。在这个上下文中，它用于展开参数包中的所有参数。

因此，std::forward<Args>(args)... 的含义是对 args 中的参数进行完美转发。
这样写的目的是确保传递给 fun 的参数保持它们原始的值类别（左值或右值）以及 const 修饰符。
这对于模板编程中的通用性非常重要，因为它允许传递的参数在传递到原始函数时保持它们的属性。*/
    if(!lyslg::t_hook_enable) {
        return fun(fd,std::forward<Args>(args)...);
    }

    LYSLG_LOG_INFO(g_logger) << "hook_fun_name:" << hook_fun_name;


    lyslg::FdCtx::ptr ctx = lyslg::FdMgr::GetInstance()->get(fd);

    
    if(!ctx) {
        return fun(fd,std::forward<Args>(args)...);
    }
    if(ctx->isClose()) {
        /*错误码表示文件描述符无效*/
        errno = EBADF;
        return -1;
    }


    if(!ctx->isSocket() || ctx->getUserNonblock()) {
        return fun(fd,std::forward<Args>(args)...);
    }


    uint64_t to = ctx->getTimerout(timeout_so);
    std::shared_ptr<timer_info> tinfo(new timer_info);
    
retry:
    ssize_t n = fun(fd, std::forward<Args>(args)...);
    /*函数尝试调用 fun 进行IO操作。如果操作返回 -1 并且 
    errno 是 EINTR（中断错误），则尝试再次调用原始函数。*/
    while(n == -1 && errno == EINTR) {
        n = fun(fd, std::forward<Args>(args)...);
    }
    /*EAGAIN 表示非阻塞操作没有立即可用数据或者没有立即可写入的空间*/
    if(n == -1 && errno == EAGAIN) {
        /*如果设置了超时时间 to，则创建一个条件定时器，用于取消事件和定时器。*/
        lyslg::IoManager* iom = lyslg::IoManager::GetThis();
        lyslg::Timer::ptr timer;
        std::weak_ptr<timer_info> winfo(tinfo);

        if(to != (uint64_t)-1) {
            timer = iom->addCondictionTimer(to,[winfo,fd,iom,event]() {
                auto t = winfo.lock();
                if(!t || t->cancelled) {
                    return;
                }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd,(lyslg::IoManager::Event)(event));
            },winfo);
        }

        int rt = iom->addEvent(fd,(lyslg::IoManager::Event)(event));
        if(rt) {
            LYSLG_LOG_ERROR(g_logger) << hook_fun_name << "addEvent ("
                                    << fd << ", " << event << ")";
            if(timer){
                timer->cancel();
            }
            return -1;
        }else{
            lyslg::Fiber::YieldToHold();
           /**处理定时器取消：**在切换回来后，如果定时器被取消，则设置 errno 并返回 -1*/

            if(timer){
                timer->cancel();
            }
            if(tinfo->cancelled) {
                errno = tinfo->cancelled;
                return -1;
            }

            goto retry;
        }
    }
    return n;
}


}

extern "C" {
    // 这里 ## 是连接运算符，用于将两个标识符连接成一个标识符。
    //  name_fun 是类型的名称，而 name_f 是变量的名称，那么它们在不同的命名空间中，符号没有冲突。
    // 在生成的代码中，someSysCall_fun 通常是一个函数指针类型，而 someSysCall_f 是一个指向该类型的变量。
// 总结：这个宏定义的意义是初始化name_f的指针
#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);
#undef XX

unsigned int sleep(unsigned int seconds){
    if(!lyslg::t_hook_enable) {
        return sleep_f(seconds);
    }

    lyslg::Fiber::ptr fiber = lyslg::Fiber::GetThis();
    lyslg::IoManager* iom = lyslg::IoManager::GetThis();
    iom->addTimer(seconds * 1000,std::bind((void(lyslg::Scheduler::*)
            (lyslg::Fiber::ptr, int thread))&lyslg::IoManager::schedule,
            iom,fiber,-1));
    lyslg::Fiber::YieldToHold();
    return 0;
}

int usleep(useconds_t usec){
    if(!lyslg::t_hook_enable) {
        return sleep_f(usec);
    }

    lyslg::Fiber::ptr fiber = lyslg::Fiber::GetThis();
    lyslg::IoManager* iom = lyslg::IoManager::GetThis();
    iom->addTimer(usec / 1000,std::bind((void(lyslg::Scheduler::*)
            (lyslg::Fiber::ptr, int thread))&lyslg::IoManager::schedule,
            iom,fiber,-1));
    lyslg::Fiber::YieldToHold();
    return 0;
}

int socket(int domain, int type, int protocol){
    if(!lyslg::t_hook_enable) {
        return socket_f(domain,type,protocol);
    }

    int fd = socket_f(domain,type,protocol);
    if(fd == -1) {
        return fd;
    }

    lyslg::FdMgr::GetInstance()->get(fd,true);
    return fd;
} 

int connect_with_timeout(int sockfd, const struct sockaddr *addr, socklen_t addrlen,uint64_t timeout_ms) {
    if(!lyslg::t_hook_enable) {
        return connect_f(sockfd,addr, addrlen);
    }

    lyslg::FdCtx::ptr ctx = lyslg::FdMgr::GetInstance()->get(sockfd);
    if(!ctx || ctx->isClose()) {
        errno = EBADF;
        return -1;
    }

    if(!ctx->isSocket() || ctx->getUserNonblock()) {
        return connect_f(sockfd,addr, addrlen);
    }

    int n = connect_f(sockfd,addr, addrlen );
    if(n == 0){
        return 0;
        /*EINPROGRESS 这表示连接操作已经启动，但尚未完成。*/
    } else if(n != -1 || errno != EINPROGRESS) {
        return n;
    }

    lyslg::IoManager* iom = lyslg::IoManager::GetThis();
    lyslg::Timer::ptr timer;
    std::shared_ptr<lyslg::timer_info> tinfo(new lyslg::timer_info);
    std::weak_ptr<lyslg::timer_info> winfo(tinfo);

    if(timeout_ms != (uint64_t)-1) {
        timer = iom->addCondictionTimer(timeout_ms,[winfo,sockfd,iom]() {
            auto t = winfo.lock();
            if(!t || t->cancelled) {
                return;
            }
            t->cancelled = ETIMEDOUT;
            iom->cancelEvent(sockfd,lyslg::IoManager::WRITE);

        },winfo);
    }

    int rt = iom->addEvent(sockfd,lyslg::IoManager::WRITE);
    if(rt) {
        if(timer){
            timer->cancel();
        }
        LYSLG_LOG_ERROR(g_logger) << "connect addEvent(" <<sockfd << ", WRITE) error";
    }else{
        lyslg::Fiber::YieldToHold();
        /**处理定时器取消：**在切换回来后，如果定时器被取消，则设置 errno 并返回 -1*/
        if(timer){
            timer->cancel();
        }
        if(tinfo->cancelled) {
            errno = tinfo->cancelled;
            return -1;
        }
    }

    int error = 1;
    socklen_t len = sizeof(int);
    if(-1 == getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&error,&len)) {
        return -1;
    }
    if(!error) {
        return 0;
    } else {
        errno = error;
        return -1;
    }
}




int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
    return connect_with_timeout(sockfd,addr, addrlen,lyslg::s_connect_timeout);
}

int accept(int s, struct sockaddr *addr, socklen_t *addrlen){
    int fd = do_io(s,accept_f,"accept",lyslg::IoManager::READ,SO_RCVTIMEO,addr,addrlen);
    if(fd >= 0) {
        lyslg::FdMgr::GetInstance()->get(fd,true);
    }
    return fd;
}

ssize_t read(int fd, void *buf, size_t count){
   return do_io(fd,read_f,"read",lyslg::IoManager::READ,SO_RCVTIMEO,buf,count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt){
   return do_io(fd,readv_f,"readv",lyslg::IoManager::READ,SO_RCVTIMEO,iov,iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flag){
   return do_io(sockfd,recv_f,"recv",lyslg::IoManager::READ,SO_RCVTIMEO,buf,len,flag);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len,  int flags, struct sockaddr *src_addr, socklen_t *addrlen){
   return do_io(sockfd,recvfrom_f,"recvfrom",lyslg::IoManager::READ,SO_RCVTIMEO,buf,len,flags,src_addr,addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flag){
   return do_io(sockfd,recvmsg_f,"recvmsg",lyslg::IoManager::READ,SO_RCVTIMEO,msg,flag);
}

ssize_t write(int fd, const void *buf, size_t count){
    return do_io(fd,write_f,"write",lyslg::IoManager::WRITE,SO_SNDTIMEO,buf,count);
}
ssize_t writev(int fd, const struct iovec *iov, int iovcnt){
    return do_io(fd,writev_f,"writev",lyslg::IoManager::WRITE,SO_SNDTIMEO,iov,iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags){
    return do_io(s,send_f,"send",lyslg::IoManager::WRITE,SO_SNDTIMEO,msg,len,flags);
}


ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen){
    return do_io(s,sendto_f,"sendto",lyslg::IoManager::WRITE,SO_SNDTIMEO,msg,len,flags,to,tolen);
}


ssize_t sendmsg(int s, const struct msghdr *msg, int flags){
    return do_io(s,sendmsg_f,"sendmsg",lyslg::IoManager::WRITE,SO_SNDTIMEO,msg,flags);
}

int close(int fd){
    if(!lyslg::t_hook_enable) {
        return close_f(fd);
    }

    lyslg::FdCtx::ptr ctx = lyslg::FdMgr::GetInstance()->get(fd);
    if(ctx) {
        auto iom = lyslg::IoManager::GetThis();
        if(iom) {
            iom->cancelAll(fd);
        }
        lyslg::FdMgr::GetInstance()->del(fd);
    }
    return close_f(fd);
}

int fcntl(int fd, int cmd, ... /* arg */ ){
    va_list va;
    va_start(va,cmd);
    switch(cmd) {
        case F_SETFL:
            {
                int arg = va_arg(va,int);
                va_end(va);
                lyslg::FdCtx::ptr ctx = lyslg::FdMgr::GetInstance()->get(fd);
                if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                    return fcntl_f(fd, cmd, arg);
                }
                ctx->setUserNonblock(arg & O_NONBLOCK);
                if(ctx->getSysNonblock()) {
                    arg |= O_NONBLOCK;
                } else {
                    arg &= ~O_NONBLOCK;
                }
                return fcntl_f(fd,cmd,arg);
            }
            break;
        case F_GETFL:
            {
                va_end(va);
                int arg = fcntl_f(fd,cmd);
                lyslg::FdCtx::ptr ctx = lyslg::FdMgr::GetInstance()->get(fd);
                if(!ctx || ctx->isClose() || !ctx->isSocket()){
                    return arg;
                }
                if(ctx->getUserNonblock()){
                    return arg | O_NONBLOCK;
                } else {
                    return arg & ~O_NONBLOCK;
                }
            }
            break;
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif  
            {
                int arg = va_arg(va,int);
                va_end(va);
                return fcntl_f(fd,cmd,arg);
            }
            break;
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif  
            {
                va_end(va);
                return fcntl_f(fd,cmd);
            }
            break;
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
            {
                struct flock* arg = va_arg(va,struct flock*);
                va_end(va);
                return fcntl_f(fd,cmd,arg);
            }
            break;
        case F_GETOWN_EX:
        case F_SETOWN_EX:
            {
                struct f_owner_exlock* arg = va_arg(va,struct f_owner_exlock*);
                va_end(va);
                return fcntl_f(fd,cmd,arg);
            }
            break;
        default:
            va_end(va);
            return fcntl_f(fd,cmd);
    }

}

int ioctl(int d, unsigned long int request, ...){
    va_list va;
    va_start(va,request);
    void* arg = va_arg(va,void*);
    va_end(va);

    if(FIONBIO == request) {
        bool user_nonblock = !!*(int*)arg;
        lyslg::FdCtx::ptr ctx = lyslg::FdMgr::GetInstance()->get(d);
        if(!ctx || ctx->isClose() || !ctx->isSocket()){
            return ioctl_f(d,request,arg);
        }
        ctx->setUserNonblock(user_nonblock);
    }
    return ioctl_f(d,request,arg);
}
int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen){
    return getsockopt_f(sockfd, level, optname,optval,optlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen){
    if(!lyslg::t_hook_enable) {
        return setsockopt_f(sockfd,  level,  optname, optval, optlen);
    }

    if(level == SOL_SOCKET) {
        if(optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
            lyslg::FdCtx::ptr ctx = lyslg::FdMgr::GetInstance()->get(sockfd);
            if(ctx) {
                const timeval* v = (const timeval*)optval;
                ctx->setTimerout(optname,v->tv_sec*1000 + v->tv_usec/1000);
            }
        }
    }
    return setsockopt_f(sockfd,  level,  optname, optval, optlen);
}



}