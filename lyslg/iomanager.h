#ifndef __LYSLG_IOMANAGER_H__
#define __LYSLG_IOMANAGER_H__

// 基于Epoll的IO协程调度器

#include "scheduler.h"
#include "timer.h"
namespace lyslg{


class IoManager: public Scheduler, public TimerManager{
public:
    typedef std::shared_ptr<IoManager> ptr;
    typedef RWMutex RWMutexType; 

    enum Event{
        // 无事件
        NONE = 0x0,
        // 读事件(EPOLLIN)
        READ = 0x1,
        /// 写事件(EPOLLOUT)
        WRITE = 0x4
    };

public:
    struct FdContext {
        typedef Mutex MutexType;
        struct EventContext{
            Scheduler* scheduler = nullptr; // 事件执行的协程调度器
            Fiber::ptr fiber;         // 事件协程
            std::function<void()> cb; // 事件的回调函数
        };
        //获取事件上下文类
        EventContext& getContext(Event event);
        //  重置事件上下文
        void resetContext(EventContext& ctx);
        // 触发事件
        void triggerEvent(Event event);

        /// 读事件上下文
        EventContext read;
        /// 写事件上下文
        EventContext write;
        /// 事件关联的句柄
        int fd = 0;
        /// 当前的事件
        Event events = NONE;
        /// 事件的Mutex
        MutexType mutex;
    };
public:
    /**
     * @brief 构造函数
     * @param[in] threads 线程数量
     * @param[in] use_caller 是否将调用线程包含进去
     * @param[in] name 调度器的名称
     */
    IoManager(size_t threads=1, bool use_caller = true, const std::string& name="");
    ~IoManager();


    // 1 success, 0 retry , -1 error
    /**
     * @brief 添加事件
     * @param[in] fd socket句柄
     * @param[in] event 事件类型
     * @param[in] cb 事件回调函数
     * @return 添加成功返回0,失败返回-1
     */
    int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
    // 删除时间，同时参数对应的Event
    bool delEvent(int fd, Event event);
    /**
     * @brief 取消事件
     * @param[in] fd socket句柄
     * @param[in] event 事件类型
     * @attention 如果事件存在则触发事件
     */
    bool cancelEvent(int fd, Event event);

    bool cancelAll(int fd);
    /**
     * @brief 返回当前的IOManager
     */
    static IoManager* GetThis();

protected:
    void tickle() override;
    bool stopping() override;
    void idle() override;

    bool stopping(uint64_t& );

    void onTimerInsertedAtFront() override;
    /**
     * @brief 重置socket句柄上下文的容器大小
     * @param[in] size 容量大小
     */
    void contextResize(size_t size);

private:
    /// epoll 文件句柄
    int m_epfd = 0;
    /// pipe 文件句柄
    int m_tickleFds[2];
    /// 当前等待执行的事件数量
    std::atomic<size_t> m_pendingEventCount = {0};
    /// IOManager的Mutex
    RWMutexType m_mutex;
    /// socket事件上下文的容器
    std::vector<FdContext* >m_fdContexts;
};

}




#endif
