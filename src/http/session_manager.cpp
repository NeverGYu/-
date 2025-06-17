#include "session_manager.h"

namespace sylar{
namespace http{

/*-------------------------------  SessionManager  ----------------------------------*/
SessionManager::SessionManager(std::unique_ptr<SessionStorage> storage)
    : m_storage(std::move(storage))
    , m_random(std::random_device{}())
{}

Session::ptr SessionManager::getSession(const HttpRequest::ptr req,  const HttpResponse::ptr rsp)
{
    // 从请求中获取或创建会话，也就是说，如果请求中包含会话ID，则从存储中加载会话，否则创建一个新的会话
    std::string sessionId = getSessionIdFromCookie(req);
    // 定义初始session
    std::shared_ptr<Session> session;
    // 如果sessionId存在，则取出对应的session
    if (!sessionId.empty())
    {
        session = m_storage->load(sessionId);
    }
    // 如果会话不存在，或者会话已过期
    if (!session || session->isExpired())
    {
        sessionId = generateSessionId();
        session = std::make_shared<Session>(sessionId, this);
        setSessionCookie(sessionId, rsp);
    }
    
    // 为现有会话设置管理器
    session->setSessionManager(this); 
    session->refresh();
    m_storage->save(session);  // 这里可能有问题，需要确保正确保存会话
    return session;
}


std::string SessionManager::generateSessionId()
{
    // 生成唯一的会话标识符，确保会话的唯一性和安全性
    std::stringstream ss;
    std::uniform_int_distribution<> dist(0, 15);

    // 生成32个字符的会话ID，每个字符是一个十六进制数字
    for (int i = 0; i < 32; ++i)
    {
        ss << std::hex << dist(m_random);
    }
    return ss.str();
}

void SessionManager::destroySession(const std::string& sessionId)
{
    m_storage->remove(sessionId);
}

void SessionManager::cleanExpiredSessions()
{
    // 注意：这个实现依赖于具体的存储实现
    // 对于内存存储，可以在加载时检查是否过期
    // 对于其他存储的实现，可能需要定期清理过期会话
}

std::string SessionManager::getSessionIdFromCookie(const HttpRequest::ptr req)
{
    std::string sessionId;
    std::string cookie = req->getHeader("Cookie");

    if (!cookie.empty())
    {
        size_t pos = cookie.find("SYLARSESSIONID=");
        if (pos != std::string::npos)
        {
            pos += 10; // 跳过"sessionId="
            size_t end = cookie.find(';', pos);
            if (end != std::string::npos)
            {
                sessionId = cookie.substr(pos, end - pos);
            }
            else
            {
                sessionId = cookie.substr(pos);
            }
        }
    }
    
    return sessionId;
}

void SessionManager::setSessionCookie(const std::string& sessionId, HttpResponse::ptr resp)
{
    // 设置会话ID到响应头中，作为Cookie
    std::string cookie = "sessionId=" + sessionId + "; Path=/; HttpOnly";
    resp->setHeader("Set-Cookie", cookie);
}

}
}
