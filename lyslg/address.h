#ifndef __LYSLG_ADDRESS_H__
#define __LYSLG_ADDRESS_H__

#include <iostream>
#include <memory>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/un.h>
#include <map>
#include <vector>

namespace lyslg{

class IPAddress;
class Address {
public:
    typedef std::shared_ptr<Address> ptr;
    virtual ~Address() {};

    static Address::ptr Create(const sockaddr* addr, socklen_t addrlen);
    static bool Lookup(std::vector<Address::ptr>& result,const std::string& host, 
                            int family = AF_UNSPEC, int type = 0,int protocol = 0);
    static Address::ptr LookupAny(const std::string& host, 
                            int family = AF_UNSPEC, int type = 0,int protocol = 0);
    static std::shared_ptr<IPAddress> LookupAnyIPAdress(const std::string& host, 
                            int family = AF_UNSPEC, int type = 0,int protocol = 0);

    static bool GetInterfaceAddresses(std::multimap<std::string, 
                                      std::pair<Address::ptr, uint32_t> >& result,
                                      int family = AF_UNSPEC);
    static bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t> >& result,
                                      const std::string& iface, int family = AF_UNSPEC);
    int getFamily() const;
    virtual const sockaddr* getAddr() const = 0;
    virtual socklen_t getAddrLen() const = 0;
    virtual std::ostream& insert(std::ostream& os) const = 0;
    std::string toString();
    bool operator<(const Address& rhs) const;
    bool operator==(const Address& rhs) const;
    bool operator!=(const Address& rhs) const;
};

class IPAddress : public Address {
public:
    typedef std::shared_ptr<IPAddress> ptr;
    // 和子类的返回类型不同，所以不会被覆盖
    static IPAddress::ptr Create(const char* address, uint32_t port =0);
    virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;
    virtual IPAddress::ptr networdAddress(uint32_t prefix_len) = 0;
    virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;

    virtual uint32_t getPort() const = 0;
    virtual void setPort(uint32_t v) = 0;
};

class IPv4Address : public IPAddress{
public:
    typedef std::shared_ptr<IPv4Address> ptr;
    // 继承的父类的默认构造函数吧
    IPv4Address(const sockaddr_in& address);
    IPv4Address(uint32_t address = INADDR_ANY, uint32_t port = 0);

    static IPv4Address::ptr Create(const char* address,uint32_t port = 0);
    const sockaddr* getAddr() const override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networdAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;

    uint32_t getPort() const override;
    void setPort(uint32_t v) override;
private:
    sockaddr_in m_addr;
};

class IPv6Address : public IPAddress{
public:
    typedef std::shared_ptr<IPv6Address> ptr;
    static IPv6Address::ptr Create(const char* address, uint32_t port = 0);


    IPv6Address();
    IPv6Address(const sockaddr_in6& address);
    IPv6Address(const uint8_t* address[16], uint32_t port = 0);

    const sockaddr* getAddr() const override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networdAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;

    uint32_t getPort() const override;
    void setPort(uint32_t v) override;
private:
    sockaddr_in6 m_addr;
};

/*Unix 域套接字是一种进程间通信（IPC）的方式，
允许同一台主机上的进程通过套接字进行通信。它不涉及网络协议，
而是通过文件系统的文件路径来实现通信。因此，Unix 域套接字使
用一个文件系统路径（地址），而不是 IP 地址和端口号。*/

class UnixAddress : public Address {
public:
    typedef std::shared_ptr<UnixAddress> ptr;
    UnixAddress();
    UnixAddress(const std::string& path);

    const sockaddr* getAddr() const override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;
private:
    struct sockaddr_un m_addr;
    socklen_t m_length;
};

class UnknowAddress : public Address {
public:
    typedef std::shared_ptr<UnknowAddress> ptr;
    UnknowAddress(int);
    UnknowAddress(const sockaddr& addr);
    const sockaddr* getAddr() const override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;
private:
    struct sockaddr m_addr;
};







    
}



#endif