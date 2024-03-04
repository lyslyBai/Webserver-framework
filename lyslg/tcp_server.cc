#include "tcp_server.h"
#include "config.h"
#include "log.h"

namespace lyslg{

static lyslg::ConfigVar<uint64_t>::ptr g_tcp_server_read_timeout = 
    lyslg::Config::Lookup("tcp_server.read_timeout", (uint64_t)(60 * 1000 * 2)
                           ,"tcp server read timeout" );

static lyslg::Logger::ptr g_logger = LYSLG_LOG_NAME("system");

TcpServer::TcpServer(lyslg::IoManager* worker,
                    lyslg::IoManager* io_worker,
                    lyslg::IoManager* accept_worker)
    :m_worker(worker)
    ,m_ioWorker(io_worker)
    ,m_acceptWorker(accept_worker)
    ,m_recvTimeout(g_tcp_server_read_timeout->getValue())
    ,m_name("lyslg/1.0.0")
    ,m_isStop(true){

}

TcpServer::~TcpServer() {
    for(auto& i : m_socks) {
        i->close();
    }
    m_socks.clear();
}

void TcpServer::setConf(const TcpServerConf& v) {
    m_conf.reset(new TcpServerConf(v));
}

bool TcpServer::bind(lyslg::Address::ptr addr,bool ssl){
    std::vector<Address::ptr> addrs;
    std::vector<Address::ptr> fails;
    addrs.push_back(addr);
    return bind(addrs,fails,ssl);
}
bool TcpServer::bind(const std::vector<Address::ptr>& addrs,
                        std::vector<Address::ptr>& fails
                        ,bool ssl){
    m_ssl = ssl;
    for(auto& addr : addrs) {
        Socket::ptr sock = Socket::CreateTCP(addr);
        if(!sock->bind(addr)) {
            LYSLG_LOG_ERROR(g_logger) << "bind fail error="
                << errno << " errstr=" << strerror(errno)
                << " addr=[" << addr->toString() << "]";
            fails.push_back(addr);
            continue;
        }
        if(!sock->listen()) {
            LYSLG_LOG_ERROR(g_logger) << "listen fail error="
                << errno << " errstr=" << strerror(errno)
                << " addr=[" << addr->toString() << "]";
            fails.push_back(addr);
            continue;
        }

        // std::cout << " sock=" << sock->getSocket() << std::endl;
        m_socks.push_back(sock);
    }

    if(!fails.empty()) {
        m_socks.clear();
        return false;
    }

    for(auto& i : m_socks) {
        LYSLG_LOG_INFO(g_logger) << "type=" << m_type
            << " name=" << m_name
            << " ssl=" << m_ssl
            << " server bind success: " << *i;
    }
    return true;    
}

void TcpServer::startAccept(Socket::ptr sock) {
    while(!m_isStop) {
        Socket::ptr client = sock->accept();
        if(client) {
            client->setRecvTimeout(m_recvTimeout);
            m_worker->schedule(std::bind(&TcpServer::handleClient,
                            shared_from_this(), client));
        } else {
            LYSLG_LOG_ERROR(g_logger) << "accept errno=" << errno   
                        << " errstr=" << strerror(errno);
        }
    }
}

bool TcpServer::start(){
    if(!m_isStop) {
        return true;
    }
    m_isStop = false;
    for(auto& sock : m_socks) {
        m_acceptWorker->schedule(std::bind(&TcpServer::startAccept,
                                shared_from_this(), sock));
    }
    return true;
}
void TcpServer::stop(){
    m_isStop = true;
    auto self = shared_from_this();
    m_acceptWorker->schedule([this,self]() {
        for(auto& sock:m_socks) {
            sock->cancelAll();
            sock->close();
        }
        m_socks.clear();
    });
}

void TcpServer::handleClient(Socket::ptr client){
    LYSLG_LOG_INFO(g_logger) << "handleClient: " << *client;
}


}