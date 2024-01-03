#ifndef __LYSLG_FIBER_H__
#define __LYSLG_FIBER_H__

#include <ucontext.h>
#include <memory>
#include "mutex.h"

namespace lyslg{

class Fiber : public std::enable_shared_from_this<Fiber> {
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
    Fiber(std::function<void()> cb,size_t stacksize = 0);
    ~Fiber();
    // 重置协程函数，并重置状态  INIT, TERM
    void reset(std::function<void()> cb);
    // 切换到当前协程执行
    void swapIn();
    // 切换到后台执行
    void swapOut();

    uint64_t getId() const {return m_id;}

public:
    // 设置当前协程
    static void SetThis(Fiber* f); 
    // 放回到当前协程
    static Fiber::ptr GetThis();
    // 协程切换到后台，并且设置为Ready状态
    static void YieldToTeady();
    // 协程切换到后台，并且设置为Hold状态
    static void YiedldToHold();
    // 总协程数
    static uint64_t TotalFibers();

    static void MainFunc();

    static uint64_t GetFiberId();

private:
    uint64_t m_id = 0;
    uint32_t m_stacksize = 0;
    State m_state = INIT;
    ucontext_t m_ctx;
    void* m_stack = nullptr;

    std::function<void()> m_cb;
};



}




#endif