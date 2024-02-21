#include "http_server.h"
#include "log.h"



namespace lyslg{

namespace http{
static lyslg::Logger::ptr g_logger = LYSLG_LOG_NAME("system");


HttpServer::HttpServer(bool keepalive, lyslg::IoManager* worker,lyslg::IoManager* io_worker 
            ,lyslg::IoManager* accept_worker)
    :TcpServer(worker, io_worker, accept_worker)
    ,m_isKeepalive(keepalive){
    m_dispatch.reset(new ServletDispatch);

}

void HttpServer::setName(const std::string& v) {
    TcpServer::setName(v);
    m_dispatch->setDefault(std::make_shared<NotFoundServlet>(v));
}

void HttpServer::handleClient(Socket::ptr client){
    HttpSession::ptr session(new HttpSession(client));
    do{
        auto req = session->recvRequest();
        if(!req) {
            LYSLG_LOG_WARN(g_logger) << "recv http request fail, errno="
                << errno << " errstr=" << strerror(errno) << " client:"
                << *client;
            break;
        }

        HttpResponse::ptr rsp(new HttpResponse(req->getVersion()
                            ,req->isClose() || !m_isKeepalive));
        rsp->setHeader("Server", getName());
        m_dispatch->handle(req, rsp, session);
        session->sendResponse(rsp);

        // std::cout << *req << std::endl;
        // std::cout << *rsp << std::endl;
        if(!m_isKeepalive || req->isClose()) {
            break;
        }
        
    }while(true);
    session->close();
}


}
}