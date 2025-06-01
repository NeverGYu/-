#pragma once 

#include <memory>
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <vector>
#include <map>
#include <netinet/in.h>
#include <sys/un.h>

namespace sylar
{

class IPAddress;

/**
 *  @brief 网络地址的基类(抽象基类) 
 */
class Address
{
public:
    using ptr = std::shared_ptr<Address>;
    
    /**
     *  @brief 根据sockaddr创建Address
     *  @param[in] addr sockadrr指针
     *  @param[in] addrlen sockaddr的长度
     *  @return 返回和sockaddr相匹配的Address，失败返回nullptr
     */
    static Address::ptr Create(const sockaddr* addr, socklen_t addrlen);

    /**
     *  @brief 根据host地址返回对应条件的所有Address
     *  @param[out] result 保存所有满足条件的Address
     *  @param[in]  host 域名、服务器名等等-->举例：www.sylar.top / [::1]:80
     *  @param[in]  family 地址协议族
     *  @param[in]  type socket类型：Tcp(SOCK_STREAM)、UDP(SOCK_DGRAM)
     *  @param[in]  protocol 协议：IPPORTO_TCP、IPPORTO_UDP
     *  @return 返回是否转换成功
     */
    bool static Lookup(std::vector<Address::ptr>& result, const std::string& host, int family = AF_INET, int type = 0, int protocal = 0);

    /**
     *  @brief 根据host地址返回对应调节的任意Address
     *  @param[out] result 保存所有满足条件的Address
     *  @param[in]  host 域名、服务器名等等-->举例：www.sylar.top
     *  @param[in]  family 地址协议族
     *  @param[in]  type socket类型：Tcp(SOCK_STREAM)、UDP(SOCK_DGRAM)
     *  @param[in]  protocol 协议：IPPORTO_TCP、IPPORTO_UDP
     *  @return 成功返回Address，失败返回nullptr
     */
    Address::ptr static LookupAny(const std::string& host,  int family = AF_INET, int type = 0, int protocal = 0);

     /**
     *  @brief 根据host地址返回对应调节的任意IPAddress
     *  @param[out] result 保存所有满足条件的Address
     *  @param[in]  host 域名、服务器名等等-->举例：www.sylar.top
     *  @param[in]  family 地址协议族
     *  @param[in]  type socket类型：Tcp(SOCK_STREAM)、UDP(SOCK_DGRAM)
     *  @param[in]  protocol 协议：IPPORTO_TCP、IPPORTO_UDP
     *  @return 成功返回Address，失败返回nullptr
     */
    std::shared_ptr<IPAddress> static LookupAnyIPAddress(const std::string& host,  int family = AF_INET, int type = 0, int protocol = 0);
    
    /**
     *  @brief 返回本机所有网卡的<网卡名，地址，子网掩码位数>
     *  @param[out] result 存放本机所有的地址
     *  @param[in]  family 地址协议族
     *  @return 是否返回成功
     */
    static bool GetInterfaceAddresses(std::multimap<std::string, std::pair<Address::ptr, uint32_t>>& result, int family = AF_INET);

    /**
     *  @brief 获取指定网卡的地址和子网掩码
     *  @param[out] 保存执行网卡的所有地址
     *  @param[in] iface 网卡名称
     *  @param[in] family 协议族
     *  @return 是否获取成功 
     */
    static bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t>>& result, const std::string& iface, int family = AF_INET);

    /**
     *  @brief 虚析构函数 
     */
    virtual ~Address() {}

    /**
     *  @brief 返回地址协议族 
     */
    int getFamily() const;

    /**
     *   @brief 返回sockaddr指针(可读不可写)
     */
    virtual const sockaddr* getAddr() const = 0;

    /**
     *  @brief 返回sockaddr的指针(可读写) 
     */
    virtual sockaddr* getAddr() = 0;

    /**
     *  @brief 返回sockaddr的长度 
     */
    virtual socklen_t getAddrlen() const = 0;

    /**
     *  @brief 可读性输出地址 
     */
    virtual std::ostream& insert(std::ostream& os) const = 0;

    /**
     *  @brief 返回可读性字符串 
     */
    std::string toString() const;

    /**
     *  @brief 地址比较函数 
     */
    bool operator<(const Address& rhs) const;

    bool operator==(const Address& rhs) const;

    bool operator!=(const Address& rhs) const;
};


/**
 *  @brief IP地址的基类(抽象基类) 
 */
class IPAddress : public Address
{
public:
    using ptr = std::shared_ptr<IPAddress>;

    /**
     *  @brief 通过域名、ip、服务器名创建IPAddress ]
     *  @param[in] address 域名、ip、服务器名
     *  @param[in] port 端口号
     *  @return 成功返回IPAddress，失败返回nullptr 
     */
    static IPAddress::ptr Create(const char* address, uint16_t port);

    /**
     *  @brief 获得该地址的广播地址
     *  @param[in] prefix_len 网络号的位数
     *  @return 成功返回IPAddress，失败返回nullptr 
     */
    virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;

     /**
     *  @brief 获得该地址的网络地址
     *  @param[in] prefix_len 网络号的位数
     *  @return 成功返回IPAddress，失败返回nullptr 
     */
    virtual IPAddress::ptr networkAddress(uint32_t prefix_len) = 0;

    /**
     *  @brief 获得子网掩码的位数
     *  @param[in] prefix_len 子网掩码的位数
     *  @return  成功返回IPAddress，失败返回nullptr 
     */
    virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;

    /**
     *  @brief 返回端口号 
     */
    virtual uint32_t getPort() const = 0;

    /**
     *  @brief 设置端口号 
     */
    virtual void setPort(uint16_t val) = 0;
private:
};


/**
 *  @brief IPv4地址 
 */
class IPv4Address : public IPAddress
{
public:
    using ptr = std::shared_ptr<IPv4Address>;

    /**
     *  @brief 使用点分十进制创建IPv4地址
     *  @param[in] address 点分十进制地址：192.168.1.1
     *  @param[in] port 端口号
     *  @return 成功返回IPv4，失败返回nullptr 
     */
    static IPv4Address::ptr Create(const char* addresss, uint16_t port = 0);

    /**
     *  @brief 通过sockaddr_in 构造IPv4地址
     *  @param[in] address sockaddr_in 
     */
    IPv4Address(const sockaddr_in& addr);

    /**
     *  @brief 使用二进制构造IPv4地址
     *  @param[in] address 二进制地址
     *  @param[in] port 端口号  
     */
    IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);

    /**
     *  @brief 下列方法属于基类Address 
     */
    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrlen() const override;
    std::ostream& insert(std::ostream& os) const override;

    /**
     *  @brief 下列方法属于基类IPAddress 
     */
    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networkAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;
    uint32_t getPort() const override;
    void setPort(uint16_t v) override;
    
private:
    sockaddr_in m_sockaddr;
};


/**
 *  @brief IPv6地址 
 */
class IPv6Address : public IPAddress
{
public:
    using ptr = std::shared_ptr<IPv6Address>;

    /**
     *  @brief 通过IPv6地址字符串构造IPv6Address
     *  @param[in] address IPv6地址字符串
     *  @param[in] port 端口号
     */
    static IPv6Address::ptr Create(const char* address, uint16_t port = 0);

    /**
     *  @brief 无参构造函数 
     */
    IPv6Address();

    /**
     *  @brief 通过sockaddr_in6 来构造IPv6地址
     *  @param[in] address sockaddr_in6结构体  
     */
    IPv6Address(const sockaddr_in6& address);

    /**
     *  @brief 通过IPv6二进制来构造IPv6Address
     *  @param[in] address IPv6二进制地址 
     */
    IPv6Address(const uint8_t address[16], uint16_t port = 0);

    /**
     *  @brief 下列方法属于基类Address 
     */
    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrlen() const override;
    std::ostream& insert(std::ostream& os) const override;

    /**
     *  @brief 下列方法属于基类IPAddress 
     */
    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networkAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;
    uint32_t getPort() const override;
    void setPort(uint16_t v) override;

private:
    sockaddr_in6 m_sockaddr;
};

/**
 *  @brief Unix Socket地址 
 */
class UnixAddress : public Address
{
public:
    using ptr = std::shared_ptr<UnixAddress>;

    /**
     *  @brief 无参构造函数 
     */
    UnixAddress();

    /**
     *  @brief 通过路径来构造UnixAddress
     *  @param[in]  path UnixSocket路径 
     */
    UnixAddress(const std::string& path);

     /**
     *  @brief 下列方法属于基类Address 
     */
    const sockaddr *getAddr() const override;
    sockaddr *getAddr() override;
    socklen_t getAddrlen() const override;
    std::ostream& insert(std::ostream& os) const override;


    void setAddrlen(uint32_t v);
    std::string getPath() const;
private:
    sockaddr_un m_addr;
    socklen_t m_length;    
};


/**
 *  @brief 未知地址 
 */
class UnknownAddress : public Address
{
public:
    typedef std::shared_ptr<UnknownAddress> ptr;

    /**
     *  @brief 构造函数 
     *  @param[in]  family 地址协议
     */
    UnknownAddress(int family);

    /**
     *  @brief 构造函数
     *  @param[in] addr sockaddr结构体 
     */
    UnknownAddress(const sockaddr &addr);

    /**
     *  @brief 下列方法属于基类Address 
     */
    const sockaddr *getAddr() const override;
    sockaddr *getAddr() override;
    socklen_t getAddrlen() const override;
    std::ostream &insert(std::ostream &os) const override;
private:
    sockaddr m_addr;
};

/**
 *  @brief 流式输出Address 
 */
std::ostream& operator<<(std::ostream& os, const Address& address);
}
