#pragma once

#include <memory>
#include <openssl/ssl.h>
#include "socket_stream.hpp"
#include "../ssl/ssl_context.h"

namespace sylar{
namespace ssl{

class SslSocketStream : public SocketStream
{
public:
    using ptr = std::shared_ptr<SslSocketStream>;

    /**
     *  @brief 构造函数
     *  @param[in] sock 用来和客户端通信的socket
     *  @param[in] ctx  保存Ssl相关的信息 
     */
    SslSocketStream(sylar::Socket::ptr sock, sylar::ssl::SslContext::ptr ctx);

    /**
     *  @brief 析构函数 
     */
    ~SslSocketStream();

    /**
     *  @brief 判断是否建立了连接 
     */
    bool isConnected() const;

    /**
     *  @brief 进行SSL 握手 
     */
    bool doHandShake();

    /**
     *  @brief 关闭socket
     */
    virtual void close() override;

    /**
     * @brief 读取数据
     * @param[out] buffer 待接收数据的内存
     * @param[in] length 待接收数据的内存长度
     * @return
     *      @retval >0 返回实际接收到的数据长度
     *      @retval =0 socket被远端关闭
     *      @retval <0 socket错误
     */
    virtual int read(void* buffer, size_t length) override;

    /**
     * @brief 写入数据
     * @param[in] buffer 待发送数据的内存
     * @param[in] length 待发送数据的内存长度
     * @return
     *      @retval >0 返回实际接收到的数据长度
     *      @retval =0 socket被远端关闭
     *      @retval <0 socket错误
     */
    virtual int write(const void* buffer, size_t length) override;

private:
    SSL* m_ssl;                 // SSL 连接
    SslContext::ptr m_ctx;      // SSL 上下文
    bool m_handshake = false;   // 标志位，判断是否握手成功
};


}
}