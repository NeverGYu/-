#pragma once

#include <memory>
#include "../base/stream.hpp"
#include "../base/socket.hpp"
#include "../base/mutex.h"
#include "../base/iomanager.h"

namespace sylar
{

/**
 *  @brief Socket 流
 */
class SocketStream : public Stream
{
public:
    using ptr = std::shared_ptr<SocketStream>;

    /**
     *  @brief 构造函数 
     */
    SocketStream(Socket::ptr sock, bool owner = true);

    /**
     *  @brief 析构函数 
     */
    ~SocketStream();

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
     * @brief 读取数据
     * @param[out] ba 接收数据的ByteArray
     * @param[in] length 待接收数据的内存长度
     * @return
     *      @retval >0 返回实际接收到的数据长度
     *      @retval =0 socket被远端关闭
     *      @retval <0 socket错误
     */
    virtual int read(ByteArray::ptr ba, size_t length) override;

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

    /**
     * @brief 写入数据
     * @param[in] ba 待发送数据的ByteArray
     * @param[in] length 待发送数据的内存长度
     * @return
     *      @retval >0 返回实际接收到的数据长度
     *      @retval =0 socket被远端关闭
     *      @retval <0 socket错误
     */
    virtual int write(ByteArray::ptr ba, size_t length) override;

    /**
     * @brief 关闭socket
     */
    virtual void close() override;

    /**
     * @brief 返回Socket类
     */
    Socket::ptr getSocket() const { return m_socket;}

    /**
     * @brief 返回是否连接
     */
    bool isConnected() const;

    Address::ptr getRemoteAddress();
    Address::ptr getLocalAddress();
    std::string getRemoteAddressString();
    std::string getLocalAddressString();
private:
    Socket::ptr m_socket;   // socket
    bool m_owner;           // 是否是该socket的控制着
};
}