#include "thread.h"
#include "log.h"

namespace lyslg{

static  thread_local Thread* t_thread = nullptr;  // thread_local
static  thread_local std::string t_thread_name = "UNKNOW";

static lyslg::Logger::ptr g_logger = LYSLG_LOG_NAME("system");


Thread* Thread::GetThis(){
    return t_thread;
}
const std::string& Thread::GetName(){
    return t_thread_name;
}
/*
在 Thread::SetName 函数中，判断 if(t_thread) 的目的是确保当前线程对象 t_thread 不为空。
这是因为 SetName 函数是一个静态函数，通过 Thread::SetName 调用，而不是通过线程对象实例调用的。
在这种情况下，无法直接通过实例对象访问 m_name 成员。

这个判断的主要目的是为了避免在没有当前线程对象的情况下，尝试设置线程名称。
如果当前线程对象为空，说明当前线程并没有通过 Thread::run 函数创建，
因此在这种情况下直接返回而不进行任何操作是比较合理的。
否则，在没有有效线程对象的情况下，直接访问 m_name 会导致未定义的行为。
*/
void Thread::SetName(const std::string& name){
    if(name.empty()) {
        return;
    }
    if(t_thread) {
        t_thread->m_name = name;
    }
    t_thread_name = name;
}

Thread::Thread(std::function<void()> cb,const std::string& name)
    :m_cb(cb)
    ,m_name(name){
    if(name.empty()) {
        m_name = "UNKNOW";
    }
    int rt = pthread_create(&m_thread, nullptr, &Thread::run, this); // 非0 失败
    if(rt) {
        LYSLG_LOG_ERROR(g_logger) << "pthread_create thread fail,rt=" << rt
                                  << " name="<< name;
        throw std::logic_error("pthread_create error");
    }
    m_semaphore.wait();
}

Thread::~Thread(){
    if(m_thread) {
        pthread_detach(m_thread);
    }
}
void Thread::join(){
    if(m_thread) {
        int rt = pthread_join(m_thread,nullptr);
        if(rt) {
            LYSLG_LOG_ERROR(g_logger) << "pthread_join thread fail,rt=" << rt
                                      << " name" << m_name;
            throw std::logic_error("pthread_join error");
        }  //  抛出异常会返回吗
        m_thread = 0; 
    }

}

// 这里是静态成员函数，所以this自身需要显示的传入
void* Thread::run(void* arg){
    Thread* thread = (Thread*)arg;
    //静态线程局部变量，用于存储当前线程的指针
    t_thread = thread;
    t_thread_name = thread->m_name;
    thread->m_id = lyslg::GetThreadId();
    // 设置当前线程的名称，这里使用 pthread_setname_np 函数。这个名称将会在调试工具和日志中用于标识线程。
    // pthread_self() 用来回去当前线程id，substr(0,15)为了确保线程名字不超过系统规定的长度
    pthread_setname_np(pthread_self(), thread->m_name.substr(0,15).c_str());
    // 并通过 swap 操作将其与线程对象中的回调函数
    std::function<void()> cb;

    /*资源安全性： 如果 thread->m_cb 在多线程环境下被写入新的值，而同时线程又在执行 thread->m_cb()，就可能导致资源不一致或未定义的行为。

    竞争条件： 如果没有合适的同步机制，多个线程同时访问 thread->m_cb，可能会导致竞争条件，破坏线程安全性。

    通过使用 cb.swap(thread->m_cb); 的方式，可以在确保线程安全的前提下，将回调函数的资源正确地传递给线程执行：

    cb.swap(thread->m_cb); 通过原子的交换操作确保了线程安全性。
    通过交换，保证了在执行 cb(); 之前，thread->m_cb 持有了正确的资源。*/
    cb.swap(thread->m_cb);
    thread->m_semaphore.notify();  // 确保当前的线程跑起来

    cb();
    return 0;
}



}