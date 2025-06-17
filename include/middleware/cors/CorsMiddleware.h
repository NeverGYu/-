#pragma once

#include "../middleware.h"
#include "../http/base/http.h"
#include "CorsConfig.h"

namespace sylar{
namespace middleware{

class CorsMiddleWare : public MiddleWare
{
public:
    explicit CorsMiddleWare(const CorsConfig &config = CorsConfig::defaultConfig());

    /**
     *  @brief 接收Http响应前处理 
     */
    void before(sylar::http::HttpRequest::ptr req) override;

    /**
     *  @brief 发送Http响应后处理 
     */
    void after(sylar::http::HttpResponse::ptr rsp) override;

    /**
     *  @brief 辅助函数
     *  @param[in] strings
     *  @param[in] delimiter 
     */
    std::string join(const std::vector<std::string> &strings, const std::string &delimiter);

private:
    /**
     *  @brief 判断对方地址是否可以被接受请求
     *  @param[in] origin 地址   
                 */ 
    bool isOriginAllowed(const std::string &origin) const;

    void handlePreflightRequest(const sylar::http::HttpRequest::ptr request, sylar::http::HttpResponse::ptr response);

    void addCorsHeaders(sylar::http::HttpResponse::ptr response, const std::string &origin);

private:
    CorsConfig m_config;
};

}
}


