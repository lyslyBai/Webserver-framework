#ifndef __LYSLG_HTTP_SERVER_H__
#define __LYSLG_HTTP_SERVER_H__

#include "../tcp_server.h"
#include "http_session.h"
#include "servlet.h"


namespace lyslg{
namespace http{

class HttpServer : public TcpServer {
public:
    typedef std::shared_ptr<HttpServer> ptr;
    HttpServer(bool keepalive = false   
                ,lyslg::IoManager* worker = lyslg::IoManager::GetThis()
                ,lyslg::IoManager* io_worker = lyslg::IoManager::GetThis()
                ,lyslg::IoManager* accept_worker = lyslg::IoManager::GetThis());

    ServletDispatch::ptr getServletDispatch() const { return m_dispatch;}
    void setServletDispatch(ServletDispatch::ptr v) {m_dispatch = v;}

    virtual void setName(const std::string& v) override;
protected:
    virtual void handleClient(Socket::ptr client) override;

private:
    bool m_isKeepalive;
    ServletDispatch::ptr m_dispatch;
};


}

}




#endif