#include "ws_server.h"
#include "log.h"

namespace lyslg {
namespace http {

static lyslg::Logger::ptr g_logger = LYSLG_LOG_NAME("system");

WSServer::WSServer(lyslg::IoManager* worker, lyslg::IoManager* io_worker, lyslg::IoManager* accept_worker)
    :TcpServer(worker, io_worker, accept_worker) {
    m_dispatch.reset(new WSServletDispatch);
    m_type = "websocket_server";
}

void WSServer::handleClient(Socket::ptr client) {
    LYSLG_LOG_DEBUG(g_logger) << "handleClient " << *client;
    WSSession::ptr session(new WSSession(client));
    do {
        HttpRequest::ptr header = session->handleShake();
        LYSLG_LOG_INFO(g_logger) << " HttpRequest=" << "/n" << *header;
        if(!header) {
            LYSLG_LOG_DEBUG(g_logger) << "handleShake error";
            break;
        }
        WSServlet::ptr servlet = m_dispatch->getWSServlet(header->getPath());
        if(!servlet) {
            LYSLG_LOG_DEBUG(g_logger) << "no match WSServlet";
            break;
        }
        int rt = servlet->onConnect(header, session);
        if(rt) {
            LYSLG_LOG_DEBUG(g_logger) << "onConnect return " << rt;
            break;
        }
        while(true) {
            auto msg = session->recvMessage();
            LYSLG_LOG_INFO(g_logger) << " msg=" << msg << "======";
            if(!msg) {
                break;
            }
            rt = servlet->handle(header, msg, session);
            if(rt) {
                LYSLG_LOG_DEBUG(g_logger) << "handle return " << rt;
                break;
            }
        }
        servlet->onClose(header, session);
    } while(0);
    session->close();
}

}
}
