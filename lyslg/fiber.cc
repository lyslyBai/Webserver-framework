#include "fiber.h"
#include "config.h"
#include "macro.h"
#include <atomic>
#include "log.h"
#include "scheduler.h"

namespace lyslg{

static Logger::ptr g_logger = LYSLG_LOG_NAME("system");

static std::atomic<uint64_t> s_fiber_id {0};
static std::atomic<uint64_t> s_fiber_count {0};

static thread_local Fiber* t_fiber = nullptr; // 记录当前执行的协程
static thread_local Fiber::ptr t_threadFiber = nullptr; // 为主协程的智能指针

static ConfigVar<uint32_t>::ptr g_fiber_stack_size = 
    Config::Lookup<uint32_t>("fiber.stack_size",1024*1024,"fiber stack size");


class MallocStackAllocator {
public:
    static void* Alloc(size_t size) {
        return malloc(size);
    }

    static void Dealloc(void* vp, size_t size) {
        return free(vp);
    }
};

using StackAllocator = MallocStackAllocator;

uint64_t Fiber::GetFiberId() {
    if(t_fiber) {
        return t_fiber->getId();
    }
    return 0;
}

Fiber::Fiber() {
    m_state = EXEC;
    SetThis(this);

    if(getcontext(&m_ctx)) {
        LYSLG_ASSERT2(false,"getcontext");
    }

    ++s_fiber_count;
    LYSLG_LOG_DEBUG(g_logger) << "Fiber::Fiber";
}

Fiber::Fiber(std::function<void()> cb,size_t stacksize,bool use_caller)
    :m_id(++s_fiber_id)
    ,m_cb(cb)
    ,m_use_caller(use_caller){
    ++s_fiber_count;
    m_stacksize = stacksize ? stacksize :g_fiber_stack_size->getValue();
    m_stack = StackAllocator::Alloc(m_stacksize);

    if(getcontext(&m_ctx)) {
        LYSLG_ASSERT2(false,"getcontext");
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    // m_use_caller 其实决定了保存点
    // m_use_caller == true ,即是使用当前线程的主协程作为保存点
    // m_use_caller == false, 即是当使用调度函数而没用启用主线程时，即是使用主线程中的m_ctx作为保存点

    if(!m_use_caller) {
        makecontext(&m_ctx, &Fiber::MainFunc, 0);
    } else {
        makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
    }
    
    LYSLG_LOG_DEBUG(g_logger) << "Fiber::Fiber id=" << m_id;
}
/*有栈空间的情况：

如果协程有栈空间，表明它是通过 malloc 或者其他方式分配的，需要手动释放。
通过 LYSLG_ASSERT(m_state == TERM || m_state == EXCEPT || m_state == INIT)
 来确保协程的状态处于终止（TERM）、
异常（EXCEPT）或者初始化（INIT）状态。这些状态下，协程不再执行，可以安全地释放资源。
无栈空间的情况：

如果协程没有栈空间，表明它是主协程（MainFunc）或者没有初始化成功。
(下面这一段有点不明白)
通过 LYSLG_ASSERT(!m_cb) 判断协程是否有回调函数。如果没有回调函数，
说明协程没有成功初始化，无需释放资源。
通过 LYSLG_ASSERT(m_state == EXEC) 判断协程状态是否为执行中。
只有执行中的协程才可能没有栈空间。在这种情况下，需要将当前协程置为 nullptr，
表示主协程，因为没有栈空间需要释放。*/
Fiber::~Fiber(){
    --s_fiber_count;
    if(m_stack) {
        LYSLG_ASSERT(m_state == TERM || m_state == EXCEPT || m_state == INIT);

        StackAllocator::Dealloc(m_stack,m_stacksize);
    }else {
        LYSLG_ASSERT(!m_cb); // 有回调函数，无栈资源，报错
        LYSLG_ASSERT(m_state == EXEC); // 上面确定无回调函数，则必为主协程，置空
        
        Fiber* cur = t_fiber;
        if(cur == this) {
            SetThis(nullptr);
        }
    }
    LYSLG_LOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << m_id;
}
// 重置协程函数，并重置状态  INIT, TERM
void Fiber::reset(std::function<void()> cb){
    LYSLG_ASSERT(m_stack);
    LYSLG_ASSERT(m_state == TERM || m_state == EXCEPT
                 || m_state == INIT);
    m_cb = cb;
    if(getcontext(&m_ctx)) {
        LYSLG_ASSERT2(false,"getcontext");
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc,0);
    m_state = INIT;
}

void Fiber::call() {
    SetThis(this);
    m_state = EXEC;

    LYSLG_LOG_DEBUG(g_logger) << "Fiber::call";

    if(swapcontext(&t_threadFiber->m_ctx,&m_ctx)) {
        LYSLG_ASSERT2(false,"swapcontext");
    }
}

void Fiber::back(){
    SetThis(t_threadFiber.get());
    
    LYSLG_LOG_DEBUG(g_logger) << "Fiber::back";

    if(swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
        LYSLG_ASSERT2(false, "swapcontext");
    }
}

// 切换到当前协程执行
void Fiber::swapIn(){
    SetThis(this);
    LYSLG_ASSERT(m_state != EXEC);
    m_state = EXEC;
    if(swapcontext(&Scheduler::GetMainFiber()->m_ctx,&m_ctx)) {
        LYSLG_ASSERT2(false,"swapcontext");
    }
}
// 切换到后台执行
void Fiber::swapOut(){
    SetThis(Scheduler::GetMainFiber());
    if(swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx)) {
        LYSLG_ASSERT2(false, "swapcontext");
    }
}

void Fiber::SetThis(Fiber* f){
    t_fiber = f;
}

// 放回到当前协程。。
// 应该也就初始化是创建主协程，获得主协程的智能指针,主协程没有协程id
Fiber::ptr Fiber::GetThis(){
    if(t_fiber) {
        return t_fiber->shared_from_this();
    }
    Fiber::ptr main_fiber(new Fiber);
    LYSLG_ASSERT(t_fiber == main_fiber.get());
    t_threadFiber = main_fiber;
    return t_fiber->shared_from_this();
}
// 协程切换到后台，并且设置为Ready状态
void Fiber::YieldToTeady(){
    Fiber::ptr cur = GetThis();
    cur->m_state = READY;
    if(!cur->m_use_caller)  {
        cur->swapOut();
    }else{
        cur->back();
    }
}
// 协程切换到后台，并且设置为Hold状态
void Fiber::YiedldToHold(){
    Fiber::ptr cur = GetThis();
    cur->m_state = HOLD;
    if(!cur->m_use_caller)  {
        cur->swapOut();
    }else{
        cur->back();
    }
}
// 总协程数
uint64_t Fiber::TotalFibers(){
    return s_fiber_count;
}

void Fiber::MainFunc(){
    Fiber::ptr cur = GetThis();
    LYSLG_ASSERT(cur);
    try{
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception& ex) {
        cur->m_state = EXCEPT;
        LYSLG_LOG_ERROR(g_logger) << "Fiber Except:" << ex.what()
        << " fiber_id=" << cur->getId()
        << std::endl
        << lyslg::BacktraceToString();
    }catch(...) {
        cur->m_state = EXCEPT;
        LYSLG_LOG_ERROR(g_logger) << "Fiber Except:"
        << " fiber_id=" << cur->getId()
        << std::endl
        << lyslg::BacktraceToString();
    }
    // 这里有些人说使用野指针，可以思考一下（我当前也不确定）
    // 这里有点问题，可以考虑修改一下
    auto raw_ptr = cur.get();
    // 这里已经调用了析构函数
    cur.reset();
    // 因此这里使用了悬空指针，可能会有点问题
    raw_ptr->swapOut();

    LYSLG_ASSERT2(false,"never reach fiber_id=" + std::to_string(raw_ptr->getId()));
}

void Fiber::CallerMainFunc() {
    Fiber::ptr cur = GetThis();
    LYSLG_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception& ex) {
        cur->m_state = EXCEPT;
        LYSLG_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
            << " fiber_id=" << cur->getId()
            << std::endl
            << lyslg::BacktraceToString();
    } catch (...) {
        cur->m_state = EXCEPT;
        LYSLG_LOG_ERROR(g_logger) << "Fiber Except"
            << " fiber_id=" << cur->getId()
            << std::endl
            << lyslg::BacktraceToString();
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->back();
    LYSLG_ASSERT2(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));

}

}