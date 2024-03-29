#ifndef __THREAD_H__
#define __THREAD_H__

#include "mutex.h"
#include "noncopyable.h"

namespace lyslg{



class Thread {
public:
    typedef std::shared_ptr<Thread> ptr;
    

    Thread(std::function<void()> cb,const std::string& name);
    ~Thread();

    pid_t getId() const {return m_id; }
    const std::string& getName() const {return m_name;}

    static void SetName(const std::string& name);

    void join();
    
    static void* run(void* arg);
    static Thread* GetThis();
    static const std::string& GetName();
private:
    Thread(const Thread&) = delete;
    Thread(const Thread&&) = delete;
    Thread& operator=(const Thread&) = delete;
private:
    pid_t m_id = -1;
    pthread_t m_thread = 0;
    std::function<void ()> m_cb;
    std::string m_name;
    Semaphore m_semaphore;
};

}


#endif