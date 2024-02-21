#ifndef __LYSLG_TCP_SERVER_H__
#define __LYSLG_TCP_SERVER_H__


#include <memory>
#include <functional>
#include "address.h"
#include "iomanager.h"
#include "socket.h"
#include "noncopyable.h"

namespace lyslg {

class TcpServer : public std::enable_shared_from_this<TcpServer> 
                    , Noncopyable {
public:
    typedef std::shared_ptr<TcpServer> ptr;
    TcpServer(lyslg::IoManager* worker = lyslg::IoManager::GetThis(),
               lyslg::IoManager* io_woker = lyslg::IoManager::GetThis(),
               lyslg::IoManager* accept_worker = lyslg::IoManager::GetThis() );
    virtual ~TcpServer();

    virtual bool bind(lyslg::Address::ptr addr);
    virtual bool bind(const std::vector<Address::ptr>& addrs,std::vector<Address::ptr>& fails);
    virtual bool start();
    virtual void stop();

    

    uint64_t getRecvTimeout() const { return m_recvTimeout;}
    std::string getName() {return m_name;}
    void setRecvTimeout(uint64_t v) {m_recvTimeout = v;}
    virtual void setName(const std::string& v) {m_name = v;}

    bool isStop() const { return m_isStop;}

protected:
    virtual void handleClient(Socket::ptr client);
    virtual void startAccept(Socket::ptr sock);

private:
    std::vector<Socket::ptr> m_socks;
    IoManager* m_worker;
    IoManager* m_ioWorker;
    IoManager* m_acceptWorker;
    uint64_t m_recvTimeout;
    std::string m_name;
    /// 服务器类型
    // std::string m_type = "tcp";
    bool m_isStop;
};



}



#endif