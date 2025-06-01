#include "sylar.h"
#include <byteswap.h>
#include <bitset>
#include <iostream>
#include <map>
#include <iomanip> 

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

/**
 *  @brief 测试字节序转换的用法 
 */
void test_byteswap()
{
    uint64_t value = 8;
    uint64_t value2 = sylar::byteswapOnLittleEndian(value);
    std::cout << std::bitset<64>(value2) << std::endl;
}

/**
 *  @brief 将协议转换成字符串 
 */
const char* familytoString(int family)
{
    switch (family)
    {
#define XX(name) case(name): return #name;
        XX(AF_INET);
        XX(AF_INET6)
        XX(AF_UNIX)
        XX(AF_UNSPEC)
#undef XX
    default:
        return "UNKNOWN";
    }
}

/**
 *  @brief 测试IPv4 
 */
void test_ipv4()
{
    SYLAR_LOG_INFO(g_logger) << "test_ipv4 ";
    auto addr = sylar::IPv4Address::Create("192.168.1.1");
    if (!addr) {
        SYLAR_LOG_ERROR(g_logger) << "IPAddress::Create error";
        return;
    }
    SYLAR_LOG_INFO(g_logger) << "addr: " << addr->toString();
    SYLAR_LOG_INFO(g_logger) << "addrlen: "<< addr->getAddrlen();
    SYLAR_LOG_INFO(g_logger) << "port: "  << addr->getPort();
    SYLAR_LOG_INFO(g_logger) << "family: " << familytoString(addr->getFamily());

    SYLAR_LOG_INFO(g_logger) << "broadcast addr: " << addr->broadcastAddress(24)->toString();
    SYLAR_LOG_INFO(g_logger) << "networkr add: " << addr->networkAddress(24)->toString();
    SYLAR_LOG_INFO(g_logger) << "subnetMask: "<< addr->subnetMask(24)->toString();
    SYLAR_LOG_INFO(g_logger) << "\n";
}

/**
 *  @brief 测试ipv6 
 */
void test_ipv6()
{
    SYLAR_LOG_INFO(g_logger) << "test ipv6";
    auto addr = sylar::IPv6Address::Create("fe80::215:5dff:fe88:d8a");
    if (!addr) {
        SYLAR_LOG_ERROR(g_logger) << "IPAddress::Create error";
        return;
    }
    SYLAR_LOG_INFO(g_logger) << "addr: " << addr->toString();
    SYLAR_LOG_INFO(g_logger) << "port: " << addr->getPort();
    SYLAR_LOG_INFO(g_logger) << "addrlen: " << addr->getAddrlen();
    SYLAR_LOG_INFO(g_logger) << "family: " << familytoString(addr->getFamily());

    SYLAR_LOG_INFO(g_logger) << "broadcast addr: " << addr->broadcastAddress(64)->toString();
    SYLAR_LOG_INFO(g_logger) << "network addr: " << addr->networkAddress(64)->toString();
    SYLAR_LOG_INFO(g_logger) << "subnetmask addr: " << addr->subnetMask(64)->toString();

    SYLAR_LOG_INFO(g_logger) << "\n";

}

/**
 *  @brief 测试unix 
 */
void test_unix()
{
    SYLAR_LOG_INFO(g_logger) << "test_unix";
    auto addr = sylar::UnixAddress("/tmp/test_unix.sock");
    SYLAR_LOG_INFO(g_logger) << "addr: " << addr.getAddr();
    SYLAR_LOG_INFO(g_logger) << "path: " << addr.getPath();
    SYLAR_LOG_INFO(g_logger) << "addrlen: " << addr.getAddrlen();
    SYLAR_LOG_INFO(g_logger) << "family: " << familytoString(addr.getFamily());

    SYLAR_LOG_INFO(g_logger) << "\n";
} 

/**
 *  @brief 查询所有网卡 
 */
void test_ifaces(int family)
{
    SYLAR_LOG_INFO(g_logger) << "test_ifaces: " << familytoString(family);

    std::multimap<std::string, std::pair<sylar::Address::ptr, uint32_t>> results;
    bool val = sylar::Address::GetInterfaceAddresses(results, family);
    if (!val)
    {
        SYLAR_LOG_ERROR(g_logger) << "GetInterfaceAddresses fail";
        return;
    }
    for (auto& i : results)
    {
        SYLAR_LOG_INFO(g_logger) << i.first 
                                 << " - " << i.second.first->toString() 
                                 << " - " << i.second.second;
    }
    
    SYLAR_LOG_INFO(g_logger) << "\n";
}

/**
 *  @brief 查询指定网卡 
 */
void test_iface(const char* iface, int family)
{
    SYLAR_LOG_INFO(g_logger) << "test_iface: " << iface << ", " << familytoString(family);

    std::vector<std::pair<sylar::Address::ptr, uint32_t>> result;
    bool val = sylar::Address::GetInterfaceAddresses(result, iface, family);

    if(!val) 
    {
        SYLAR_LOG_ERROR(g_logger) << "GetInterfaceAddresses fail";
        return;
    }

    for(auto &i : result)
    {
        SYLAR_LOG_INFO(g_logger) << i.first->toString() << " - " << i.second;
    }

    SYLAR_LOG_INFO(g_logger) << "\n";
}

/**
 *  @brief 测试域名解析服务 
 */
void test_lookup(const char* host)
{
    SYLAR_LOG_INFO(g_logger) << "test_lookup: " << host;

    SYLAR_LOG_INFO(g_logger) << "Lookup: ";
    std::vector<sylar::Address::ptr> results;
    bool v  = sylar::Address::Lookup(results, host, AF_INET);
    if (!v)
    {
        SYLAR_LOG_INFO(g_logger) << "Lookup fail";
        return ;
    }
    for (auto& i: results)
    {
        SYLAR_LOG_INFO(g_logger) << i->toString();
    }
    
    SYLAR_LOG_INFO(g_logger) <<"LookupAny:";
    auto addr2 = sylar::Address::LookupAny(host);
    SYLAR_LOG_INFO(g_logger) << addr2->toString();

    SYLAR_LOG_INFO(g_logger) <<"LookupAnyIPAddress:";
    auto addr1 = sylar::Address::LookupAnyIPAddress(host);
    SYLAR_LOG_INFO(g_logger) << addr1->toString();

    SYLAR_LOG_INFO(g_logger) << "\n";
}

int main(int argc, char**argv)
{
    // test_byteswap();

    // 测试ipv4
    test_ipv4();

    // 测试ipv6
    test_ipv6();

    // 测试unix
    test_unix();

    // 测试获取本机所有网卡的IPv4地址和IPv6地址以及掩码长度功能
    test_ifaces(AF_INET);
    test_ifaces(AF_INET6);

     // 获取本机eth0网卡的IPv4地址和IPv6地址以及掩码长度
    test_iface("ens33", AF_INET);
    test_iface("ens33", AF_INET6); 

    // ip域名解析服务
    test_lookup("127.0.0.1");
    test_lookup("127.0.0.1:80");
    test_lookup("127.0.0.1:http");
    test_lookup("127.0.0.1:ftp");
    test_lookup("localhost");
    test_lookup("localhost:80");
    test_lookup("www.baidu.com");
    test_lookup("www.baidu.com:80");
    test_lookup("www.baidu.com:http");
}