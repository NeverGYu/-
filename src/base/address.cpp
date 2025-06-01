#include "address.hpp"
#include "log.h"
#include "endian.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <string.h>
#include <sstream>
#include <arpa/inet.h>

namespace sylar
{

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

/**
 *  @brief 该模板函数的作用生成一个低位全是1，高位全是0的掩码 
 *  @param[in] bits 表示希望高位有多少个0
 *  @example bits = 24
 *           sizeof(T) * 8 - bits = 32 - 24 = 8
 *           1 << 8 ----> 把1向左移动8为：100000000
 *           减一可得：011111111 => 00000000 00000000 00000000 11111111
 */
template<class T>
static T CreateMask(uint32_t bits)
{
    return (1 << (sizeof(T) * 8 - bits)) - 1;
}

/**
 *  @brief 该模板函数的作用是统计传入的二进制参数有多少个 1
 *  @example value:     01100110
 *           value-1:   01100101
 *             &    :   01100100  ---------> 这就删除掉了value第二位的1
 */
template<class T>
static uint32_t CountBytes(T value)
{
    uint32_t result = 0;
    for (; value; ++result)
    {
        value &= value - 1;
    }
    return result;

}


/*-------------------------------------  Address  -------------------------------------------------*/
bool Address::Lookup(std::vector<Address::ptr>& result, const std::string& host, int family, int type, int protocol)
{
    // 定义地址信息结构体
    addrinfo hints, *results, *next;
    hints.ai_flags = 0;
    hints.ai_family = family;
    hints.ai_socktype = type;
    hints.ai_protocol = protocol;
    hints.ai_addrlen = 0;
    hints.ai_next = NULL;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    // 主机名/域名(www.baidu.com)
    std::string node;
    // 服务名/端口(http)
    const char* service = NULL;
    // 检查 ipv6 address service --> 假设处理的是ipv6地址 [::1]:80
    if (!host.empty() && host[0] == '[')
    {
        // memchr用于对给定内存起始位置 s 进行范围为 n 的对 c 的逐字节进行比较，返回比较成功的位置，否则返回nullptr
        const char* endipv6 = (const char*)memchr(host.c_str() + 1, ']', host.size() - 1);
        if (endipv6)
        {
            if (*(endipv6 + 1) == ':')
            {
                service = endipv6 + 2;
            }
            node = host.substr(1,endipv6 - host.c_str()  -1);
        }
    }
     // 检查 ipv4 address / port service --> www.baidu.com:80 | 192.168.200.2:80
     if (node.empty())
     {
         service = (const char*)memchr(host.c_str(), ':', host.size());
         if (service)
         {
            // 这用来判断是否是这种情况 --> ::1:8
            if (!memchr(service + 1, ':', host.c_str() + host.size() - service - 1))
            {
                // 这说明是ipv4 + 端口模式
                node = host.substr(0, service - host.c_str());
                ++service;
            }  
         }
     }
     // 这说明找不到 : , 没有端口，只有ipv4地址
     if (node.empty())
     {
         node = host;
     }

     int error = getaddrinfo(node.c_str(), service, &hints, &results);
     if (error) {
         SYLAR_LOG_DEBUG(g_logger) << "Address::Lookup getaddress(" << host << ", "
                                   << family << ", " << type << ") err=" << error << " errstr="
                                   << gai_strerror(error);
         return false;
     }
     next = results;
     while (next)
     {
         result.push_back(Create(next->ai_addr, (socklen_t)next->ai_addrlen));
         next = next->ai_next;
     }
     freeaddrinfo(results);
     return !result.empty();
    
}


Address::ptr Address::LookupAny(const std::string& host, int family, int type, int protocol)
{
    std::vector<Address::ptr> results;
    if (Lookup(results, host ,family, type, protocol))
    {
        return results[0];
    }
    return nullptr;
}

IPAddress::ptr Address::LookupAnyIPAddress(const std::string& host,  int family, int type, int protocol)
{
    std::vector<Address::ptr> results;
    if (Lookup(results, host, family, type, protocol))
    {
        for (auto& i : results)
        {
            IPAddress::ptr v = std::dynamic_pointer_cast<IPAddress>(i);
            if (v)
            {
                return v;
            }  
        }
    }
    return nullptr;
}

bool Address::GetInterfaceAddresses(std::multimap<std::string, std::pair<Address::ptr, uint32_t>>& result, int family)
{
    // 定义网卡地址结构
    struct ifaddrs* next, *results;
    // 通过api函数来获得本地主机的网卡信息
    if (getifaddrs(&results) != 0)
    {
        SYLAR_LOG_DEBUG(g_logger) << "Address::GetInterfaceAddress getifaddrs "
                                  << " err="
                                  << errno << " errstr=" << strerror(errno);
        return false;
    }
    try
    {
        // 遍历本地主机的网卡
        for (next = results; next; next = next->ifa_next)
        {
            Address::ptr addr;
            uint32_t prefix_len = ~0u;
            if (family != AF_UNSPEC && family != next->ifa_addr->sa_family)
            {
                // 这表示协议族未明确，并且给定的协议不符合本网卡的地址协议，所以直接跳过
                continue;
            }
            switch (next->ifa_addr->sa_family)
            {
            case AF_INET:
                {
                    addr = Create(next->ifa_addr, sizeof(sockaddr_in));
                    uint32_t netmask = ((sockaddr_in*)next->ifa_netmask)->sin_addr.s_addr;
                    prefix_len = CountBytes(netmask);
                }
                break;
            case AF_INET6:
                {
                    addr = Create(next->ifa_addr,sizeof(sockaddr_in6));
                    in6_addr& netmask = ((sockaddr_in6*)next->ifa_netmask)->sin6_addr;
                    prefix_len = 0;
                    for (int i = 0; i < 16; ++i)
                    {
                        prefix_len += CountBytes(netmask.s6_addr[i]);
                    }
                }
                break;
            default:
                break;
            }
            if (addr)
            {
                result.insert(std::make_pair(next->ifa_name, std::make_pair(addr, prefix_len)));
            }
        }   
    }
    catch(...)
    {
        SYLAR_LOG_ERROR(g_logger) << "Address::GetInterfaceAddresses exception";
        freeifaddrs(results);
        return false;
    }
    freeifaddrs(results);
    return !result.empty();
}

bool Address::GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t>>&result, const std::string& iface, int family)
{
    if (iface.empty() || iface == "*")
    {
        if (family == AF_INET || family == AF_UNSPEC)
        {
            result.push_back(std::make_pair(Address::ptr(new IPv4Address()), 0u));
        }
        if (family == AF_INET6 || family == AF_UNSPEC)
        {
            result.push_back(std::make_pair(Address::ptr(new IPv6Address()), 0u));
        }
        return true;
    }
    std::multimap<std::string, std::pair<Address::ptr, uint32_t>> results;
    if (!GetInterfaceAddresses(results, family))
    {
        return false;
    }
    auto its = results.equal_range(iface);
    for (; its.first != its.second; ++its.first)
    {
        result.push_back(its.first->second);
    }
    return !result.empty();
}

int Address::getFamily() const
{
    return getAddr()->sa_family;
}

std::string Address::toString() const
{
    std::stringstream ss;
    insert(ss);
    return ss.str();
}

Address::ptr Address::Create(const sockaddr* addr, socklen_t addrlen)
{
    if (!addr)
    {
        return nullptr;
    }
    Address::ptr result;
    switch (addr->sa_family)
    {
    case AF_INET:
        result.reset(new IPv4Address(*(const sockaddr_in*)addr));
        break;
    case AF_INET6:
        result.reset(new IPv6Address(*(const sockaddr_in6*)addr));
        break;
    default:
        result.reset(new UnknownAddress(*addr));
        break;
    }
    return result;
}

bool Address::operator<(const Address& rhs) const 
{
    socklen_t minlen = std::min(getAddrlen(), rhs.getAddrlen());
    int result = memcmp(getAddr(), rhs.getAddr(), minlen);
    if (result < 0)
    {
        return true;
    }
    else if (result > 0)
    {
        return false;
    }
    else if (getAddrlen() < rhs.getAddrlen())
    {
        return true;
    }
    return false;
}

bool Address::operator==(const Address& rhs) const
{
    return getAddrlen() == rhs.getAddrlen() && memcmp(getAddr(), rhs.getAddr(), getAddrlen()) == 0;
}

bool Address::operator!=(const Address& rhs) const
{
    return !(this == &rhs);
}


/*-------------------------------------  IPAddress  -----------------------------------------------*/
IPAddress::ptr IPAddress::Create(const char* address, uint16_t port)
{
    addrinfo hints, *results;
    bzero(&hints, sizeof(addrinfo));
    // AI_NUMERICHOST 表示：“你传入的主机名是一个纯数字 IP（例如 127.0.0.1 或 ::1），不要进行 DNS 解析” --> 不进行域名到IP地址的转换。
    hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = AF_UNSPEC;
    // 通过api 函数getaddrinfo来生成出满足条件的地址
    int error = getaddrinfo(address,NULL,&hints,&results);
    if (error)
    {
        SYLAR_LOG_DEBUG(g_logger) << "IPAddress::Create(" << address
                                  << ", " << port << ") error=" << error
                                  << " errno=" << errno << " errstr=" << strerror(errno);
        return nullptr;
    }

    try
    {
        IPAddress::ptr result = std::dynamic_pointer_cast<IPAddress>(Address::Create(results->ai_addr, (socklen_t)results->ai_addrlen));
        if (result)
        {
            result->setPort(port);
        }
        freeaddrinfo(results);
        return result;
    }
    catch(...)
    {
        freeaddrinfo(results);
        return nullptr;
    }
}

/*-------------------------------------  IPv4Address  ---------------------------------------------*/
IPv4Address::ptr IPv4Address::Create(const char* address, uint16_t port)
{
    IPv4Address::ptr rt(new IPv4Address);
    rt->m_sockaddr.sin_port = byteswapOnLittleEndian(port);
    int result = inet_pton(AF_INET, address, &rt->m_sockaddr.sin_addr);
    if (!result)
    {
        SYLAR_LOG_DEBUG(g_logger) << "IPv4Address::Create(" << address << ", "
                                  << port << ") rt=" << result << " errno= " << errno
                                  << " errstr=" << strerror(errno);
        return nullptr;
    }
    return rt;
}

IPv4Address::IPv4Address(const sockaddr_in& addr)
{
    m_sockaddr = addr;
}

IPv4Address::IPv4Address(uint32_t address, uint16_t port)
{
    bzero(&m_sockaddr, sizeof(sockaddr_in));
    m_sockaddr.sin_port = byteswapOnLittleEndian(port);
    m_sockaddr.sin_addr.s_addr = byteswapOnLittleEndian(address);
    m_sockaddr.sin_family = AF_INET;
}

const sockaddr* IPv4Address::getAddr() const
{
    return (sockaddr*)&m_sockaddr;
}

sockaddr* IPv4Address::getAddr() 
{
    return (sockaddr*)&m_sockaddr;
}

socklen_t IPv4Address::getAddrlen() const   
{
    return sizeof(m_sockaddr);
}
    
std::ostream& IPv4Address::insert(std::ostream& os) const  
{
    uint32_t addr = byteswapOnLittleEndian(m_sockaddr.sin_addr.s_addr);
    os << ((addr >> 24) & 0xff) << "."
       << ((addr >> 16) & 0xff) << "."
       << ((addr >> 8) & 0xff) << "."
       << (addr & 0xff);
    os << ":" << byteswapOnLittleEndian(m_sockaddr.sin_port);
    return os;
}

IPAddress::ptr IPv4Address::broadcastAddress(uint32_t prefix_len)
{
    // 判断子网掩码的长度是否大于32
    if (prefix_len > 32)
    {
        return nullptr;
    }
    sockaddr_in addr(m_sockaddr);
    // s_addr主要是IPv4地址的二进制形式(uint32_t)
    addr.sin_addr.s_addr |= byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr(new IPv4Address(addr));
}

IPAddress::ptr IPv4Address::networkAddress(uint32_t prefix_len)
{
    if (prefix_len > 32)
    {
        return nullptr;
    }

    sockaddr_in addr(m_sockaddr);
    addr.sin_addr.s_addr &= byteswapOnLittleEndian(~CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr(new IPv4Address(addr));
}

IPAddress::ptr IPv4Address::subnetMask(uint32_t prefix_len)
{
    sockaddr_in subnet;
    bzero(&subnet,sizeof(sockaddr_in));
    subnet.sin_family = AF_INET;
    subnet.sin_addr.s_addr = ~byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr(new IPv4Address(subnet));
}
uint32_t IPv4Address::getPort() const
{
    return byteswapOnLittleEndian(m_sockaddr.sin_port);
}

void IPv4Address::setPort(uint16_t v)
{
    m_sockaddr.sin_port = byteswapOnLittleEndian(v);
}


/*-------------------------------------  IPv6Address  ---------------------------------------------*/
IPv6Address::ptr IPv6Address::Create(const char* address, uint16_t port)
{
    IPv6Address::ptr rt(new IPv6Address);
    rt->m_sockaddr.sin6_port = byteswapOnLittleEndian(port);
    int result = inet_pton(AF_INET6, address, &rt->m_sockaddr.sin6_addr);
    if (result <= 0) {
        SYLAR_LOG_DEBUG(g_logger) << "IPv6Address::Create(" << address << ", "
                                  << port << ") rt=" << result << " errno=" << errno
                                  << " errstr=" << strerror(errno);
        return nullptr;
    }
    return rt;
}

IPv6Address::IPv6Address()
{
    bzero(&m_sockaddr, sizeof(sockaddr_in6));
    m_sockaddr.sin6_family = AF_INET6;
}

IPv6Address::IPv6Address(const sockaddr_in6& address)
{
    bzero(&m_sockaddr, sizeof(sockaddr_in6));
    m_sockaddr = address;
}

IPv6Address::IPv6Address(const uint8_t address[16], uint16_t port )
{
    bzero(&m_sockaddr, sizeof(sockaddr_in6));
    m_sockaddr.sin6_family = AF_INET6;
    m_sockaddr.sin6_port = byteswapOnLittleEndian(port);
    memcpy(&m_sockaddr.sin6_addr.s6_addr, address, 16);
}

const sockaddr* IPv6Address::getAddr() const
{
    return (sockaddr*)&m_sockaddr;
}

sockaddr* IPv6Address::getAddr()
{
    return (sockaddr*)&m_sockaddr;
}

socklen_t IPv6Address::getAddrlen() const
{
    return sizeof(m_sockaddr);  
}

/**
 *  @example 2001:0db8:0000:0000:0000:0000:0000:0001
 *                      |
 *                      v
 *                [2001:db8::1]
 */
std::ostream& IPv6Address::insert(std::ostream& os) const
{
    // 首先输出[,防止和端口混淆
    os << '[';
    // 将 16 个 uint8_t ，替换成 8 个 uint16_t
    uint16_t* addr = (uint16_t*)(m_sockaddr.sin6_addr.s6_addr);
    // 根据IPv6的规定，连续压缩 0 只允许一次
    bool used_zeros = false;
    // 遍历地址addr
    for (size_t i = 0; i < 8; ++i)
    {
        // 如果当前 uint16_t 段的值为 0，则先跳过，为后续连续压缩 0 做准备
        if (addr[i] == 0 && !used_zeros)
        {
            continue;
        }
        // 如果前一个 uint16_t 段的值为 0， 且当前 uint16_t 段的值不为 0，就输出个 ':'，为压缩做准备
        if (i && addr[i-1] == 0 && !used_zeros)
        {
            os << ":";
            used_zeros = true;
        }
        // 再次输出冒号
        if (i)
        {
           os <<  ":";
        }
        // 输出当前 uint16_t段的16 进制的值
        os << std::hex << (int)byteswapOnLittleEndian(addr[i]) << std::dec;
    }
    // 这说明ipv6的地址全是0
    if (!used_zeros && addr[7] == 0) {
        os << "::";
    }

    os << "]:" << byteswapOnLittleEndian(m_sockaddr.sin6_port);
    return os;
}

IPAddress::ptr IPv6Address::broadcastAddress(uint32_t prefix_len)
{
    sockaddr_in6 addr(m_sockaddr);
    addr.sin6_addr.s6_addr[prefix_len / 8] |= CreateMask<uint8_t>(prefix_len % 8);
    for (int i = (prefix_len / 8) + 1; i < 16; i++)
    {
        addr.sin6_addr.s6_addr[i] = 0xff;
    }
    return IPv6Address::ptr(new IPv6Address(addr));
}

IPAddress::ptr IPv6Address::networkAddress(uint32_t prefix_len)
{
   sockaddr_in6 addr(m_sockaddr);
   addr.sin6_addr.s6_addr[prefix_len / 8] &= ~CreateMask<uint8_t>(prefix_len % 8);
   for (int i = prefix_len / 8 + 1; i < 16; ++i)
   {
        addr.sin6_addr.s6_addr[i] = 0x00;
   }
   return IPv6Address::ptr(new IPv6Address(addr));
}

IPAddress::ptr IPv6Address::subnetMask(uint32_t prefix_len)
{
    sockaddr_in6 addr;
    bzero(&addr,sizeof(sockaddr_in6));
    addr.sin6_addr.s6_addr[prefix_len / 8 ] = ~CreateMask<uint8_t>(prefix_len % 8);
    for (int i = 0; i < prefix_len / 8; i++)
    {
        addr.sin6_addr.s6_addr[i] = 0xff;
    }
    return IPv6Address::ptr(new IPv6Address(addr));
}

uint32_t IPv6Address::getPort() const 
{
    return byteswapOnLittleEndian(m_sockaddr.sin6_port);
}
    
void IPv6Address::setPort(uint16_t v) 
{
    m_sockaddr.sin6_port = byteswapOnLittleEndian(v);
}


/*-------------------------------------  UnixAddress  ---------------------------------------------*/
static const size_t MAX_PATH_LEN = sizeof(((sockaddr_un*)0)->sun_path) - 1;

UnixAddress::UnixAddress()
{
    bzero(&m_addr, sizeof(sockaddr_un));
    m_addr.sun_family = AF_UNIX;
    m_length = offsetof(sockaddr_un,sun_path) + MAX_PATH_LEN;
}

UnixAddress::UnixAddress(const std::string& path)
{
    bzero(&m_addr, sizeof(sockaddr_un));
    m_addr.sun_family = AF_UNIX;
    // +1 是因为 size 计算的长度不包括字符串的 '\0'
    m_length = path.size() + 1;
    /* 如果 path[0] == '\0'，说明这是一个 抽象套接字，不会出现在文件系统中
     * 抽象套接字的路径不是以 \0 结尾的字符串，而是以 \0 开头、其后是原始字节数组
     */ 
    if (!path.empty() && path[0] == '\0')
    {
        --m_length;
    }
    // 检查路径长度是否超出 sockaddr_un.sun_path 的最大长度（108 字节）
    if (m_length > sizeof(m_addr.sun_path))
    {
        throw std::logic_error("path too long");
    }
    // 将路径复制到m_addr.sun_path中
    memcpy(m_addr.sun_path, path.c_str(), m_length);
    // 得到实力路径的总长度
    m_length += offsetof(sockaddr_un,sun_path);
}

void UnixAddress::setAddrlen(uint32_t v) {
    m_length = v;
}

socklen_t UnixAddress::getAddrlen() const {
    return m_length;
}
sockaddr *UnixAddress::getAddr() {
    return (sockaddr *)&m_addr;
}

const sockaddr *UnixAddress::getAddr() const {
    return (sockaddr *)&m_addr;
}

std::string UnixAddress::getPath() const {
    std::stringstream ss;
    if (m_length > offsetof(sockaddr_un, sun_path) && m_addr.sun_path[0] == '\0') {
        ss << "\\0" << std::string(m_addr.sun_path + 1, m_length - offsetof(sockaddr_un, sun_path) - 1);
    } else {
        ss << m_addr.sun_path;
    }
    return ss.str();
}

std::ostream &UnixAddress::insert(std::ostream &os) const {
    if (m_length > offsetof(sockaddr_un, sun_path) && m_addr.sun_path[0] == '\0') {
        return os << "\\0" << std::string(m_addr.sun_path + 1, m_length - offsetof(sockaddr_un, sun_path) - 1);
    }
    return os << m_addr.sun_path;
}

/*-------------------------------------  UnkonwnAddress  -------------------------------------------*/
UnknownAddress::UnknownAddress(int family) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sa_family = family;
}

UnknownAddress::UnknownAddress(const sockaddr& addr) {
    m_addr = addr;
}

sockaddr* UnknownAddress::getAddr() {
    return (sockaddr*)&m_addr;
}

const sockaddr* UnknownAddress::getAddr() const {
    return &m_addr;
}

socklen_t UnknownAddress::getAddrlen() const {
    return sizeof(m_addr);
}

std::ostream& UnknownAddress::insert(std::ostream& os) const {
    os << "[UnknownAddress family=" << m_addr.sa_family << "]";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Address& addr) {
    return addr.insert(os);
}

}