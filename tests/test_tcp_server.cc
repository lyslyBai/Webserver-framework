#include "tcp_server.h"
#include "iomanager.h"
#include "log.h"

lyslg::Logger::ptr g_logger = LYSLG_LOG_ROOT();

void runs()
{
    auto addr = lyslg::Address::LookupAny("0.0.0.0:8033");
    // auto addr2 = lyslg::UnixAddress::ptr(new lyslg::UnixAddress("../tmp/unix_addr"));
   // LYSLG_LOG_INFO(g_logger) << *addr << " - " << *addr2;
    
    std::vector<lyslg::Address::ptr> addrs;
    addrs.push_back(addr);
    // addrs.push_back(addr2);

    lyslg::TcpServer::ptr tcp_server(new lyslg::TcpServer);
    std::vector<lyslg::Address::ptr> fails;
    while(!tcp_server->bind(addrs,fails)) {
        sleep(2);
    }
    tcp_server->start();

}

int main()
{
    lyslg::IoManager iom(1);
    iom.schedule(runs);
    // run_3();
    return 0;
}