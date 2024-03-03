#include "timer.h"
#include "util.h"

namespace lyslg{


Timer::Timer(uint64_t ms, std::function<void()> cb,
            bool recurring, TimerManager* manager)
    :m_recurring(recurring)
    ,m_ms(ms)
    ,m_cb(cb)
    ,m_manager(manager){

    m_next = lyslg::GetCurrentMS() + m_ms;
}

Timer::Timer(uint64_t next)
    :m_next(next){
}

bool Timer::cancel(){
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if(m_cb) {
        m_cb = nullptr;
        auto it = m_manager->m_timers.find(shared_from_this());
        m_manager->m_timers.erase(it);
        return true;
    }
    return false;
}
bool Timer::refresh(){
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if(!m_cb) {
       return false;
    }
    auto it = m_manager->m_timers.find(shared_from_this());
    if(it == m_manager->m_timers.end()) {
        return false;
    }
    // 可以考虑一下这里为什么不直接修改 ??
    //C++ 中的 std::set 是一个有序的关联容器，它在元素插入时会按照某种排序准则
    //（默认是升序）进行排序。一旦元素插入到 std::set 中，它们就会按照排序准则保持有序。
    // 然而，std::set 并不会在元素的其他属性发生变化时自动重新排序。
    m_manager->m_timers.erase(it);
    m_next =  lyslg::GetCurrentMS() + m_ms;
    m_manager->m_timers.insert(shared_from_this());
    return true;
}

bool Timer::reset(uint64_t ms,bool from_now){
    if(ms == m_ms && !from_now) {
        return true;
    }
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if(!m_cb) {
        return false;
    }
    auto it =m_manager->m_timers.find(shared_from_this());
    if(it == m_manager->m_timers.end()) {
        return false;
    }
    m_manager->m_timers.erase(it);
    uint64_t start = 0;
    if(from_now) {
        start = lyslg::GetCurrentMS();
    } else {
        start = m_next - m_ms;
    }

    m_ms = ms;
    m_next = start + m_ms;
    m_manager->addTimer(shared_from_this(),lock);
    return true;
}   


bool Timer::Comparator::operator() (const Timer::ptr& lhs, const Timer::ptr rhs) const {
    /*如果两个 Timer::ptr 智能指针都为空（即 nullptr），那么返回 false，
    因为它们相等。如果 lhs 为空而 rhs 不为空，那么返回 true，因为一个空指
    针被认为小于非空指针。如果 lhs 不为空而 rhs 为空，那么返回 false，
    因为非空指针被认为大于空指针。*/
    if(!lhs && !rhs) {
        return false;
    }
    if(!lhs) {
        return true;
    }
    if(!rhs) {
        return false;
    }

    if(lhs->m_next < rhs->m_next) {
        return true;
    }
    if(lhs->m_next > rhs->m_next) {
        return false;
    }
    return lhs.get() < rhs.get(); 
}


TimerManager::TimerManager(){
    m_previousTime = lyslg::GetCurrentMS();
}

TimerManager::~TimerManager(){

}

Timer::ptr TimerManager::addTimer(uint16_t ms,std::function<void()> cb
                        , bool recurring ){
    Timer::ptr timer(new Timer(ms,cb,recurring,this));
    RWMutexType::WriteLock lock(m_mutex);
    addTimer(timer,lock);
    return timer;
}
/*这样的操作会检查弱指针是否关联着有效的共享指针，如果是，则将其转换为共享指针，否则得到一个空的共享指针。*/
static void OnTimer(std::weak_ptr<void> weak_cond,std::function<void()> cb) {
    std::shared_ptr<void> tmp = weak_cond.lock();
    if(tmp) {
        cb();
    }
}
// 这个函数的回调函数为std::bind(&OnTimer,weak_cond,cb)，执行时会先检查weak_cond是否存在，即是否能转化为智能指针，能，则执行
Timer::ptr TimerManager::addCondictionTimer(uint16_t ms,std::function<void()> cb
                                , std::weak_ptr<void> weak_cond
                                , bool recurring){
    return addTimer(ms,std::bind(&OnTimer,weak_cond,cb),recurring);
}

uint64_t TimerManager::getNextTimer(){
    RWMutexType::ReadLock lock(m_mutex);
    // m_tickled
    m_tickled = false;
    if(m_timers.empty()){
        return ~0ull;
    }

    const Timer::ptr next = *m_timers.begin();
    uint64_t now_ms = lyslg::GetCurrentMS();
    // 即这个计时器已经执行完了
    if(now_ms >= next->m_next) {
        return 0;
    } else {
        return next->m_next - now_ms;
    }
}

void  TimerManager::listExpiredCb(std::vector<std::function<void()> >& cbs){
    uint64_t now_ms = lyslg::GetCurrentMS();
    std::vector<Timer::ptr> expired;
    {
        RWMutexType::ReadLock lock(m_mutex);
        if(m_timers.empty()) {
            return;
        }
    }
    RWMutexType::WriteLock lock(m_mutex);

    bool rollover = detectClockRollover(now_ms);
    if(!rollover && ((*m_timers.begin())->m_next > now_ms)) {
        return ;
    }

    Timer::ptr now_timer(new Timer(now_ms));
    // 从容器中找到不小于它的第一个元素的位置（即大于等于，还需要去除等于，将之放到队列中）
    /*while(it != m_timers.end() && (*it)->m_next == now_ms) 
    使用 while 循环，继续向后移动迭代器 it 直到遇到第一个过期时间
    不等于当前时间的计时器。这是因为 lower_bound 可能会找到多个计
    时器的过期时间等于当前时间，而我们只关心第一个过期时间不等于当
    前时间的计时器。
    整个过程的目的是为了找到在容器中第一个过期时间大于
     now_timer 的位置，以便在后续的逻辑中正确处理这些计时器。*/
    auto it = rollover ? m_timers.end() : m_timers.lower_bound(now_timer);
    while(it!=m_timers.end() && (*it)->m_next == now_ms) {
        ++it;
    }
    // 这里系统回绕则全部放入过期队列
    expired.insert(expired.begin(),m_timers.begin(),it);
    m_timers.erase(m_timers.begin(),it);
    cbs.reserve(expired.size());

    for(auto& timer:expired){
        cbs.push_back(timer->m_cb);
        if(timer->m_recurring) {
            timer->m_next = now_ms + timer->m_ms; // 这里以当前时间为基准，再次插入到定时器队列
            m_timers.insert(timer);
        } else {
            timer->m_cb = nullptr;
        }

    }

}

void TimerManager::addTimer(Timer::ptr val,RWMutexType::WriteLock& lock){
    /*在C++中，std::set 的 insert 方法会返回一个 std::pair 对象，
    该对象包含两个成员：first 和 second。对于 std::set，insert 
    方法的返回值的 first 成员指向插入的元素（或者已经存在的相同元素）
    ，而 second 成员是一个布尔值，表示插入是否成功。*/
    auto it = m_timers.insert(val).first;
    bool at_front = (it == m_timers.begin()) && !m_tickled;
    if(at_front) {
        m_tickled = true;
    }

    lock.unlock();
    if(at_front) {
        onTimerInsertedAtFront();
    }
}

/*这段代码用于检测系统时钟是否发生了溢出（即回绕）。
在某些情况下，系统时钟的值可能会回绕，这可能导致定时器逻辑出现问题。*/
bool TimerManager::detectClockRollover(uint64_t now_ms){
    bool rollover = false;
    if(now_ms < m_previousTime && now_ms < (m_previousTime - 60*60*1000)) {
        rollover = true;
    }
    m_previousTime = now_ms;
    return rollover;
}

bool TimerManager::hasTimer(){
    RWMutexType::ReadLock lock(m_mutex);
    return !m_timers.empty();
}


}