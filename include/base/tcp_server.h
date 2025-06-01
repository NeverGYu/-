#pragma once

#include <memory>
#include <functional>
#include <vector>
#include <string>
#include "noncopyable.h"
#include "iomanager.h"
#include "socket.hpp"
#include "address.hpp"

namespace sylar
{

class TcpServer : public Noncopyable
                , public std::enable_shared_from_this<TcpServer>
{
public:
    using ptr = std::shared_ptr<TcpServer>;

    /**
     *  @brief  构造函数
     *  @param[in]  ioworker 工作调度器
     *  @param[in]  accept 接收调度器
     */
    TcpServer(IOManager* ioworker = IOManager::GetThis(), IOManager* accept = IOManager::GetThis());

    /**
     *  @brief 析构函数 
     */
    virtual ~TcpServer();

    /**
     *  @brief 绑定地址 
     */
    virtual bool bind(Address::ptr address);

    /**
     *  @brief 绑定地址数组
     *  @param[in] addrs 绑定成功的地址集合
     *  @param[in] fails 绑定失败的地址集合
     */
    virtual bool bind(std::vector<Address::ptr>& addrs, std::vector<Address::ptr>& fails);

    /**
     *  @brief 启动服务 
     */
    virtual bool start();

    /**
     *  @brief 启动服务 
     */
    virtual void stop();

    /**
     *  @brief 返回接收超时时间 
     */
    uint64_t getRecvTimeout() const { return m_recvtimeout; }

    /**
     *  @brief 设置接收超时时间 
     */
    void setRecvTimeout(uint64_t v) { m_recvtimeout = v; }
    
    /**
     *  @brief 返回服务器的名称 
     */
    std::string getName() const { return m_name; }

     /**
     * @brief 设置服务器名称
     */
    virtual void setName(const std::string& v) { m_name = v;}

    /**
     *  @brief 判断服务器是否停止 
     */
    bool isStop() const { return m_isStop; }

     /**
     * @brief 以字符串形式dump server信息
     */
    virtual std::string toString(const std::string& prefix = "");

protected:
    /**
     *  @brief 处理新连接的Socket 
     */
    virtual void handleClient(Socket::ptr client);

    /**
     *  @brief 开始接收连接 
     */
    virtual void startAccept(Socket::ptr sock);
private:
    std::vector<Socket::ptr> m_socks;   // 用于保存连接的socket集合
    IOManager* m_acceptworker;          // 处理连接请求的调度器
    IOManager* m_ioworker;              // 处理新连接工作的调度器
    uint64_t m_recvtimeout;             // 接收超时时间
    std::string m_name;                 // 服务器的名称
    std::string m_type;                 // 服务器的类型
    bool m_isStop;                      // 服务器是否停止
};
}