#pragma once

#include <memory>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include "address.hpp"
#include "noncopyable.h"

namespace sylar
{

class Socket : public std::enable_shared_from_this<Socket>, Noncopyable
{
public:
    using ptr = std::shared_ptr<Socket>;
    using weak_ptr = std::weak_ptr<Socket>;

    /**
     *  @brief Socket类型 
     */
    enum Type
    {
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM,
    };

    /**
     *  @brief 协议类型
     */
    enum Family
    {
        IPv4 = AF_INET,
        IPv6 = AF_INET6,
        UNIX = AF_UNIX,
    };

    /**
     *  @brief 创建TcpSocket 
     */
    static Socket::ptr CreateTCP(Address::ptr address);

    /**
     *  @brief 创建UDPSocket 
     */
    static Socket::ptr CreateUDP(Address::ptr address);

    /**
     *  @brief 创建IPv4的TcpSocket 
     */
    static Socket::ptr CreateTCPSocket();

    /**
     *  @brief 创建IPv4的UDPSocket 
     */
    static Socket::ptr CreateUDPSocket();

    /**
     *  @brief 创建IPv6的TCPSocket 
     */
    static Socket::ptr CreateTCPSocket6();

    /**
     *  @brief 创建IPv6的UDPSocket 
     */
    static Socket::ptr CreateUDPSocket6();

     /**
     *  @brief 创建Unix的Tcp Socket 
     */
    static Socket::ptr CreateUnixTCPSocket();

    /**
     *  @brief 创建Unix的UDP Socket 
     */
    static Socket::ptr CreateUnixUDPSocket();

    /**
     *  @brief 构造函数
     *  @param[in] domain   协议簇
     *  @param[in] type     类型
     *  @param[in] protocal 协议
     */
    Socket(int domain, int type, int protocol = 0);

    /**
     *  @brief 析构函数 
     */
    virtual ~Socket();

    /**
     *  @brief 获得发送超时时间 
     */
    int64_t getSendTimeout();

    /**
     *  @brief 设置发送超时时间 
     */
    void setSendTimeout(int64_t timeout_ms);

    /**
     *  @brief 获取接收超时时间 
     */
    int64_t getRecvTimeout();

    /**
     *  @brief 设置接收超时时间 
     */
    void setRecvTimeout(int64_t timeout_ms);

    /**
     *  @brief 获取sockopt 
     */
    bool getOption(int level, int option, void* result, socklen_t* len);

    /**
     *  @brief 获取sockopt模板 
     */
    template<class T>
    bool getOption(int level, int option, T& result)
    {
        socklen_t len = sizeof(T);
        return getOption(level, option, &result, &len);
    }

    /**
     *  @brief 设置sockopt 
     */
    bool setOption(int level, int option, void* result, socklen_t len);

    /**
     *  @brief sockopt模板类 
     */
    template<class T>
    bool setOption(int level, int option, T& result)
    {
        socklen_t len = sizeof(T);
        return setOption(level, option, &result, len);
    }

    /**
     *  @brief 接收accept连接
     *  @return 成功后返回新的Socket::ptr，失败后返回nullptr
     *  @details accept的前提是：bind、listen成功 
     */
    virtual Socket::ptr accept();

    /**
     *  @brief 绑定地址
     *  @param[in] addr 地址
     *  @return 是否绑定成功 
     */
    virtual bool bind(const Address::ptr addr);

    /**
     *  @brief 连接地址
     *  @param[in] addr 地址
     *  @param[in] timeout_ms 超时时间 
     */ 
    virtual bool connect(const Address::ptr addr, uint64_t timeout_ms = -1);

    virtual bool reconnect(uint64_t timeout_ms = -1);

    /**
     *  @brief 监听socket
     *  @param[in] 未完成连接队列的最大长度
     *  @return 是否监听成功
     *  @details 必须先bind成功 
     */
    virtual bool listen(int backlog = SOMAXCONN);

    /**
     *  @brief 关闭socket 
     */
    virtual bool close();

    /**
     *  @brief 发送数据
     *  @param[in] buffer 待发送的数据
     *  @param[in] length 待发送数据的长度
     *  @param[in] flags  标志位 
     */
    virtual ssize_t send(const void* buffer, size_t length, int flags = 0);

    /**
     *  @brief 发送数据
     *  @param[in] buffers 发送送数据的内存(iovec数组)
     *  @param[in] length 待发送数据的长度(iovec长度)
     *  @param[in] flags 标志字段
     */ 
    virtual ssize_t send(const iovec* buffer, size_t length, int flags = 0);

    /**
     *  @brief 发送数据
     *  @param[in] buffer 待发送的数据
     *  @param[in] length 待发送数据的长度
     *  @param[in] to 发送的目的地址
     *  @param[in] flags  标志位 
     */
    virtual ssize_t sendto(const void* buffer, size_t length, const Address::ptr to,int flags = 0);

    /**
     *  @brief 发送数据
     *  @param[in] buffer 待发送的数据
     *  @param[in] length 待发送数据的长度
     *  @param[in] to 发送的目的地址
     *  @param[in] flags  标志位 
     */
    virtual ssize_t sendto(const iovec* buffer, size_t length, const Address::ptr to,int flags = 0);

    /**
     *  @brief 接受数据
     *  @param[in] buffer 待接收的数据
     *  @param[in] length 待接收的数据的长度
     *  @param[in] flags  标志字段 
     */
    virtual ssize_t recv(void* buf, size_t length, int flags = 0);

    /**
     *  @brief 接受数据
     *  @param[in] buffer 待接收的数据
     *  @param[in] length 待接收的数据的长度
     *  @param[in] flags  标志字段 
     */
    virtual ssize_t recv(iovec* buf, size_t length, int flags = 0);

     /**
     *  @brief 接受数据
     *  @param[in] buffer 待接收的数据
     *  @param[in] length 待接收的数据的长度
     *  @param[in] from   发送方的地址
     *  @param[in] flags  标志字段 
     */
    virtual ssize_t recvfrom(void* buf, size_t length, const Address::ptr from, int flags = 0);

    /**
     *  @brief 接收数据
     *  @param[in] buffer 待接收的数据
     *  @param[in] length 待接收数据的长度
     *  @param[in] from   发送方的地址
     *  @param[in] flags  标志位
     */
    virtual ssize_t recvfrom(iovec* buffer, size_t length, const Address::ptr from, int flags = 0);

    /**
     *  @brief 获取远端地址 
     */
    Address::ptr getRemoteAddress();

    /**
     *  @brief 获得本机地址 
     */
    Address::ptr getLocalAddress();

    /**
     *  @brief 返回文件句柄 
     */
    int getSock() const { return m_sock; }

    /**
     *  @brief 获得协议族 
     */
    int getFamily() const { return m_family;}

    /**
     *  @brief 获得类型 
     */
    int getType() const { return m_type; }

    /**
     *  @brief 获得协议 
     */
    int getProtocol() const { return m_protocol; }

    /**
     *  @brief 返回是否连接成功 
     */
    bool isConnected() const { return m_isConnected; }

    /**
     *  @brief 判断socket是否有效 
     */
    bool isVaild() const ;

    /**
     *  @brief 返回socket错误 
     */
    int getError();

    /**
     *  @brief 输出信息到流中 
     */
    virtual std::ostream& dump(std::ostream& os) const;

    virtual std::string toString() const ;

    /**
     *  @brief 取消读 
     */
    bool cancelRead();

    /**
     *  @brief 取消写 
     */
    bool cancelWrite();

    /**
     *  @brief 取消accept 
     */
    bool cancelAccept();

    /**
     * @brief 取消所有事件
     */
    bool cancelAll();

protected:
    /**
     *  @brief 初始化socket 
     */
    void initSock();

    /**
     *  @brief 创建Socket 
     */
    void newSock();

    /**
     *  @brief 初始化sock 
     */
    virtual bool init(int sock);

protected:
    int m_sock;         // socket文件句柄
    int m_family;       // 协议簇
    int m_type;         // 类型
    int m_protocol;     // 协议
    bool m_isConnected;   // 是否连接
    Address::ptr m_localAddress;    // 本地地址
    Address::ptr m_remoteAddress;   //  远端地址
};

/**
 * @brief 流式输出socket
 * @param[in, out] os 输出流
 * @param[in] sock Socket类
 */
std::ostream &operator<<(std::ostream &os, const Socket &sock);

}