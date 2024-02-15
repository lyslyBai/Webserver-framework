#include "socket.h"
#include"fd_manager.h"
#include "log.h"
#include "macro.h"
#include "hook.h"
#include <netinet/tcp.h>
#include "fd_manager.h"
#include "iomanager.h"

namespace lyslg{

static lyslg::Logger::ptr g_logger = LYSLG_LOG_NAME("system");


Socket::ptr Socket::CreateTCP(lyslg::Address::ptr address){
    Socket::ptr sock(new Socket(address->getFamily(),TCP,0));
    return sock;
}
Socket::ptr Socket::CreateUDP(lyslg::Address::ptr address){
    Socket::ptr sock(new Socket(address->getFamily(),UDP,0));
    return sock;
}

Socket::ptr Socket::CreateTCPSocket(){
    Socket::ptr sock(new Socket(Ipv4,TCP,0));
    return sock;
}
Socket::ptr Socket::CreateUDPSocket(){
    Socket::ptr sock(new Socket(Ipv4,UDP,0));
    return sock;
}

Socket::ptr Socket::CreateTCPSocket6(){
    Socket::ptr sock(new Socket(Ipv6,TCP,0));
    return sock;
}
Socket::ptr Socket::CreateUDPSocket6(){
    Socket::ptr sock(new Socket(Ipv6,UDP,0));
    return sock;
}

Socket::ptr Socket::CreateUnixTCPSocket(){
    Socket::ptr sock(new Socket(UNIX,TCP,0));
    return sock;
}

Socket::ptr Socket::CreateUnixUDPSocket(){
    Socket::ptr sock(new Socket(UNIX,UDP,0));
    return sock;
}


Socket::Socket(int family,int type,int protocol)
    :m_sock(-1)
    ,m_family(family)
    ,m_type(type)
    ,m_protocol(protocol)
    ,m_isConnected(false){

}

Socket::~Socket(){
    close();
}


int64_t Socket::getSendTimeout(){
    FdCtx::ptr ctx = FdMgr::GetInstance()->get(m_sock);
    if(ctx) {
        return ctx->getTimerout(SO_SNDTIMEO);
    }
    return -1;
}

void Socket::setSendTimeout(int64_t v){
    struct timeval tv{int(v/1000),int(v%1000 * 1000)};
    setOption(SOL_SOCKET,SO_SNDTIMEO,tv);
    return;
}

int64_t Socket::getRecvTimeout(){
    FdCtx::ptr ctx = FdMgr::GetInstance()->get(m_sock);
    if(ctx) {
        return ctx->getTimerout(SO_RCVTIMEO);
    }
    return -1;
}

void Socket::setRecvTimeout(int64_t v){
    struct timeval tv{int(v/1000),int(v%1000 * 1000)};
    setOption(SOL_SOCKET,SO_RCVTIMEO,tv);
    return;
}

bool Socket::getOption(int level, int option,void* result,size_t len){
    int rt = getsockopt(m_sock, level, option, result, (socklen_t*)len);
    if(rt) {
        LYSLG_LOG_DEBUG(g_logger) << "setOption sock=" << m_sock
                << " level=" << level << " option=" << option 
                << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}


bool Socket::setOption(int level, int option,const void* result,socklen_t len){
    
    if(setsockopt(m_sock,level,option,result,(socklen_t)len)) {
        LYSLG_LOG_DEBUG(g_logger) << "setOption sock=" << m_sock
                << " level=" << level << " option=" << option 
                << " errno=" << errno << " errstr=" << strerror(errno);
        LYSLG_ASSERT(false);
        return false;
    }
    return true;
}

Socket::ptr Socket::accept(){
    Socket::ptr sock(new Socket(m_family,m_type,m_protocol));
    int newsock = ::accept(m_sock,nullptr,nullptr);
    if(newsock == -1) {
        LYSLG_LOG_ERROR(g_logger) <<"accept (" << m_sock << ") errno="
                << errno << " errstr=" << strerror(errno);
        return nullptr; 
    }
    if(sock->init(newsock)) {
        return sock;
    }
    return nullptr;
}

bool Socket::init(int sock){
    FdCtx::ptr ctx = FdMgr::GetInstance()->get(sock);
    if(ctx && ctx->isSocket() && !ctx->isClose()) {
        m_sock = sock;
        m_isConnected = true;
        initSock();
        getLocalAddress();
        getRemoteAddress();
        return true;
    }
    return false;
}

bool Socket::bind(const Address::ptr addr){
    if(!isValid()) {
        newSock();
        if(LYSLG_UNLICKLY(!isValid())) {
            return false;
        }
    }

    if(LYSLG_UNLICKLY(addr->getFamily() != m_family)) {
        LYSLG_LOG_ERROR(g_logger) << "bind sock.family (" 
            << m_family << ") addr.fanily(" << addr->getFamily()
            << ") not equal, addr=" << addr->toString();
        return false;
    }

    // UnixAddress::ptr uaddr = std::dynamic_pointer_cast<UnixAddress>(addr);
    // if(uaddr) {
    //     Socket::ptr sock = Socket::CreateUnixTCPSocket();
    //     if(sock->connect(uaddr)) {
    //         return false;
    //     }
    //     // } else {
    //     //     lyslg::FSUtil::Unlink(uaddr->getPath(), true);
    //     // }
    // }
    
    if(::bind(m_sock,addr->getAddr(),addr->getAddrLen())) {
        LYSLG_LOG_ERROR(g_logger) << "bind error errno=" <<errno
            << " errstr=" <<strerror(errno);
        return false;
    }
    getLocalAddress();
    return true;
}

bool Socket::reconnect(uint64_t timeout_ms){
    if(!m_remoteAddress) {
        LYSLG_LOG_ERROR(g_logger) << "reconnect m_remoteAddress is null";
        return false;
    }
    m_localAddress.reset();
    return connect(m_remoteAddress, timeout_ms);
}


bool Socket::connect(const Address::ptr addr,uint64_t timeout_ms){
    m_remoteAddress = addr;
    if(!isValid()) {
        newSock();
        if(LYSLG_UNLICKLY(!isValid())) {
            return false;
        }
    }

    if(LYSLG_UNLICKLY(addr->getFamily() != m_family)) {
        LYSLG_LOG_ERROR(g_logger) << "bind connect.family (" 
            << m_family << ") addr.fanily(" << addr->getFamily()
            << ") not equal, addr=" << addr->toString();
        return false;
    }


    if(timeout_ms == (uint64_t)-1) {
        if(::connect(m_sock,addr->getAddr(),addr->getAddrLen())) {
            LYSLG_LOG_ERROR(g_logger) << "sock=" << m_sock << " connect(" << addr->toString()
                << ") error errno=" << errno << " errstr=" << strerror(errno);
            close();
            return false;
        }
    } else {
        if(connect_with_timeout(m_sock,addr->getAddr(),addr->getAddrLen(),timeout_ms)) {
            LYSLG_LOG_ERROR(g_logger) << "sock=" << m_sock << " connect(" << addr->toString()
                << ") error errno=" << errno << " errstr=" << strerror(errno);
            close();
            return false;
        }
    }
    m_isConnected = true;
    getRemoteAddress();
    getLocalAddress();
    return true;
}

bool Socket::listen(int backlog){
    if(!isValid()) {
        LYSLG_LOG_ERROR(g_logger) << "listen error sock=-1" ;
        return false;
    }

    if(::listen(m_sock,backlog)) {
        LYSLG_LOG_ERROR(g_logger) << "listen error errno=" << errno
             << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::close(){
    if(!m_isConnected && m_sock == -1) {
        return true;
    }
    
    if(m_sock != -1) {
        ::close(m_sock);
        m_sock = -1;
    }

    m_isConnected = false;
    
    return false;
}

int Socket::send(const void* buffer, size_t length,int flags){
    if(isConnected()) {
        return ::send(m_sock,buffer,length,flags);
    }
    return -1;
}

int Socket::send(const iovec* buffers, size_t length,int flags){
    if(isConnected()) {
        msghdr msg;
        memset(&msg,0,sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = length;
        return ::sendmsg(m_sock,&msg,flags);
    }
    return -1;
}

int Socket::sendTo(const void* buffer, size_t length,const Address::ptr to,int flags){
    if(isConnected()) {
        return ::sendto(m_sock,buffer,length,flags,to->getAddr(),to->getAddrLen());
    }
    return -1;
}

int Socket::sendTo(const iovec* buffers, size_t length,const Address::ptr to,int flags){
    if(isConnected()) {
        msghdr msg;
        memset(&msg,0,sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = length;
        msg.msg_name = (void*)to->getAddr();
        msg.msg_namelen = to->getAddrLen();
        return ::sendmsg(m_sock,&msg,flags);
    }
    return -1;
}

int Socket::recv(void* buffer, size_t length, int flags){
    if(isConnected()) {
        return ::recv(m_sock,buffer,length,flags);
    }
    return -1;
}

int Socket::recv(iovec* buffers, size_t length, int flags){
    if(isConnected()) {
        msghdr msg;
        memset(&msg,0,sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = length;
        return ::recvmsg(m_sock,&msg,flags);
    }
    return -1;
}

int Socket::recvFrom(void* buffers, size_t length,Address::ptr from, int flags){
    if(isConnected()) {
        socklen_t len = from->getAddrLen(); 
        return ::recvfrom(m_sock,buffers,length,flags,from->getAddr(),&len);
    }
    return -1;
}

int Socket::recvFrom(iovec* buffers, size_t length,Address::ptr from, int flags){
    if(isConnected()) {
        msghdr msg;
        memset(&msg,0,sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = length;
        msg.msg_name = (void*)from->getAddr();
        msg.msg_namelen = from->getAddrLen();
        return ::recvmsg(m_sock,&msg,flags);
    }
    return -1;
}

Address::ptr Socket::getRemoteAddress(){
    if(m_remoteAddress) {
        return m_remoteAddress;
    }

    Address::ptr result;
    switch(m_family) {
        case AF_INET:
            result.reset(new IPv4Address());
        case AF_INET6:
            result.reset(new IPv6Address());
        case AF_UNIX:
            result.reset(new UnixAddress());
        default:
            result.reset(new UnknowAddress(m_family));
    }

    socklen_t addrlen = result->getAddrLen();
    if(getpeername(m_sock,result->getAddr(),&addrlen)) {
        LYSLG_LOG_ERROR(g_logger) << "getpeername error sock=" << m_sock
            << " errno=" << errno << " errstr=" << strerror(errno);
        return Address::ptr(new UnknowAddress(m_family));
    }

    if(m_family == AF_UNIX) {
        UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
        addr->setAddrLen(addrlen);
    }

    m_remoteAddress = result;
    return m_remoteAddress;
}

Address::ptr Socket::getLocalAddress(){
    if(m_localAddress) {
        return m_localAddress;
    }

    Address::ptr result;
    switch(m_family) {
        case AF_INET:
            result.reset(new IPv4Address());
            break;
        case AF_INET6:
            result.reset(new IPv6Address());
            break;
        case AF_UNIX:
            result.reset(new UnixAddress());
            break;
        default:
            result.reset(new UnknowAddress(m_family));
            break;
    }
    

    socklen_t addrlen = result->getAddrLen();
    if(getsockname(m_sock,result->getAddr(),&addrlen)) {
        LYSLG_LOG_ERROR(g_logger) << "getpeername error sock=" << m_sock << " m_family=" << m_family
            << " errno=" << errno << " errstr=" << strerror(errno);
        return Address::ptr(new UnknowAddress(m_family));
    }

    if(m_family == AF_UNIX) {
        UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
        addr->setAddrLen(addrlen);
    }

    m_localAddress = result;
    return m_localAddress;
}


bool Socket::isValid() const{
    return m_sock != -1;
}

int Socket::getError(){
    int error = 0;
    size_t len = sizeof(error);
    if(!getOption(SOL_SOCKET,SO_ERROR,&error,len)) {
        return -1;
    }
    return error;
}

std::ostream& Socket::dump(std::ostream& os) const{
    os << "[Socket sock=" << m_sock
       << " is_connected=" << m_isConnected
       << " family=" << m_family
       << " type=" << m_type
       << " protocol=" << m_protocol;
    if(m_localAddress) {
        os << " local_address=" << m_localAddress->toString();
    }
    if(m_remoteAddress) {
        os << " remote_address=" << m_remoteAddress->toString();
    }
    os << "]";
    return os;
}

std::string Socket::toString() const {
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

bool Socket::cancelRead(){
    return IoManager::GetThis()->cancelEvent(m_sock,lyslg::IoManager::READ);
}

bool Socket::cancelWrite(){
    return IoManager::GetThis()->cancelEvent(m_sock,lyslg::IoManager::WRITE);
}
bool Socket::cancelAccept(){
    return IoManager::GetThis()->cancelEvent(m_sock,lyslg::IoManager::READ);
}
bool Socket::cancelAll(){
    return IoManager::GetThis()->cancelAll(m_sock);
}

void Socket::initSock(){
    int val = 1;
    setOption(SOL_SOCKET,SO_REUSEADDR,val);
    if(m_family != AF_UNIX && m_type == SOCK_STREAM) { // m_family != AF_UNIX && 
        setOption(IPPROTO_TCP,TCP_NODELAY,val); // 这个不支持UNIX
    }
}

void Socket::newSock(){
    m_sock = socket(m_family,m_type,m_protocol);
    if(LYSLG_LICKLY(m_sock != -1)) {
        initSock();
    } else {
        LYSLG_LOG_ERROR(g_logger) << "socket (" << m_family
        << ", " << m_type << ", " << m_protocol << ") errno="
        << errno << " strerr" << strerror(errno);
    }
}

std::ostream& operator<<(std::ostream& os, const Socket& sock){
    return sock.dump(os);
}

}