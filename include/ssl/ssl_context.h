#pragma once

#include "ssl_config.h"
#include "../base/noncopyable.h"
#include <openssl/ssl.h>

namespace sylar{
namespace ssl{

class SslContext : Noncopyable
{
public:
    using ptr = std::shared_ptr<SslContext>;

    /**
     *  @brief 构造函数 
     */
    explicit SslContext(const SslConfig& config);

    /**
     *  @brief 析构函数 
     */
    ~SslContext();

    /**
     *  @brief 初始化 
     */
    bool initilaize();

    /**
     *   @brief 返回OpenSSL指针，用于执行底层操作
     */
    SSL_CTX* getNativeHandle() const { return m_ctx; }

private:
    /**
     *  @brief 加载证书 
     */
    bool loadCertificates();

    /**
     *  @brief 建立协议 
     */
    bool setupProtocol();

    /**
     *  @brief 设置会话缓存 
     */
    void setupSessionCache();

    /**
     *  @brief 检查句柄是否发生出错误 
     */
    static void handleSslError(const char* msg);

private:
    SSL_CTX* m_ctx;         // SSL上下文
    SslConfig m_config;     // SSL配置
};

}
}