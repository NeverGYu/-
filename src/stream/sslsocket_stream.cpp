#include "../../stream/sslsocket_stream.hpp"
#include "../../base/log.h"
#include <openssl/err.h>

namespace sylar{
namespace ssl{

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

SslSocketStream::SslSocketStream(sylar::Socket::ptr sock, sylar::ssl::SslContext::ptr ctx)
    : SocketStream(sock)
    , m_ctx(ctx)
{
    // 根据SSL上下文创建SSL对象
    m_ssl = SSL_new(m_ctx->getNativeHandle());
    // 判断是否创建成功
    if (!m_ssl)
    {
        SYLAR_LOG_ERROR(g_logger) << "SSL_new failed";
        return;
    }
    // 将 socket 与 SSL对象绑定
    SSL_set_fd(m_ssl, sock->getSock());
    // 设置服务器模式
    SSL_set_accept_state(m_ssl);
}

SslSocketStream::~SslSocketStream()
{
    if (m_ssl)
    {
        SSL_shutdown(m_ssl);
        SSL_free(m_ssl);
        m_ssl = nullptr;
    }
}

bool SslSocketStream::isConnected() const
{
    return m_socket->isConnected() && m_handshake;
}

bool SslSocketStream::doHandShake()
{
    // 判断是否建立SSL连接
    if (m_handshake)
    {
        SYLAR_LOG_INFO(g_logger) << "SSL is already connected";
        return true;
    }
    // 接收客户端发来的SSL请求
    int rt = SSL_do_handshake(m_ssl);
    // 判断是否连接成功
    if (rt == 1)
    {
        m_handshake = true;
        SYLAR_LOG_INFO(g_logger) << "SSL handshake success. Cipher: "
                                << SSL_get_cipher(m_ssl) << ", Version: "
                                << SSL_get_version(m_ssl);
        return m_handshake;
    }
    // 获取错误码 
    int err = SSL_get_error(m_ssl, rt);
    // 解析错误码
    if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
        // 需要重试
        return false;
    }
    else
     {
        char err_buf[256];
        ERR_error_string_n(ERR_get_error(), err_buf, sizeof(err_buf));
        SYLAR_LOG_ERROR(g_logger) << "SSL handshake failed: " << err_buf;
        return false;
    }
    
}

int SslSocketStream::read(void* buffer, size_t length) {
    if (!m_handshake) 
    {
        if (!doHandShake()) 
        {
            return -1;
        }
    }
    int rt = SSL_read(m_ssl, buffer, length);
    SYLAR_LOG_DEBUG(g_logger) << "SSL_read return = " << rt;
    if (rt <= 0) 
    {
        int err = SSL_get_error(m_ssl, rt);
        if (err == SSL_ERROR_ZERO_RETURN) 
        {
            return 0; // SSL connection closed
        }
        char err_buf[256];
        ERR_error_string_n(ERR_get_error(), err_buf, sizeof(err_buf));
        SYLAR_LOG_ERROR(g_logger) << "SSL_read failed: " << err_buf;
    }
    std::string s((char*)buffer, rt);
    SYLAR_LOG_DEBUG(g_logger) << "SSL_read content (hex): " << std::hex << s;
    return rt;
}

int SslSocketStream::write(const void* buffer, size_t length) 
{
    if (!m_handshake) 
    {
        if (!doHandShake())
        {
            return -1;
        }
    }

    int rt = SSL_write(m_ssl, buffer, length);
    if (rt <= 0) 
    {
        int err = SSL_get_error(m_ssl, rt);
        char err_buf[256];
        ERR_error_string_n(ERR_get_error(), err_buf, sizeof(err_buf));
        SYLAR_LOG_ERROR(g_logger) << "SSL_write failed: " << err_buf;
    }
    return rt;
}

void SslSocketStream::close() 
{
    if (m_ssl) {
        SSL_shutdown(m_ssl);
        SSL_free(m_ssl);
        m_ssl = nullptr;
    }
    if (m_socket) 
    {
        m_socket->close();
    }
}


}
}
