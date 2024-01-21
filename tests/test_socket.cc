#include "socket.h"
#include "lyslg.h"
#include "iomanager.h"

static lyslg::Logger::ptr g_looger = LYSLG_LOG_ROOT();

void test_socket() {
    //std::vector<lyslg::Address::ptr> addrs;
    //lyslg::Address::Lookup(addrs, "www.baidu.com", AF_INET);
    //lyslg::IPAddress::ptr addr;
    //for(auto& i : addrs) {
    //    lyslg_LOG_INFO(g_looger) << i->toString();
    //    addr = std::dynamic_pointer_cast<lyslg::IPAddress>(i);
    //    if(addr) {
    //        break;
    //    }
    //}
    lyslg::IPAddress::ptr addr = lyslg::Address::LookupAnyIPAddress("www.baidu.com");
    if(addr) {
        LYSLG_LOG_INFO(g_looger) << "get address: " << addr->toString();
    } else {
        LYSLG_LOG_ERROR(g_looger) << "get address fail";
        return;
    }

    lyslg::Socket::ptr sock = lyslg::Socket::CreateTCP(addr);
    addr->setPort(80);
    LYSLG_LOG_INFO(g_looger) << "addr=" << addr->toString();
    if(!sock->connect(addr)) {
        LYSLG_LOG_ERROR(g_looger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        LYSLG_LOG_INFO(g_looger) << "connect " << addr->toString() << " connected";
    }

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    if(rt <= 0) {
        LYSLG_LOG_INFO(g_looger) << "send fail rt=" << rt;
        return;
    }

    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());

    if(rt <= 0) {
        LYSLG_LOG_INFO(g_looger) << "recv fail rt=" << rt;
        return;
    }

    buffs.resize(rt);
    LYSLG_LOG_INFO(g_looger) << buffs;
}

void test2() {
    lyslg::IPAddress::ptr addr = lyslg::Address::LookupAnyIPAddress("www.baidu.com:80");
    if(addr) {
        LYSLG_LOG_INFO(g_looger) << "get address: " << addr->toString();
    } else {
        LYSLG_LOG_ERROR(g_looger) << "get address fail";
        return;
    }

    lyslg::Socket::ptr sock = lyslg::Socket::CreateTCP(addr);
    if(!sock->connect(addr)) {
        LYSLG_LOG_ERROR(g_looger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        LYSLG_LOG_INFO(g_looger) << "connect " << addr->toString() << " connected";
    }

    uint64_t ts = lyslg::GetCurrentUS();
    for(size_t i = 0; i < 10000000000ul; ++i) {
        if(int err = sock->getError()) {
            LYSLG_LOG_INFO(g_looger) << "err=" << err << " errstr=" << strerror(err);
            break;
        }

        //struct tcp_info tcp_info;
        //if(!sock->getOption(IPPROTO_TCP, TCP_INFO, tcp_info)) {
        //    LYSLG_LOG_INFO(g_looger) << "err";
        //    break;
        //}
        //if(tcp_info.tcpi_state != TCP_ESTABLISHED) {
        //    LYSLG_LOG_INFO(g_looger)
        //            << " state=" << (int)tcp_info.tcpi_state;
        //    break;
        //}
        static int batch = 10000000;
        if(i && (i % batch) == 0) {
            uint64_t ts2 = lyslg::GetCurrentUS();
            LYSLG_LOG_INFO(g_looger) << "i=" << i << " used: " << ((ts2 - ts) * 1.0 / batch) << " us";
            ts = ts2;
        }
    }
}

int main(int argc, char** argv) {
    lyslg::IoManager iom;
    // iom.schedule(&test_socket);
    iom.schedule(&test2);
    return 0;
}
