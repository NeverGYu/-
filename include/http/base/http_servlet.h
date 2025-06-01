#pragma once

#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include "http.h"
#include "http_session.h"
#include "../base/mutex.h"

namespace sylar{
namespace http{

/**
 *  @brief Servlet类封装 
 */
class Servlet
{
public:
    using ptr = std::shared_ptr<Servlet>;
    /**
     *  @brief 构造函数
     *  @param[in] name 
     */
    Servlet(const std::string& name)
        : m_name(name)
    {}

    /**
     *  @brief 析构函数 
     */
    virtual ~Servlet() {}

    /**
     *  @brief 处理请求
     *  @param[in] req Http请求结构体
     *  @param[in] rsp Http响应结构体
     *  @param[in] session Http连接 
     */
    virtual int32_t handle(HttpRequest::ptr req, HttpResponse::ptr rsp, HttpSession::ptr session) = 0;

    /**
     *  @brief 返回当前Servlet的名称 
     */
    std::string getName() const { return m_name; }
protected:
    std::string m_name;
};

/**
 *  @brief 函数式Servlet  
 */
class FunctionServlet : public Servlet
{
public:
    using ptr = std::shared_ptr<FunctionServlet>;
    using callback = std::function<int32_t(HttpRequest::ptr req, HttpResponse::ptr rsp, HttpSession::ptr session)>;

    /**
     *  @brief 构造函数 
     */
    FunctionServlet(callback cb);

    /**
     *  @brief 对应的处理方法
     */
    virtual int32_t handle(HttpRequest::ptr req, HttpResponse::ptr rsp, HttpSession::ptr session) override;
private:
    callback m_cb;
};


/**
 *  @brief Servlet 分发器
 */
class ServletDispatch : public Servlet
{
public:
    using ptr = std::shared_ptr<ServletDispatch>;
    using RWMutexType = RWMutex;

    /**
     *  @brief 构造函数 
     */
    ServletDispatch();

    /**
     *  @brief url对应的处理方法 
     */
    virtual int32_t handle(HttpRequest::ptr req, HttpResponse::ptr rsp, HttpSession::ptr session) override;

    /**
     *  @brief 添加一个Servlet
     *  @param[in] url 资源存放的地址
     *  @param[in] slt 该资源对应的Servlet
     */
    void addServlet(const std::string& url, Servlet::ptr slt);

    /**
     *  @brief 添加一个Servlet
     *  @param[in] url 资源存放的地址
     *  @param[in] cb 回调函数
     */
    void addServlet(const std::string& url, FunctionServlet::callback cb);

    /**
     * @brief 添加模糊匹配servlet
     * @param[in] uri uri 模糊匹配 /sylar_*
     * @param[in] slt servlet
     */
    void addGlobServlet(const std::string& url, Servlet::ptr slt);

    /**
     * @brief 添加模糊匹配servlet
     * @param[in] uri uri 模糊匹配 /sylar_*
     * @param[in] cb FunctionServlet回调函数
     */
    void addGlobServlet(const std::string& url, FunctionServlet::callback cb);

    /**
     *  @brief 删除一个Servlet 
     */
    void deleteServlet(const std::string& url);

    /**
     * @brief 删除模糊匹配servlet
     */
    void delGlobServlet(const std::string& url);

    /**
     *  @brief 返回默认的Servlet 
     */
    Servlet::ptr getDefault() const { return m_default; }

    /**
     *  @brief 设置默认的Servlet 
     */
    void setDefault(Servlet::ptr v)  { m_default = v; }

    /**
     *  @brief 通过url获取servlet
     *  @param[in] url 
     */
    Servlet::ptr getServlet(const std::string& url);

    /**
     *  @brief 通过url获取模糊匹配Servlet
     *  @param[in] url 
     */
    Servlet::ptr getGlobServlet(const std::string& url);

     /**
     * @brief 通过uri获取servlet
     * @param[in] uri uri
     * @return 优先精准匹配,其次模糊匹配,最后返回默认
     */
    Servlet::ptr getMatchedServlet(const std::string& url);

private:
    RWMutexType m_mutex;                                         // 读写锁
    std::unordered_map<std::string, Servlet::ptr> m_datas;      // 精准匹配(uri --> servlet)
    std::vector<std::pair<std::string, Servlet::ptr>> m_globs;  // 模糊匹配(uri --> servlets)
    Servlet::ptr m_default;                                     // 默认Servlet
};


/**
 * @brief NotFoundServlet(默认返回404)
 */
class NotFoundServlet : public Servlet
{
public:
    // 智能指针类型定义
    using ptr = std::shared_ptr<NotFoundServlet>;

    /**
     * @brief 构造函数
     */
    NotFoundServlet(const std::string &name);
    virtual int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session) override;

private:
    std::string m_name;
    std::string m_content;
};

}
}