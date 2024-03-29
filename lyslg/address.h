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

/*IPAddress（IP 地址）:

使用场景： IP 地址用于在 Internet 协议（IP）网络中标识和定位设备。
它是在 Internet 上进行通信时的基本构建块，用于唯一标识网络中的计算机或设备。
不同之处： IP 地址可以分为 IPv4 和 IPv6 两个主要版本。IPv4 地址是 32 位的，
通常表示为点分十进制（例如，192.168.1.1），而 IPv6 地址是 128 位的，
通常表示为八组四位十六进制数（例如，2001:0db8:85a3:0000:0000:8a2e:0370:7334）。
Unix域套接字地址:

使用场景： Unix 域套接字是在本地主机上进行进程间通信（IPC）时使用的一种机制。
它不涉及网络，而是在同一台计算机上的进程之间创建连接，实现本地通信。
不同之处： Unix 域套接字不涉及网络协议栈，因此它们在性能上通常比网络套接字更高效。
它们使用文件系统路径作为地址，而不是 IP 地址和端口号。Unix 域套接字通常用于同一台
机器上的进程之间的通信，因为它们提供了一种轻量级的、低延迟的通信方式。*/

namespace lyslg{

class IPAddress;
/**
 * @brief 网络地址的基类,抽象类
 */
class Address {
public:
    typedef std::shared_ptr<Address> ptr;
    virtual ~Address() {};
    /**
     * @brief 通过sockaddr指针创建Address
     * @param[in] addr sockaddr指针
     * @param[in] addrlen sockaddr的长度
     * @return 返回和sockaddr相匹配的Address,失败返回nullptr
     */
    static Address::ptr Create(const sockaddr* addr, socklen_t addrlen);
    /**
     * @brief 通过host地址返回对应条件的所有Address
     * @param[out] result 保存满足条件的Address
     * @param[in] host 域名,服务器名等.举例: www.sylar.top[:80] (方括号为可选内容)
     * @param[in] family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @param[in] type socketl类型SOCK_STREAM、SOCK_DGRAM 等
     * @param[in] protocol 协议,IPPROTO_TCP、IPPROTO_UDP 等
     * @return 返回是否转换成功
     */
    static bool Lookup(std::vector<Address::ptr>& result,const std::string& host, 
                            int family = AF_INET, int type = 0,int protocol = 0);
    static Address::ptr LookupAny(const std::string& host, 
                            int family = AF_INET, int type = 0,int protocol = 0);
    static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string& host, 
                            int family = AF_INET, int type = 0,int protocol = 0);

    static bool GetInterfaceAddresses(std::multimap<std::string, 
                                      std::pair<Address::ptr, uint32_t> >& result,
                                      int family = AF_UNSPEC);
    static bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t> >& result,
                                      const std::string& iface, int family = AF_UNSPEC);
    int getFamily() const;
    virtual const sockaddr* getAddr() const = 0;
    virtual sockaddr* getAddr() = 0;
    virtual socklen_t getAddrLen() const = 0;
    virtual std::ostream& insert(std::ostream& os) const = 0;
    std::string toString() const;
    bool operator<(const Address& rhs) const;
    bool operator==(const Address& rhs) const;
    bool operator!=(const Address& rhs) const;
};

class IPAddress : public Address {
public:
    typedef std::shared_ptr<IPAddress> ptr;
    // 和子类的返回类型不同，所以不会被覆盖
    static IPAddress::ptr Create(const char* address, uint16_t port =0);
    virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;
    virtual IPAddress::ptr networdAddress(uint32_t prefix_len) = 0;
    virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;

    virtual uint16_t getPort() const = 0;
    virtual void setPort(uint16_t v) = 0;
};

class IPv4Address : public IPAddress{
public:
    typedef std::shared_ptr<IPv4Address> ptr;
    // 继承的父类的默认构造函数吧

    IPv4Address(const sockaddr_in& address);
    IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);

    static IPv4Address::ptr Create(const char* address,uint16_t port = 0);
    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networdAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;

    uint16_t getPort() const override;
    void setPort(uint16_t v) override;
private:
    sockaddr_in m_addr;
};

class IPv6Address : public IPAddress{
public:
    typedef std::shared_ptr<IPv6Address> ptr;
    static IPv6Address::ptr Create(const char* address, uint16_t port = 0);


    IPv6Address();
    IPv6Address(const sockaddr_in6& address);
    IPv6Address(const uint8_t* address[16], uint16_t port = 0);

    sockaddr* getAddr() override;
    const sockaddr* getAddr() const override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networdAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;

    uint16_t getPort() const override;
    void setPort(uint16_t v) override;
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

    sockaddr* getAddr() override;
    const sockaddr* getAddr() const override;
    socklen_t getAddrLen() const override;
    void setAddrLen(uint32_t v);
    std::string getPath() const;
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
    sockaddr* getAddr() override;
    const sockaddr* getAddr() const override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;
private:
    struct sockaddr m_addr;
};


/**
 * @brief 流式输出Address
 */
std::ostream& operator<<(std::ostream& os, const Address& addr);




    
}



#endif