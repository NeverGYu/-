#pragma once 

#include <memory>
#include "http_session.h"
#include "session_manager.h"
#include "http_servlet.h"
#include "../base/tcp_server.h"
#include "../middleware/middleware.h"
#include "../middleware/cors/CorsMiddleware.h"
#include "../ssl/ssl_context.h" 

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
               , sylar::IOManager* accept_worker = sylar::IOManager::GetThis()
               , bool use_ssl = true
               , sylar::ssl::SslContext::ptr ssl_ctx = nullptr);
    
    /**
     *  @brief 获得ServletDispatch 
     */
    ServletDispatch::ptr getServletDispatch() const { return m_dispatch; }

    /**
     *  @brief 设置ServletDispatch
     *  @param[in] v  
     */
    void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = v; }
    
    /**
     *  @brief 添加中间件
     *  @param[in] middleware 中间件
     */
    void addMiddleware(middleware::MiddleWare::ptr middleware);

    /**
     *  @brief 添加ssl
     *  @param[in] ctx SSL上下文  
     */
    void setSsl(bool enabled, sylar::ssl::SslContext::ptr ctx = nullptr);

protected:
    virtual void handleClient(Socket::ptr client) override;
    
private:
    bool m_isKeepalive;                          // 是否支持长连接
    bool m_useSsl;                               // 是否使用SSL
    ssl::SslContext::ptr m_ctx;                       // SSL上下文
    ServletDispatch::ptr m_dispatch;             // 用于返回对应的Servlet
    SessionManager::ptr m_sessionManager;        // 用于管理连接的会话
    middleware::MiddleChain::ptr m_middleChain;  // 中间件调用链
};


}
}