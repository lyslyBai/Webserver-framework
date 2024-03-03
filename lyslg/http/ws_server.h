#ifndef __LYSLG_HTTP_WS_SERVER_H__
#define __LYSLG_HTTP_WS_SERVER_H__

#include "tcp_server.h"
#include "ws_session.h"
#include "ws_servlet.h"

namespace lyslg {
namespace http {

class WSServer : public TcpServer {
public:
    typedef std::shared_ptr<WSServer> ptr;

    WSServer(lyslg::IoManager* worker = lyslg::IoManager::GetThis()
             , lyslg::IoManager* io_worker = lyslg::IoManager::GetThis()
             , lyslg::IoManager* accept_worker = lyslg::IoManager::GetThis());

    WSServletDispatch::ptr getWSServletDispatch() const { return m_dispatch;}
    void setWSServletDispatch(WSServletDispatch::ptr v) { m_dispatch = v;}
protected:
    virtual void handleClient(Socket::ptr client) override;
protected:
    WSServletDispatch::ptr m_dispatch;
};

}
}

#endif
