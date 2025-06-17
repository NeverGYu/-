#include "../../middleware/cors/CorsMiddleware.h"
#include "../../base/log.h"

namespace sylar{
namespace middleware{

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

CorsMiddleWare::CorsMiddleWare(const CorsConfig& config)
    : m_config(config)
{}

void CorsMiddleWare::before(sylar::http::HttpRequest::ptr req)
{
    SYLAR_LOG_DEBUG(g_logger) << "CorsMiddleware::before - Processing request";

    if (req->getMethod() == sylar::http::HttpMethod::OPTIONS)
    {
        SYLAR_LOG_DEBUG(g_logger) << "Processing CORS preflight request";
        sylar::http::HttpResponse::ptr rsp = std::make_shared<sylar::http::HttpResponse>();
        handlePreflightRequest(req,rsp);
        throw rsp;
    }
    
}

void CorsMiddleWare::after(sylar::http::HttpResponse::ptr rsp) 
{
    SYLAR_LOG_DEBUG(g_logger) << "CorsMiddleware::after - Processing response";
    
    // 直接添加CORS头，简化处理逻辑
    if (!m_config.allowedOrigins.empty()) 
    {
        // 如果允许所有源
        if (std::find(m_config.allowedOrigins.begin(), m_config.allowedOrigins.end(), "*")  != m_config.allowedOrigins.end()) 
        {
            addCorsHeaders(rsp, "*");
        } 
        else 
        {
            // 添加第一个允许的源
            addCorsHeaders(rsp, m_config.allowedOrigins[0]);
        }
    }
}


bool CorsMiddleWare::isOriginAllowed(const std::string &origin) const 
{
    return m_config.allowedOrigins.empty() || 
           std::find(m_config.allowedOrigins.begin(), 
           m_config.allowedOrigins.end(), "*") != m_config.allowedOrigins.end() ||
           std::find(m_config.allowedOrigins.begin(), 
           m_config.allowedOrigins.end(), origin) != m_config.allowedOrigins.end();
}

void CorsMiddleWare::handlePreflightRequest(const sylar::http::HttpRequest::ptr request, sylar::http::HttpResponse::ptr response)
{
    const std::string& origin = request->getHeader("origin");
    if (!isOriginAllowed(origin))
    {
        SYLAR_LOG_WARN(g_logger) << "Origin not allowed: " << origin;
        response->setStatus(sylar::http::HttpStatus::FORBIDDEN);
        return;
    }

    addCorsHeaders(response, origin);
    response->setStatus(sylar::http::HttpStatus::NO_CONTENT);
    SYLAR_LOG_INFO(g_logger) << "Preflight request processed successfully";
    
}

void CorsMiddleWare::addCorsHeaders(sylar::http::HttpResponse::ptr response, const std::string& origin)
{
    try
    {
        response->setHeader("Access-Control-Allow-Origin", origin);

        if (m_config.allowCredentials)
        {
            response->setHeader("Access-Control-Allow-Credentials", "true");
        }

        if (!m_config.allowedMethods.empty())
        {
            response->setHeader("Access-Control-Allow-Methods", join(m_config.allowedMethods, ", "));
        }
        
        if (!m_config.allowedHeaders.empty()) 
        {
            response->setHeader("Access-Control-Allow-Headers", join(m_config.allowedHeaders, ", "));
        }
        
        response->setHeader("Access-Control-Max-Age", std::to_string(m_config.maxAge));
        SYLAR_LOG_DEBUG(g_logger) << "CORS headers added successfully";
    }
    catch(const std::exception& e)
    {
        SYLAR_LOG_ERROR(g_logger) << "Error adding CORS headers: " << e.what();
    }
    
}

// 工具函数：将字符串数组连接成单个字符串
std::string CorsMiddleWare::join(const std::vector<std::string>& strings, const std::string& delimiter) 
{
    std::ostringstream result;
    for (size_t i = 0; i < strings.size(); ++i) 
    {
        if (i > 0) 
        {
            result << delimiter;
        }
        result << strings[i];
    }
    return result.str();
}


}}