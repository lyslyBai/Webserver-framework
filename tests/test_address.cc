#include "address.h"
#include "log.h"
#include <map>



lyslg::Logger::ptr g_logger = LYSLG_LOG_ROOT();

void test() {
    std::vector<lyslg::Address::ptr> addrs;

    bool v = lyslg::Address::Lookup(addrs,"www.baidu.com");
    if(!v) {
        LYSLG_LOG_ERROR(g_logger) << "lookup fail";
        return;
    }

    for(size_t i = 0;i<addrs.size();i++) {
        LYSLG_LOG_INFO(g_logger) << i << " - " << addrs[i]->toString();
    }

}

void test_iface() {
    std::multimap<std::string, std::pair<lyslg::Address::ptr,uint32_t> > results;

    bool v = lyslg::Address::GetInterfaceAddresses(results);

    if(!v) {
        LYSLG_LOG_ERROR(g_logger) << "GetInterfaceAddresses fail";
        return;
    }

    for(auto& i:results) {
        LYSLG_LOG_INFO(g_logger) << i.first << " - " << i.second.first->toString() << " - "
                << i.second.second;
    }
}

void test_ipv4() {
    // auto addr = lyslg::IPAddress::Create("www.sylar.top");
    auto addr = lyslg::IPAddress::Create("127.0.0.8");
    if(addr) {
        LYSLG_LOG_INFO(g_logger) << addr->toString();
    }
}

int main(int argc,char** argv) {
    // test_ipv4();
    test();
    // test_iface();
    return 0;
}