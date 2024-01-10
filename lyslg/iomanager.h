#ifndef __LYSLG_IOMANAGER_H__
#define __LYSLG_IOMANAGER_H__


#include "scheduler.h"

namespace lyslg{


class IoManager: public Scheduler{
public:
    typedef std::shared_ptr<IoManager> ptr;
    typedef RWMutex RWMutexType; 

    enum Event{
        NONE = 0x0,
        READ = 0x1,
        WRITE = 0x2
    };

public:
    struct FDContext {
        typedef Mutex MuexType;
        struct EventContext{
            Scheduler* schedule = nullptr; // 事件执行的协程调度器
            Fiber::ptr fiber;         // 事件协程
            std::function<void()> cb; // 事件的回调函数
        };

        EventContext read;
        EventContext write;
        int fd;
        Event m_evetn = NONE;
        MutexType mutex;
    };
public:
    IoManager(size_t threads=1, bool use_caller = true, const std::string& name="");
    ~IoManager();


    // 1 success, 0 retry , -1 error
    int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
    bool delEvent(int fd, Event event);
    bool cancel(int fd, Event event);

    bool cancelAll(int fd);
    static IoManager* GetThis();

protected:
    void tickle() override;
    bool stopping() override;
    void idle() override;

private:
    int m_epfd = 0;
    int m_tickleFds[2];

    std::atomic<size_t> m_pendingEventCount = {0};
    RWMutexType m_mutex;
    std::vector<FDContext* >m_fdContexts;
};

}




#endif
