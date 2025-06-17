#include "sylar.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test()
{
    // 创建一个Http服务器
    sylar::http::HttpServer::ptr server(new sylar::http::HttpServer);
    // 创建一个Http服务器的地址
    sylar::Address::ptr addr = sylar::Address::LookupAnyIPAddress("192.168.100.130:8020");
    // 判断是否为空
    if (!addr)
    {
        SYLAR_LOG_INFO(g_logger) << "access addr fails";
    }
    // 该服务器绑定地址
    while (!server->bind(addr))
    {
        sleep(2);
    }
    // 创建中间件CORS配置
    sylar::middleware::CorsConfig cors_config;
    cors_config.allowedOrigins = {"http://192.168.100.130"};
    cors_config.allowedMethods = {"GET", "POST", "PUT", "DELETE", "OPTIONS"};
    cors_config.allowedHeaders = {"Content-Type", "Authorization"};
    cors_config.allowCredentials = true;
    cors_config.maxAge = 3600;
    // 添加CORS中间件
    auto corsMiddle = std::make_shared<sylar::middleware::CorsMiddleWare>(cors_config);
    server->addMiddleware(corsMiddle);

    // 该服务器设置对应的Servlet
    auto sd = server->getServletDispatch();
    sd->addServlet("/sylar/xx", [](sylar::http::HttpRequest::ptr req, sylar::http::HttpResponse::ptr rsp, sylar::http::HttpSession::ptr session){
        rsp->setBody(req->toString());
        return 0;
    });
    sd->addGlobServlet("/sylar/*",[](sylar::http::HttpRequest::ptr req, sylar::http::HttpResponse::ptr rsp, sylar::http::HttpSession::ptr session){
        rsp->setBody(req->toString());
        return 0;
    });
    sd->addRoute(sylar::http::HttpMethod::GET, "/api/users", std::make_shared<sylar::http::FunctionServlet>(
        [](sylar::http::HttpRequest::ptr req, sylar::http::HttpResponse::ptr res, sylar::http::HttpSession::ptr) {
        auto id = req->getParam("param1"); // 提取动态参数 
        res->setBody("UserDetail ID: " + id);
        return 0;
    }));

    sd->addRoute(sylar::http::HttpMethod::OPTIONS, "/xx", std::make_shared<sylar::http::FunctionServlet>(
        [](sylar::http::HttpRequest::ptr req, sylar::http::HttpResponse::ptr rsp, sylar::http::HttpSession::ptr) {
        rsp->setBody(rsp->toString());
        return 0;
    }));

    server->start();
}


int main()
{
    sylar::IOManager iom(1);
    iom.schedule(&test);
}