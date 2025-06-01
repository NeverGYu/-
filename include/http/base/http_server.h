#pragma once 

#include <memory>
#include "http_session.h"
#include "http_servlet.h"
#include "../base/tcp_server.h"

namespace sylar{
namespace http{

/**
 *  @brief Http服务器类 
 */
class HttpServer : public TcpServer
{
public:
    using ptr = std::shared_ptr<HttpServer>;

    /**
     *  @brief 构造函数 
     */
    HttpServer(bool keepalive = true
               , sylar::IOManager* worker = sylar::IOManager::GetThis()
               , sylar::IOManager* accept_worker = sylar::IOManager::GetThis());
    
    /**
     *  @brief 获得ServletDispatch 
     */
    ServletDispatch::ptr getServletDispatch() const { return m_dispatch; }

    /**
     *  @brief 设置ServletDispatch 
     */
    void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = v; }
    
protected:
    virtual void handleClient(Socket::ptr client) override;
    
private:
    bool m_isKeepalive;             // 是否支持长连接
    ServletDispatch::ptr m_dispatch; // 用于返回
};

}
}