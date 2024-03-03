#include "scheduler.h"
#include "log.h"
#include "hook.h"



namespace lyslg{

static lyslg::Logger::ptr g_logger = LYSLG_LOG_NAME("system");

static thread_local Scheduler* t_scheduler = nullptr;

// 这个为每个线程执行的协程，不为每个线程的主协程
// 主协程每个线程中都存在的调度协程，相当于中转站，t_fiber为在执行的协程
static thread_local Fiber* t_fiber = nullptr;
// 区分主线程和子线程
// 当主线程同样处理协程调度器中的协程时，由于是使用m_rootFiber执行run函数的，而run函数中执行了其他的协程，
// 所以需要两个中转站，避免覆盖，m_rootFiber使用fiber中的t_threadFiber,而run中的协程使用t_fiber充当中转站。
// 其他的线程直接执行run函数，所以只需要一个中转站，直接使用t_fiber.逻辑上好像也可以使用t_threadFiber。不知道有没有问题，之后在尝试吧！
// use_caller 是否使用当前调用线程   这里很奇怪name的默认值好像没有用
Scheduler::Scheduler(size_t threads ,bool use_caller,const std::string& name)
    :m_name(name){
    LYSLG_ASSERT(threads > 0);

    if(use_caller) {
        lyslg::Fiber::GetThis();
        // 使用当前调度线程，需要创建的线程数就减一
        --threads;

        LYSLG_ASSERT(GetThis() == nullptr);
        t_scheduler = this;

        // 本线程执行的函数
        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run,this),0,true));
        // std::cout << "name :"<< name << std::endl;
        lyslg::Thread::SetName(name); // 默认没有名字才对，但我原来好像不是

        t_fiber = m_rootFiber.get();
        m_rootThread = lyslg::GetThreadId();
        m_threadIds.push_back(m_rootThread);
    }else {
        m_rootThread = -1;
    }
    // 需要创建的线程个数
    m_threadCount = threads;
}
Scheduler::~Scheduler(){
    LYSLG_ASSERT(m_stopping);
    if(GetThis() == this) {
        t_scheduler = nullptr;
    }
}


Scheduler* Scheduler::GetThis(){
    return t_scheduler;
}

Fiber* Scheduler::GetMainFiber(){
    return t_fiber;
}

void Scheduler::start(){
    MutexType::Lock lock(m_mutex);
    if(!m_stopping) {
        return;
    }

    m_stopping = false;
    LYSLG_ASSERT(m_threads.empty());

    m_threads.resize(m_threadCount);
    for(size_t i = 0; i<m_threadCount; ++i) {
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this)
                                , m_name + "_" + std::to_string(i)));
        m_threadIds.push_back(m_threads[i]->getId());
    }
    lock.unlock();

    // if(m_rootFiber ) {
    //     m_rootFiber->call();
    //     LYSLG_LOG_INFO(g_logger) << " call out";
    // }
}   
void Scheduler::stop(){
    // LYSLG_LOG_INFO(g_logger) << " m_rootFiber before";
    // m_rootFiber->swapIn();
    // LYSLG_LOG_INFO(g_logger) << " m_rootFiber after";

    m_autoStop = true;
    if(m_rootFiber  // 考虑主线程，若不使用主线程，这m_rootFiber即为nullptr
            && m_threadCount == 0  // 需要创建的线程，及不需要创建其他线程
            && (m_rootFiber->getState() == Fiber::TERM  // 主线程INIT或结束term 
                || m_rootFiber->getState() == Fiber::INIT)){
        
        m_stopping = true;
        LYSLG_LOG_INFO(g_logger) << this <<" "<< stopping() << " stopped";
        if(stopping()) {
            return; // 满足以上条件可以退出
        }
    }

    // bool exit_on_this_fiber = false;
    // t_scheduler当m_rootThread 使用时赋值
    if(m_rootThread != -1) {  // 使用此线程
        LYSLG_ASSERT(GetThis() == this);
    }else {         // 未使用此线程
        LYSLG_ASSERT(GetThis() != this);
    }

    // 这一点也还是不太明白，tickle();是唤醒线程，那么这里就是唤醒所有线程和主线程？
    m_stopping = true;
    for(size_t i = 0; i<m_threadCount; ++i) {  // 这里还是主线程在执行，重复执行n次
        // LYSLG_LOG_INFO(g_logger) << " i = " << i;
        tickle();
    }

    // LYSLG_LOG_INFO(g_logger) << " m_rootFiber before";

    if(m_rootFiber) {
        tickle();
    }

    if(m_rootFiber) {
        // while(!stopping()) {
        //     if(m_rootFiber->getState() == Fiber::TERM
        //             || m_rootFiber->getState() == Fiber::EXCEPT) {
        //         m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run,this),0,true));
        //         LYSLG_LOG_INFO(g_logger) << "root->fiber is term ,reset";
            //     t_fiber = m_rootFiber.get();
        //     }
        //     m_rootFiber->call();
        // }
        if(!stopping()) {   // 未停止，
            // 执行m_rootFiber中函数，然后
            m_rootFiber->call();
            // LYSLG_LOG_INFO(g_logger) << " m_rootFiber before";
            // m_rootFiber->swapIn();
            // LYSLG_LOG_INFO(g_logger) << " m_rootFiber after";
        }
        
    }

    std::vector<Thread::ptr> thrs;
    {
        MutexType::Lock lock(m_mutex);
        thrs.swap(m_threads);
    }

    for(auto& i:thrs) {
        i->join();
    }
}

void Scheduler::SetThis(){
    t_scheduler = this;
}

void Scheduler::run(){
    
    LYSLG_LOG_DEBUG(g_logger) << m_name << " run";
    // 设置hook
    set_hook_enable(true);
    SetThis();
    // 非主线程，则为初始化线程中的主协程
    if(lyslg::GetThreadId() != m_rootThread) {
        t_fiber = Fiber::GetThis().get();
    }

    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle,this)));
    Fiber::ptr cb_fiber;

    FiberAndThread ft;
    while(true) {
        ft.reset();
        bool tickle_me = false;
        bool is_active = false;
        {
            MutexType::Lock lock(m_mutex);
            auto it = m_fibers.begin();
            while(it != m_fibers.end()) {
                if(it->thread != -1 && it->thread!=lyslg::GetThreadId()) {
                    ++it;
                    tickle_me = true;
                    continue;
                }

                LYSLG_ASSERT(it->fiber || it->cb);
                if(it->fiber && it->fiber->getState() == Fiber::EXEC) {
                    ++it;
                    continue;
                }

                ft = *it;
                m_fibers.erase(it);
                ++m_activeThreadCount;
                is_active = true;
                break;
            }
            // 上面的代码出循环两种情况
            // it == m_fibers.end()   说明当前线程不需要执行协程队列中的协程
            // 找到需要执行的协程
            tickle_me |= it != m_fibers.end(); // 按位或
        }

        if(tickle_me) {
            tickle();
            // LYSLG_LOG_INFO(g_logger) << "===========================";
        }

        if(ft.fiber && ft.fiber->getState() != Fiber::TERM
                    && ft.fiber->getState() != Fiber::EXCEPT) {
            ft.fiber->swapIn();
            --m_activeThreadCount; // 当前协程执行完，工作线程减1
            // 判断当前协程的状态，执行对应的操作
            if(ft.fiber->getState() == Fiber::READY) {
                schedule(ft.fiber);  // 这里重新加入到协程列队，不需要线程id吗
            }else if(ft.fiber->getState() != Fiber::TERM
                    && ft.fiber->getState() != Fiber::EXCEPT) {
                // 这里我感觉只能是HOLD状态了，也可能为INIT状态
                ft.fiber->m_state = Fiber::HOLD;
            }
            ft.reset();
        }else if(ft.cb) {
            if(cb_fiber) {
                cb_fiber->reset(ft.cb);
            }else{
                cb_fiber.reset(new Fiber(ft.cb));
            }
            ft.reset();
            // LYSLG_LOG_INFO(g_logger) << " cb_fiber before";
            cb_fiber->swapIn();
            // LYSLG_LOG_INFO(g_logger) << " cb_fiber after";
            --m_activeThreadCount;
            if(cb_fiber->getState() == Fiber::READY) {
                schedule(cb_fiber);
                cb_fiber.reset();
            } else if(cb_fiber->getState() == Fiber::EXCEPT
                    || cb_fiber->getState() == Fiber::TERM) {
                cb_fiber->reset(nullptr);
            }else {
                // 此时也可能为INIT状态
                cb_fiber->m_state = Fiber::HOLD;
                cb_fiber.reset();
            }
        }else {
            if(is_active) {
                --m_activeThreadCount;
                continue;
            }
            if(idle_fiber->getState() == Fiber::TERM) {
                LYSLG_LOG_INFO(g_logger) << "idle fiber term";
                break;
            }
           

            ++m_idleThreadCount;
            // LYSLG_LOG_INFO(g_logger) << " idle_fiber before";
            idle_fiber->swapIn();
            // LYSLG_LOG_INFO(g_logger) << " idle_fiber after";
            --m_idleThreadCount;
            if(idle_fiber->getState() != Fiber::TERM
                    && idle_fiber->getState() != Fiber::EXCEPT) {
                idle_fiber->m_state = Fiber::HOLD;
            }
           
        }
    }
}

void Scheduler::tickle(){
    LYSLG_LOG_INFO(g_logger) << "tickle";
}


bool Scheduler::stopping(){
    MutexType::Lock lock(m_mutex);
    // m_fibers.empty() 不为空
    // LYSLG_LOG_INFO(g_logger) << " " << m_autoStop << " " << m_stopping<< " " <<  m_fibers.empty()<< " " <<( m_activeThreadCount == 0); 
    return m_autoStop && m_stopping
        && m_fibers.empty() && m_activeThreadCount == 0;                                
} // 协程列队和工作线程无

void Scheduler::idle(){
    LYSLG_LOG_INFO(g_logger) << "idle";
    while(!stopping()) {
        lyslg::Fiber::YieldToHold();
    }
}
    
}
