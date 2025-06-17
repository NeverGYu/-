#pragma once 

#include "../http/session/session.h"
#include "mutex.h"
#include <memory>
#include <unordered_map>

namespace sylar{
namespace http{

class SessionStorage
{
public:
    using ptr = std::shared_ptr<SessionStorage>;

    /**
     *  @brief 析构函数 
     */
    virtual ~SessionStorage() {};

    /**
     *  @brief 存储会话数据
     *  @param[in] session 会话
     */
    virtual void save(Session::ptr session) = 0;

    /**
     *  @brief 根据会话ID返回会话 
     *  @param[in] sessionId 会话ID
     *  @return Session::ptr
     */
    virtual Session::ptr load(const std::string& sessionId) = 0;

    /**
     *  @brief 删除指定id的已存储的会话 
     */
    virtual void remove(const std::string& sessionId) = 0;
};


/**
 *  @brief 基于内存的会话存储实现 
 */
class MemorySessionStorage : public SessionStorage
{
public:
    using ptr = std::shared_ptr<MemorySessionStorage>;
    using RWMutexType = RWMutex;

    virtual ~MemorySessionStorage() {}

    /**
     *  @brief 存储会话数据
     *  @param[in] session 会话
     */
    virtual void save(Session::ptr session) override;

    /**
     *  @brief 根据会话ID返回会话 
     *  @param[in] sessionId 会话ID
     *  @return Session::ptr
     */
    virtual Session::ptr load(const std::string& sessionId) override;

    /**
     *  @brief 删除指定id的已存储的会话 
     */
    virtual void remove(const std::string& sessionId) override;

private:
    std::unordered_map<std::string, Session::ptr> m_sessions;   // 已存储的会话数据
    RWMutexType m_mutex;
};






}
}