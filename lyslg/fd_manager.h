#ifndef __FD_MANAGER_H__
#define __FD_MANAGER_H__

#include <memory>
#include <vector>
#include "mutex.h"
#include "singleton.h"

namespace lyslg{

class FdCtx : public std::enable_shared_from_this<FdCtx> {
public:
    typedef std::shared_ptr<FdCtx> ptr;
    FdCtx(int fd);
    ~FdCtx();

    bool init();
    bool isInited() const {return m_isInit;}
    bool isSocket() const {return m_isSocket;}
    bool isClose() const {return m_isClosed;}

    /**
     * @brief 设置用户主动设置非阻塞
     * @param[in] v 是否阻塞
     */
    void setUserNonblock(bool v) {m_userNonblock = v;}
     /**
     * @brief 获取是否用户主动设置的非阻塞
     */
    bool getUserNonblock() const {return m_userNonblock;}

    /**
     * @brief 设置系统非阻塞
     * @param[in] v 是否阻塞
     */
    void setSysNonblock(bool v) {m_sysNonblock = v;}
    /**
     * @brief 获取系统非阻塞
     */
    bool getSysNonblock() const {return m_sysNonblock;}
    /**
     * @brief 设置超时时间
     * @param[in] type 类型SO_RCVTIMEO(读超时), SO_SNDTIMEO(写超时)
     * @param[in] v 时间毫秒
     */
    void setTimerout(int type,uint64_t v);
    /**
     * @brief 获取超时时间
     * @param[in] type 类型SO_RCVTIMEO(读超时), SO_SNDTIMEO(写超时)
     * @return 超时时间毫秒
     */
    uint64_t getTimerout(int type);
private:
/*确保初始化列表中的顺序与声明的顺序相匹配*/
    /// 是否初始化
    bool m_isInit: 1;
    /// 是否socket
    bool m_isSocket: 1;
    /// 是否hook非阻塞
    bool m_sysNonblock: 1;
    /// 是否用户主动设置非阻塞
    bool m_userNonblock: 1;
    /// 是否关闭
    bool m_isClosed: 1;
    /// 文件句柄
    int m_fd;
    /*一般情况下，uint64_t 作为一个整数类型可以很好地满足大多数场景
    ，以ms为单位而 std::chrono::nanoseconds 则提供了更高的精度。*/
    /// 读超时时间毫秒
    uint64_t m_recvTimeout;
    /// 写超时时间毫秒
    uint64_t m_sendTimeout;
};

class FdManager {
public:
    typedef RWMutex RWMutexType;  
    // typedef Mutex MutexType;
    FdManager();
    /**
     * @brief 获取/创建文件句柄类FdCtx
     * @param[in] fd 文件句柄
     * @param[in] auto_create 是否自动创建
     * @return 返回对应文件句柄类FdCtx::ptr
     */
    FdCtx::ptr get(int fd, bool auto_create = false);
    void del(int fd);

private:
    RWMutexType m_mutex;
    /// 文件句柄集合
    std::vector<FdCtx::ptr> m_datas;

};
/// 文件句柄单例
typedef Singleton<FdManager> FdMgr;



}


#endif