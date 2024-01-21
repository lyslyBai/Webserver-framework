#include "address.h"
#include "my_endian.h"
#include "log.h"
#include <sstream>
#include <netdb.h>
#include <ifaddrs.h>


namespace lyslg{

static lyslg::Logger::ptr g_logger = LYSLG_LOG_NAME("system");

template<class T>
static T CreateMask(uint32_t bits) {
    // 前bits位为0
    return static_cast<T>((1 << (sizeof(T) * 8 - bits)) - 1);
}
// 用于计算输入值 value 二进制表示中设置为1的位数。
template <class T>
static uint32_t CountBytes(T value) {
    uint32_t result = 0;
    for(;value;++result) {
        // 这有效地清除了最低有效位的1,高位不变，直到value为0，结束循环
        value &= value -1;   // value 至少有一位由1变成0，0变成1，则不变，1变成0，则覆盖
    }
    return result;
}


Address::ptr Address::Create(const sockaddr* addr, socklen_t addrlen){
    if(addr == nullptr) {
        return nullptr;
    }

    Address::ptr result;
    switch(addr->sa_family) {
        case AF_INET:
            result.reset(new IPv4Address(*(const sockaddr_in*)addr));
            break;
        case AF_INET6:
            result.reset(new IPv6Address(*(const sockaddr_in6*)addr));
            break;
        default:
            result.reset(new UnknowAddress(*addr));
            break;
    }
    return result;
}

Address::ptr Address::LookupAny(const std::string& host, 
                        int family, int type,int protocol){
    std::vector<Address::ptr> result;
    if(Lookup(result,host,family,type,protocol)) {
        return result[0];  // 返回第一个ip
    }
    return nullptr;
}

std::shared_ptr<IPAddress> Address::LookupAnyIPAddress(const std::string& host, 
                        int family, int type ,int protocol ){
    std::vector<Address::ptr> result;
    if(Lookup(result,host,family,type,protocol)) {
        for(auto& i : result) {
            IPAddress::ptr v = std::dynamic_pointer_cast<IPAddress>(i); // 可以转换成IP指针，这属于IP地址，返回
            if(v) {
                return v;
            }
        }
    }
    return nullptr;                        
}

// 寻找符合条件的ip
bool Address::Lookup(std::vector<Address::ptr>& result,const std::string& host, 
                    int family , int type,int protocol){
    addrinfo hints, *results,*next;
    hints.ai_flags = 0;
    hints.ai_family = family;
    hints.ai_socktype = type;
    hints.ai_protocol = protocol;
    hints.ai_addrlen = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    std::string node;
    const char* service = NULL;

    // 检查 ipv6address service
    if(!host.empty() && host[0] == '[') {
        /*该函数的目的是在host字符串的剩余部分中查找右方括号 ']' 的位置。*/
        const char* endipv6 = (const char* )memchr(host.c_str() +1 ,']',host.size()-1);
        if(endipv6) {
            // TODO check out of range
            if(*(endipv6 + 1) == ':') {
                service = endipv6 +2;  // 这个部分实际上是为了找到host中的端口号，例子：[www.baidu.com]:80,service 即为端口号
            }
        }
        /*host.c_str() 返回的是指向字符串 host 中第一个字符的指针*/
        node = host.substr(1,endipv6 - host.c_str() - 1);  // 这个不是是为了获取域名或ip，即为上例中的www.baidu.com
    }

    // 检查 node service
    if(node.empty()) { // 即node未初始化   处理 www.baidu.com:ftp ,情况
        service = (const char* )memchr(host.c_str(),':',host.size());  
        if(service) {
            if(!memchr(service +1 ,':',host.c_str() + host.size() - service -1)) {
                node = host.substr(0,service - host.c_str());  // 上面未找到：，node = www.baidu.com
                service++;  // service = ftp
            }
        }
        
    }

    if(node.empty()) {
        node = host;   
    }

    /*getaddrinfo 函数是一个用于获取地址信息的系统调用，它可以根据给定的主机名
    （或 IP 地址字符串）、服务名（或端口号字符串）、地址族、套接字类型和协议等信
    息，返回一个或多个符合条件的地址信息。*/
    int error = getaddrinfo(node.c_str(),service,&hints,&results);
    if(error) {
        LYSLG_LOG_ERROR(g_logger) << "Address::lockup getaddress (" << host <<" "
                            << family << ", " << type << ") err=" << error << " errstr="
                            << strerror(errno);
        return false;
    }
    // results 为一个链表的头部
    next = results;
    while(next) {
        result.push_back(Create(next->ai_addr, (socklen_t)next->ai_addrlen));
        next = next->ai_next;
    }
    // 调用者需要负责释放这块内存
    freeaddrinfo(results);
    return true;  
}

// 未指定网卡
bool Address::GetInterfaceAddresses(std::multimap<std::string, 
                                    std::pair<Address::ptr, uint32_t> >& result,int family){
    struct ifaddrs *next, *results;
    // 于获取系统中所有网络接口的信息
    if(getifaddrs(&results) != 0) {
        LYSLG_LOG_ERROR(g_logger) << "Adress::GetInterfaceAddress getifaddrs "
                                  << " err=" << errno << " errstr=" << strerror(errno);
        return 0;
    }

    try{
        // 遍历链表，打印每个接口的信息
        for(next = results;next;next = next->ifa_next) {
            Address::ptr addr;
            uint32_t prefix_len = ~0u;
            if(family != AF_UNSPEC && family != next->ifa_addr->sa_family) {
                continue;
            }
            switch(next->ifa_addr->sa_family) {
                case AF_INET:
                    {
                        addr = Create(next->ifa_addr, sizeof(sockaddr_in));
                        // 先next->ifa_netmask，然后(sockaddr_in*),指向网络接口信息结构体中网关掩码的指针 ，， 但有点问题，问为什么这里有子网掩码了，其他其他地方还需要prefix_len
                        uint32_t netmask = ((sockaddr_in*)next->ifa_netmask)->sin_addr.s_addr;
                        // 通过子网掩码，获得，prefix_len，其实也就是网络位的个数
                        prefix_len = CountBytes(netmask);
                    }
                    break;
                case AF_INET6:
                    {
                        addr = Create(next->ifa_addr, sizeof(sockaddr_in6));
                        in6_addr& netmask = ((sockaddr_in6*)next->ifa_netmask)->sin6_addr;
                        for(int i = 0;i<16;++i) {
                            prefix_len += CountBytes(netmask.s6_addr[i]);
                        }
                    }
                    break;
                default:
                    break;
            }

            if(addr) {
                result.insert(std::make_pair(next->ifa_name, std::make_pair(addr,prefix_len)));
            }

        }

    }catch(...) {
        LYSLG_LOG_ERROR(g_logger) << "Adress::GetInterfaceAddress exception ";
        freeifaddrs(results);
        return false;

    }
    freeifaddrs(results);
    return true;
}

// 指定网卡
bool Address::GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t> >& result,
                                      const std::string& iface, int family){
    if(iface.empty() || iface == "*") {
        if(family == AF_INET || family == AF_UNSPEC) {
            // 提供默认的网络接口
            result.push_back(std::make_pair(Address::ptr(new IPv4Address()),0u));
        }
        if(family == AF_INET6 || family == AF_UNSPEC) {
             // 提供默认的网络接口
            result.push_back(std::make_pair(Address::ptr(new IPv6Address()),0u));
        }
        return true;
    }

    std::multimap<std::string , std::pair<Address::ptr,uint32_t>> results;

    if(!GetInterfaceAddresses(results,family)) {
        return false;
    }
    // 该函数返回一个 std::pair，表示给定键在容器中的等值范围。
    // range 是一个 std::pair，其中 range.first 是指向第一个匹配 iface 的迭代器，
    // 而 range.second 是指向最后一个匹配的后一个位置的迭代器。
    auto its = results.equal_range(iface);
    for(;its.first != its.second; ++its.first){
        result.push_back(its.first->second);
    }
    return true;
}



int Address::getFamily() const{
    return getAddr()->sa_family;
}

std::string Address::toString(){
    std::stringstream ss;
    insert(ss);
    return ss.str();
}

IPAddress::ptr IPAddress::Create(const char* address, uint16_t port){
    addrinfo hints ,* results;
    memset(&hints,0,sizeof(addrinfo));

    hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = AF_UNSPEC;

    int error = getaddrinfo(address,NULL,&hints,&results);
    if(error) {
        LYSLG_LOG_ERROR(g_logger) << "IPAdress::Create(" << address 
                << ", " << port << ") error=" << error
                << " errno=" << errno << " errstr=" << strerror(errno);
        return nullptr;
    }

    try{
        IPAddress::ptr result = std::dynamic_pointer_cast<IPAddress>(
            Address::Create(results->ai_addr,(socklen_t)results->ai_addrlen));
        if(result) {
            result->setPort(port);
        }
        freeaddrinfo(results);
        return result;
    }catch(...){
        freeaddrinfo(results);
        return nullptr;
    }
}



bool Address::operator<(const Address& rhs) const {
    socklen_t minlen = std::min(getAddrLen(),rhs.getAddrLen());
    /*
    如果 ptr1 和 ptr2 的前 num 个字节相等，则返回 0。
    如果 ptr1 比 ptr2 小，则返回负值。
    如果 ptr1 比 ptr2 大，则返回正值。
    */
    int result = memcmp(getAddr(),rhs.getAddr(),minlen);
    if(result < 0) {
        return true;
    } else if(result > 0) {
        return false;
    } else if(getAddrLen() < rhs.getAddrLen()) {
        return true;
    }
    return false;
}

bool Address::operator==(const Address& rhs) const{
    return getAddrLen() == rhs.getAddrLen() 
        && memcmp(getAddr(),rhs.getAddr(),getAddrLen()) == 0;
}
bool Address::operator!=(const Address& rhs) const{
    return !(*this == rhs);
}  

IPv4Address::IPv4Address(const sockaddr_in& address) {
    m_addr = address;
}
 
IPv4Address::IPv4Address(uint32_t address, uint16_t port){
    memset(&m_addr,0,sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = byteswapOnLittleEndian(port);
    m_addr.sin_addr.s_addr = byteswapOnLittleEndian(address);
}

IPv4Address::ptr IPv4Address::Create(const char* address,uint16_t port){
    IPv4Address::ptr rt(new IPv4Address);
    rt->m_addr.sin_port = byteswapOnLittleEndian(port);
    int result = inet_pton(AF_INET,address,&rt->m_addr.sin_addr);
    if(result <= 0) {
        LYSLG_LOG_ERROR(g_logger) << "IPv4Address::Create (" << address << ", "
                                  << port << ") rt=" <<result << " errno=" << errno      
                                  << " errstr=" << strerror(errno);
        return nullptr;
    }
    return rt;
}

const sockaddr* IPv4Address::getAddr() const {
    return (sockaddr*)&m_addr;
}

sockaddr*  IPv4Address::getAddr() {
    return (sockaddr*)&m_addr;
}
socklen_t  IPv4Address::getAddrLen() const {
    return sizeof(m_addr);
}

/*这么做是因为在某些情况下，移位操作可能导致高位的位被填充，
而 & 0xff 的操作会清除高位的所有位，保留最低的 8 位。这样
可以确保提取的字节值在有效范围内，不受到其他位的影响。*/
std::ostream&  IPv4Address::insert(std::ostream& os) const{
    uint32_t addr = byteswapOnLittleEndian(m_addr.sin_addr.s_addr);
    os << ((addr >> 24) & 0xff) << "."  
       << ((addr >> 16) & 0xff) << "."
       << ((addr >> 8) & 0xff) << "."
       <<(addr & 0xff);
    os << ":" << byteswapOnLittleEndian(m_addr.sin_port);
    return os;
}   

/*广播地址是结束(即对应的主机位为1),*/
IPAddress::ptr  IPv4Address::broadcastAddress(uint32_t prefix_len) {
    if(prefix_len > 32) {
        return nullptr;
    }

    sockaddr_in baddr(m_addr);
    baddr.sin_addr.s_addr |= byteswapOnLittleEndian(
        CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr(new IPv4Address(baddr));
}
/*而网络地址是子网IP地址的开始(对应的主机位为0)，感觉有问题？？？？？*/
IPAddress::ptr  IPv4Address::networdAddress(uint32_t prefix_len) {
    if(prefix_len > 32) {
        return nullptr;
    }

    sockaddr_in baddr(m_addr); // 感觉这里应该取反，得到的是知网掩码
    baddr.sin_addr.s_addr &= ~byteswapOnLittleEndian(
        CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr(new IPv4Address(baddr));
}
IPAddress::ptr  IPv4Address::subnetMask(uint32_t prefix_len) {
    sockaddr_in subnet;
    memset(&subnet,0,sizeof(subnet));
    subnet.sin_family = AF_INET;
    subnet.sin_addr.s_addr = ~byteswapOnLittleEndian(
        CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr(new IPv4Address(subnet));
}

uint16_t IPv4Address::getPort() const {
    return byteswapOnLittleEndian(m_addr.sin_port);
}

// 要注意port为16位地址，如果写成32位，再转换字节序，截断成16位地址，会出错
void IPv4Address::setPort(uint16_t v) {
    // LYSLG_LOG_INFO(g_logger) << "m_addr.sin_port:" << v << " byteswapOnLittleEndian(v):" << byteswapOnLittleEndian(v);
    m_addr.sin_port = byteswapOnLittleEndian(v);
    // LYSLG_LOG_INFO(g_logger) << "m_addr.sin_port:" << m_addr.sin_port;
}

IPv6Address::ptr IPv6Address::Create(const char* address, uint16_t port ){
    IPv6Address::ptr rt(new IPv6Address);
    rt->m_addr.sin6_port = byteswapOnLittleEndian(port);
    int result = inet_pton(AF_INET6,address,&rt->m_addr.sin6_addr);
    if(result <= 0) {
        LYSLG_LOG_ERROR(g_logger) << "IPv6Address::Create (" << address << ", "
                                  << port << ") rt=" <<result << " errno" << errno      
                                  << " errstr=" << strerror(errno);
        return nullptr;
    }
    return rt;
}

IPv6Address::IPv6Address(){
    memset(&m_addr,0,sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
}

IPv6Address::IPv6Address(const sockaddr_in6& address){
    m_addr = address;
}

IPv6Address::IPv6Address(const uint8_t* address[16], uint16_t port){
    memset(&m_addr,0,sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
    m_addr.sin6_port = byteswapOnLittleEndian(port);
    memcpy(&m_addr.sin6_addr.s6_addr,address,16);
}

const sockaddr* IPv6Address::getAddr() const {
    return (sockaddr*)&m_addr;
}

sockaddr* IPv6Address::getAddr() {
    return (sockaddr*)&m_addr;
}
socklen_t  IPv6Address::getAddrLen() const {
    return sizeof(m_addr);
}

/*省略零只能应用于连续的一组零，不能单独省略。因此，省略单独的零是不合法的。*/
/*:: 只能在地址中使用一次，表示省略一个或多个连续的零。
IPv6 地址的最后一块为零时是可以省略的*/
std::ostream&  IPv6Address::insert(std::ostream& os) const{
    os << "[";
    uint16_t* addr = (uint16_t*)m_addr.sin6_addr.s6_addr;
    bool used_zeros = false;  // 判断是否省略过0
    for(size_t i = 0;i<8;i++) {
        // 如果为0，可省略
        if(addr[i] == 0 && !used_zeros) {
            continue;
        }
        // 前面的一组0被省略了，后面的0就要输出了
        if(i && addr[i-1] == 0 && !used_zeros) {
            os << ":";
            used_zeros = true;
        }
        if(i) {
            os << ":";
        }
        os << std::hex << (int)byteswapOnLittleEndian(addr[i]) << std::dec;
    }
    // 如果最后一项为0，且前面没有省略过0，这输入::，表示省略0
    if(!used_zeros && addr[7]) {
        os << "::";
    }

    os << "]:" << byteswapOnLittleEndian(m_addr.sin6_port);
    return os;
}

IPAddress::ptr  IPv6Address::broadcastAddress(uint32_t prefix_len) {
    sockaddr_in6 baddr(m_addr);
    // 将指定前缀长度的位设为0,后面设为1，
    baddr.sin6_addr.s6_addr[prefix_len/8] |= CreateMask<uint8_t>(prefix_len % 8);
    // 将后续所有位都设为1
    for(uint32_t i = prefix_len/8+1;i < 16 ;i++) {
        baddr.sin6_addr.s6_addr[i] = 0xff;
    }
    return IPv6Address::ptr(new IPv6Address(baddr));
}

IPAddress::ptr  IPv6Address::networdAddress(uint32_t prefix_len) {
    sockaddr_in6 baddr(m_addr);
    baddr.sin6_addr.s6_addr[prefix_len/8] &= ~CreateMask<uint8_t>(prefix_len % 8);
    // 网络地址，全置为0
    for(uint32_t i = prefix_len/8+1;i < 16 ;i++) {
        baddr.sin6_addr.s6_addr[i] = 0x00;
    }
    return IPv6Address::ptr(new IPv6Address(baddr));
}

IPAddress::ptr  IPv6Address::subnetMask(uint32_t prefix_len) {
    sockaddr_in6 subnet;
    memset(&subnet,0,sizeof(subnet));
    subnet.sin6_family = AF_INET6;
    subnet.sin6_addr.s6_addr[prefix_len/8] = ~CreateMask<uint8_t>(prefix_len % 8);
    // 前面几个字节全为1
    for(uint32_t i =0; i<prefix_len/8;i++) {
        subnet.sin6_addr.s6_addr[i] = 0xff;
    }
    return IPv6Address::ptr(new IPv6Address(subnet));
}

uint16_t IPv6Address::getPort() const {
    return byteswapOnLittleEndian(m_addr.sin6_port);
}
void IPv6Address::setPort(uint16_t v) {
    m_addr.sin6_port = byteswapOnLittleEndian(v);
}

static size_t MAX_PATH_LEN = sizeof(((sockaddr_un*)0)->sun_path)-1;

UnixAddress::UnixAddress(){
    memset(&m_addr,0,sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    m_length = offsetof(sockaddr_un,sun_path) + MAX_PATH_LEN;

}

// 包括结尾的 null 终止符 \0。
// std::strlen(myString);得到的包含末尾的'\0'
UnixAddress::UnixAddress(const std::string& path){
    memset(&m_addr,0,sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    m_length = path.size()+1; 
    // 这里好像永远不会满足，除非path为c风格字符串，注释掉试一下
    // 若为空字符串拷贝过来，最后会存在一个'\0',但不会计数
    // 若为空字符串，path.empty()为true，所以这点有问题
    // if(!path.empty() && path[0] == '\0') {
    //     --m_length;
    // }

    if(m_length > sizeof(m_addr.sun_path)) {
        throw std::logic_error("path too long");
    }
    // 这里为空字符串的应该不会考'\0'吧
    memcpy(m_addr.sun_path,path.c_str(),m_length);
    // 这里m_length等于结构体的实际存储空间
    m_length += offsetof(sockaddr_un,sun_path);
}

const sockaddr* UnixAddress::getAddr() const {
    return (sockaddr*)&m_addr;
}

sockaddr* UnixAddress::getAddr(){
    return (sockaddr*)&m_addr;
}

socklen_t UnixAddress::getAddrLen() const{
    return m_length;
}

void UnixAddress::setAddrLen(uint32_t v) {
    m_length = v;
}

std::ostream& UnixAddress::insert(std::ostream& os) const{

    if(m_length > offsetof(sockaddr_un,sun_path)
        && m_addr.sun_path[0] == '\0'){
            return os << "\\0" << std::string(m_addr.sun_path +1 
            , m_length - offsetof(sockaddr_un,sun_path) - 1);
    }
    return os << m_addr.sun_path;
}

UnknowAddress::UnknowAddress(int family){
    memset(&m_addr,0,sizeof(m_addr));
    m_addr.sa_family = family;
}

UnknowAddress::UnknowAddress(const sockaddr& addr){
    m_addr = addr;
}

const sockaddr* UnknowAddress::getAddr() const {
    return (sockaddr*)&m_addr;
}

sockaddr* UnknowAddress::getAddr(){
    return (sockaddr*)&m_addr;
}
socklen_t UnknowAddress::getAddrLen() const{
    return sizeof(m_addr);
}
std::ostream& UnknowAddress::insert(std::ostream& os) const{
    os << "[UnknowAddress family=" << m_addr.sa_family << "]";
    return os; 
}


}