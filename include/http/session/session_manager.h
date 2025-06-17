#pragma once 

#include <memory>
#include <random>
#include "session.h"
#include "session_storage.h"
#include "http.h"

namespace sylar{
namespace http{

/**
 *   @brief SessionManager类负责创建、检索和销毁会话
 */
class SessionManager
{
public:
    using ptr = std::shared_ptr<SessionManager>;

    /**
     *  @brief 构造函数 
     */
    explicit SessionManager(std::unique_ptr<SessionStorage> storage);

    /**
     *  @brief 从请求中获取或创建会话 
     *  @param[in] req Http请求
     *  @param[in] rsp Http响应
     */
    std::shared_ptr<Session> getSession(const HttpRequest::ptr req,  const HttpResponse::ptr rsp);

    /**
     *  @brief 销毁会话
     *  @param[in] sessionId 会话ID
     */
    void destroySession(const std::string& sessionId);

    /**
     *  @brief 清理过期会话 
     */
     void cleanExpiredSessions();

     /**
     *  @brief 更新会话
     *  @param[in] session 会话
     */ 
    void updateSession(std::shared_ptr<Session> session) { m_storage->save(session); }

private:
    /**
     *  @brief 生成会话ID 
     */
    std::string generateSessionId();

    /**
     *  @brief 从Http请求中的Cookie生成会话ID
     *  @param[in] req Http请求 
     */
    std::string getSessionIdFromCookie(const HttpRequest::ptr req);

    /**
     *  @brief 服务端设置Http响应的唯一会话ID
     *  @param[in] sessionId 唯一会话ID
     *  @param[in] rsp Http响应
     */
    void setSessionCookie(const std::string& sessionId, HttpResponse::ptr rsp);

private:
    std::unique_ptr<SessionStorage> m_storage;  // 每一个Manager只有一个Storage，用于保存会话数据
    std::mt19937 m_random;                      // 用于随机生成会话id
};

}
}
