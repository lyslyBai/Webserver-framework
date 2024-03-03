#ifndef __LYSLG_FIBER_H__
#define __LYSLG_FIBER_H__

#include <ucontext.h>
#include <memory>
#include "mutex.h"

namespace lyslg{

class Fiber : public std::enable_shared_from_this<Fiber> {
friend class Scheduler;
public:
    typedef std::shared_ptr<Fiber> ptr;
    // 状态
    enum State {
        INIT = 0,  // 初始化
        HOLD,      // 暂停
        EXEC,     // 执行中
        TERM,      // 结束
        READY,      // 准备完成 ？
        EXCEPT
    };
private:
    Fiber();

public:
    Fiber(std::function<void()> cb,size_t stacksize = 0,bool use_caller=false);
    ~Fiber();
    // 重置协程函数，并重置状态  INIT, TERM
    void reset(std::function<void()> cb);

    void call();
    void back();
    // 切换到当前协程执行
    void swapIn();
    // 切换到后台执行
    void swapOut();

    uint64_t getId() const {return m_id;}
    
    State getState() const { return m_state;}
public:
    // 设置当前协程
    static void SetThis(Fiber* f); 
    // 放回到当前协程
    static Fiber::ptr GetThis();
    // 协程切换到后台，并且设置为Ready状态
    static void YieldToReady();
    // 协程切换到后台，并且设置为Hold状态
    static void YieldToHold();
    // 总协程数
    static uint64_t TotalFibers();

    static void MainFunc();

    static void CallerMainFunc();
    
    static uint64_t GetFiberId();

private:
    uint64_t m_id = 0;
    uint32_t m_stacksize = 0; 
    State m_state = INIT;
    ucontext_t m_ctx;
    void* m_stack = nullptr;
/*
std::function<void()> 表示一个可以接受无参数并返回 void 的函数对象
（函数指针、函数、Lambda 表达式等）。
它并不仅限于无参的函数，而是表示一个特定的函数签名：
接受无参数，返回 void。*/
    std::function<void()> m_cb;

    bool m_use_caller = false;
};



}




#endif