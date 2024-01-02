#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <thread>  // 线程库
#include <pthread.h>
#include <functional>
#include <memory>
#include <semaphore.h>
#include <atomic>

namespace lyslg {

class Semaphore{
public:
    Semaphore(uint32_t count = 0);
    ~Semaphore();

    void wait();
    void notify();
private:
    Semaphore(const Semaphore&) = delete;
    Semaphore(const Semaphore&&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;
private:
    sem_t m_semaphore;
};

template<class T>
struct ScopedLockImpl {
public:
    ScopedLockImpl(T& mutex) 
        :m_mutex(mutex){
        m_mutex.lock();
        m_locked = true;
    }

    ~ScopedLockImpl() {
        unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.lock();
            m_locked = true;
        }
    }

    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    T& m_mutex;
    bool m_locked;
};

template<class T>
struct ReadScopedLockImpl {
public:
    ReadScopedLockImpl(T& mutex) 
        :m_mutex(mutex){
        m_mutex.rdlock();
        m_locked = true;
    }

    ~ReadScopedLockImpl() {
        unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.rdlock();
            m_locked = true;
        }
    }

    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    T& m_mutex;
    bool m_locked;
};

template<class T>
struct WriteScopedLockImpl {
public:
    WriteScopedLockImpl(T& mutex) 
        :m_mutex(mutex){
        m_mutex.wrlock();
        m_locked = true;
    }

    ~WriteScopedLockImpl() {
        unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.wrlock();
            m_locked = true;
        }
    }

    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    T& m_mutex;
    bool m_locked;
};
/*自旋锁（Spin Lock）：
实现方式：

自旋锁是通过不断地在一个循环中进行忙等待（自旋）的方式来实现的。线程在获取锁时，如果发现锁已经被其他线程占用，则不会被阻塞，而是会一直循环检测，直到获取到锁为止。
适用场景：

自旋锁适用于对锁的占用时间非常短暂的情况。如果线程被阻塞的时间较长，自旋锁的性能可能会变差，因为线程在自旋等待时会消耗 CPU 资源。
优点：

自旋锁的优点在于避免了线程切换的开销，适用于对锁的占用时间短暂的情况。
缺点：

在锁被长时间占用的情况下，自旋锁可能导致线程长时间占用 CPU 资源，浪费 CPU 时间。*/
class SpinMutex{
public:
    typedef ScopedLockImpl<SpinMutex> Lock; 
    SpinMutex() {
        pthread_spin_init(&m_mutex,0);
    }

    ~SpinMutex() {
        pthread_spin_destroy(&m_mutex);
    }

    void lock() {
        pthread_spin_lock(&m_mutex);
    }

    void unlock() {
        pthread_spin_unlock(&m_mutex);
    }
private:
    pthread_spinlock_t m_mutex;
};


class CASLock { // 这个和spinlock差不多
public:
    typedef ScopedLockImpl<CASLock> Lock; 
    CASLock() 
        :m_mutex(ATOMIC_FLAG_INIT){
        /*在C++中，使用 std::atomic_flag 时，不需要显式初始化。std::atomic_flag 对象的默认构造函数会将其初始化为“清除”状态。*/
        // m_mutex.clear();  /*但是我测试是发现还是需要这个的，不然直接自旋（上面是GPT的回答），可能受版本影响,不对，因为之前我明确没有初始化，所以才这样的*/
    }
    ~CASLock() {
    }

    void lock() {
        while(m_mutex.test_and_set(std::memory_order_acquire)) {
        }
    }

    void unlock() {
        m_mutex.clear(std::memory_order_release);
    }

private:
    /*使用 volatile 关键字是多余的，因为 std::atomic_flag 已经提供了适当的内存顺序控制。*/
    std::atomic_flag m_mutex;
};

/*互斥锁（Mutex Lock）：
实现方式：

互斥锁是通过操作系统提供的原语（如 futex 在 Linux 中的实现）来实现的。当线程尝试获取锁时，如果锁已经被其他线程占用，线程会被阻塞，直到锁被释放。
适用场景：

互斥锁适用于对锁的占用时间较长的情况。在锁被占用时，等待线程会被放入睡眠状态，不会占用 CPU 资源。
优点：

互斥锁适用于对锁的占用时间较长的情况，可以有效地避免线程在等待锁时占用 CPU 资源。
缺点：

互斥锁的缺点在于线程切换的开销较大，如果锁的占用时间很短，可能会影响性能。*/
/*锁的占用时间：

如果锁的占用时间较短，考虑使用自旋锁，以避免线程切换的开销。
如果锁的占用时间较长，考虑使用互斥锁，以避免线程在等待锁时占用 CPU 资源。
系统负载：

在高负载的系统中，使用自旋锁可能会导致线程长时间占用 CPU 资源，影响系统性能。
在低负载的系统中，使用自旋锁可能更为合适，因为线程切换的开销相对较低。
硬件支持：

有些处理器提供了硬件级别的支持，如原子操作指令，可以提高自旋锁的性能。
在一些情况下，使用硬件支持的自旋锁可能比互斥锁更有效。*/
/*上面是理论分析，，实际上具体机器具体测试，不同机器可能会有不同的效果，起码，在我这里是互斥锁更好，在原机器上，是自旋锁更好*/
class Mutex{
public:
    typedef ScopedLockImpl<Mutex> Lock; 
    Mutex() {
        pthread_mutex_init(&m_mutex,nullptr);
    }

    ~Mutex() {
        pthread_mutex_destroy(&m_mutex);
    }

    void lock() {
        pthread_mutex_lock(&m_mutex);
    }

    void unlock() {
        pthread_mutex_unlock(&m_mutex);
    }
private:
    pthread_mutex_t m_mutex;
};

class NullMutex {
public:
    typedef ScopedLockImpl<NullMutex> Lock;
    NullMutex() {}
    ~NullMutex() {}
    void lock() {}
    void unlock() {}
};



// 一般读频率远大于写频率，用这个会好一点
class RWMutex {
public:
    typedef ReadScopedLockImpl<RWMutex> ReadLock;
    typedef WriteScopedLockImpl<RWMutex> WriteLock;
    RWMutex() {
        pthread_rwlock_init(&m_lock,nullptr);
    }

    ~RWMutex() {
        pthread_rwlock_destroy(&m_lock);
    }

    void rdlock() {
        pthread_rwlock_rdlock(&m_lock);
    }
    void wrlock() {
        pthread_rwlock_wrlock(&m_lock);
    }

    void unlock() {
        pthread_rwlock_unlock(&m_lock);
    }
private:
    pthread_rwlock_t m_lock;
};

class NullRWMutex {
public:
    typedef ReadScopedLockImpl<NullRWMutex> ReadLock;
    typedef WriteScopedLockImpl<NullRWMutex> WriteLock;
    NullRWMutex() {}
    ~NullRWMutex() {}

    void rdlock() {}
    void wrlock() {}
    void unlock() {}
private:
    pthread_rwlock_t m_lock;
};


}


#endif 