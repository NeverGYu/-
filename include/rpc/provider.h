#pragma once

#include "google/protobuf/service.h"
#include <memory>
#include "tcp_server.h"
#include "address.hpp"

namespace sylar{
namespace rpc{

/**
 *  @brief 框架提供的专门发布rpc服务的网络对象类 
 */
class RpcProvider
{
public:
    using ptr = std::shared_ptr<RpcProvider>;
    /**
     *  @brief 这里是专门提供给外部使用的，可以发布rpc方法的函数接口 
     */
    void NotifyService(google::protobuf::Service* service);

    /**
     *  @brief 启动rpc服务，开始提供rpc远程网络调用服务 
     */
    void run();

private:
    
};


}
}

