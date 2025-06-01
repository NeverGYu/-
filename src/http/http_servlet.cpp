#include "http_servlet.h"
#include <fnmatch.h>

namespace sylar{
namespace http{

FunctionServlet::FunctionServlet(callback cb)
    : Servlet("FunctionServlet")
    , m_cb(cb)
{}

int32_t FunctionServlet::handle(HttpRequest::ptr req, HttpResponse::ptr rsp, HttpSession::ptr session)
{
    return m_cb(req, rsp, session);
}

ServletDispatch::ServletDispatch()
    : Servlet("ServletDispatch")
{
    m_default.reset(new NotFoundServlet("sylar/1.0"));
}

int32_t ServletDispatch::handle(HttpRequest::ptr req, HttpResponse::ptr rsp, HttpSession::ptr session)
{
    auto slt = getMatchedServlet(req->getPath());
    if (slt)
    {
        slt->handle(req, rsp, session);
    }
    return 0;
}

void ServletDispatch::addServlet(const std::string& url, Servlet::ptr slt)
{
    RWMutexType::WriteLock lock(m_mutex);
    m_datas[url] = slt;
}

void ServletDispatch::addServlet(const std::string& url, FunctionServlet::callback cb)
{
    RWMutexType::WriteLock lock(m_mutex);
    m_datas[url].reset(new FunctionServlet(cb));
}

void ServletDispatch::addGlobServlet(const std::string& url, Servlet::ptr slt)
{
    RWMutexType::WriteLock lock(m_mutex);
    for (auto it = m_globs.begin(); it != m_globs.end(); ++it)
    {
        if (it->first == url)
        {
            m_globs.erase(it);
            break;
        }
    }
    m_globs.push_back(std::make_pair(url, slt));
}

void ServletDispatch::addGlobServlet(const std::string& url, FunctionServlet::callback cb)
{
    return addGlobServlet(url, FunctionServlet::ptr(new FunctionServlet(cb)));
}

void ServletDispatch::deleteServlet(const std::string& url)
{
    RWMutexType::WriteLock lock(m_mutex);
    m_datas.erase(url);
}

void ServletDispatch::delGlobServlet(const std::string& url)
{
    RWMutexType::WriteLock lock(m_mutex);
    for (auto it = m_globs.begin(); it != m_globs.end(); ++it)
    {
        if (it->first == url)
        {
            m_globs.erase(it);
            break;
        }
    }
}

Servlet::ptr ServletDispatch::getServlet(const std::string& url)
{
    RWMutexType::ReadLock lock(m_mutex);
    auto it = m_datas.find(url);
    return it == m_datas.end() ? nullptr :it->second;
}

Servlet::ptr ServletDispatch::getGlobServlet(const std::string& url)
{
    RWMutexType::ReadLock lock(m_mutex);
    for (auto it = m_globs.begin(); it != m_globs.end(); ++it)
    {
        if (it->first == url)
        {
            return it->second;
        }
    }
    return nullptr;
}

Servlet::ptr ServletDispatch::getMatchedServlet(const std::string& url)
{
    RWMutexType::ReadLock lock(m_mutex);
    auto mit = m_datas.find(url);
    if (mit != m_datas.end())
    {
        return mit->second;
    }
    for (auto it = m_globs.begin(); it != m_globs.end(); ++it)
    {
        if (!fnmatch(it->first.c_str(), url.c_str(), 0))
        {
            return it->second;
        }
    }
    return m_default;
}

NotFoundServlet::NotFoundServlet(const std::string& name)
    : Servlet("NotFoundServlet")
    , m_name(name) 
{
    m_content = "<html><head><title>404 Not Found"
        "</title></head><body><center><h1>404 Not Found</h1></center>"
        "<hr><center>" + name + "</center></body></html>";

}

int32_t NotFoundServlet::handle(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session) 
{
    response->setStatus(sylar::http::HttpStatus::NOT_FOUND);
    response->setHeader("Server", "sylar/1.0.0");
    response->setHeader("Content-Type", "text/html");
    response->setBody(m_content);
    return 0;
}


}
}