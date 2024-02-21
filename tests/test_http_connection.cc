#include <iostream>
#include "http_connection.h"
#include "log.h"
#include "iomanager.h"


static lyslg::Logger::ptr g_logger = LYSLG_LOG_ROOT();


void test_pool() {
    lyslg::http::HttpConnectionPool::ptr pool(new lyslg::http::HttpConnectionPool(
                "www.baidu.com", "", 80, false, 10, 1000 * 30, 5));

    lyslg::IoManager::GetThis()->addTimer(1000, [pool](){
            auto r = pool->doGet("/", 300);
            LYSLG_LOG_INFO(g_logger) << r->toString();
    }, true);
}

void run() {
    lyslg::Address::ptr addr = lyslg::Address::LookupAnyIPAddress("www.baidu.com:80");
    if(!addr) {
        LYSLG_LOG_INFO(g_logger) << "get addr error";
        return;
    }

    lyslg::Socket::ptr sock = lyslg::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    if(!rt) {
        LYSLG_LOG_INFO(g_logger) << "connect " << *addr << " failed";
        return;
    }

    lyslg::http::HttpConnection::ptr conn(new lyslg::http::HttpConnection(sock));
    lyslg::http::HttpRequest::ptr req(new lyslg::http::HttpRequest);
    LYSLG_LOG_INFO(g_logger) << " req:" << std::endl
        << *req;
    
    conn->sendRequest(req);
    auto rsp = conn->recvResponse();

    if(!rsp) {
        LYSLG_LOG_INFO(g_logger) << "recv response error";
        return;
    }
    LYSLG_LOG_INFO(g_logger) << "rsp:" << std::endl
        << *rsp;

    LYSLG_LOG_INFO(g_logger) << "--------------";

    auto ret = lyslg::http::HttpConnection::DoGet("http://www.baidu.com",300);
    LYSLG_LOG_INFO(g_logger) << "result=" << ret->result
        << " errno=" << ret->error << " rsp" 
        << (ret->response ? ret->response->toString() : "");
    
    LYSLG_LOG_INFO(g_logger) << "=========================";
    test_pool();
}


int main() {
    lyslg::IoManager iom(2);
    iom.schedule(run);
    return 0;
}