#pragma once
#include "../stream/socket_stream.hpp"
#include "http.h"
#include <memory>

namespace sylar{
namespace http{

/**
 *  @brief HttpSession用来封装进行accept的socket 
 */
class HttpSession : public SocketStream
{
public:
    using ptr = std::shared_ptr<HttpSession>;

    /**
     *  @brief 构造函数 
     */
    HttpSession(Socket::ptr sock, bool owner = true);

    /**
     *  @brief 用来接收Http请求 
     */
    HttpRequest::ptr recvRequest();

    /**
     *  @brief 用来发送Http响应
     *  @param[in] rsp HTTP响应
     *  @return >0 发送成功
     *          =0 对方关闭
     *          <0 Socket异常
     */
    int sendResponse(HttpResponse::ptr rsp);
};

}
}