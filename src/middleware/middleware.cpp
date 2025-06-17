#include "../middleware/middleware.h"
#include "../base/log.h"

namespace sylar{
namespace middleware{

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

void MiddleChain::addMiddleware(MiddleWare::ptr middle)
{
    m_middlewares.push_back(middle);
}

void MiddleChain::processBefore(sylar::http::HttpRequest::ptr req)
{
    if (m_middlewares.empty())
    {
        SYLAR_LOG_INFO(g_logger) << "middlerwares is empty()";
        return;
    }
    
    for (auto middle : m_middlewares)
    {
        middle->before(req);
    }
}


void MiddleChain::processAfter(sylar::http::HttpResponse::ptr rsp)
{
    try
    {
        // 反向处理响应，以保持中间件的正确执行顺序
        for (auto it = m_middlewares.rbegin(); it != m_middlewares.rend(); ++it)
        {
            if (*it)    // 添加空指针检查
            { 
                (*it)->after(rsp);
            }
        }
    }
    catch (const std::exception &e)
    {
        SYLAR_LOG_ERROR(g_logger) << "Error in middleware after processing: " << e.what();
    }
}

}
}