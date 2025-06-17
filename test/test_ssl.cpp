#include "../base/sylar.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();


void test_ssl()
{
    // 设置 SSL 的配置
    sylar::ssl::SslConfig config;
    config.setCertificateFile("/home/gch/sylar/bin/conf/server.crt");
    config.setPrivateKeyFile("/home/gch/sylar/bin/conf/server.key");
    config.setVersion(sylar::ssl::SslVersion::TLS_1_2);
    // 创建 SSL 上下文
    sylar::ssl::SslContext::ptr ssl_ctx = std::make_shared<sylar::ssl::SslContext>(config);
    // 初始化 SSL 上下文
    if (!ssl_ctx->initilaize()) {
        SYLAR_LOG_ERROR(g_logger) << "Failed to initialize SSL context" << std::endl;
        return;
    }
    // 创建HttpServer
    sylar::http::HttpServer::ptr server(new sylar::http::HttpServer(true
                                , sylar::IOManager::GetThis()
                                , sylar::IOManager::GetThis()
                                , true
                                , ssl_ctx));
    // 设置ip地址
    sylar::Address::ptr address = sylar::Address::LookupAnyIPAddress("0.0.0.0:8443");
    // 判断地址是否存在
    if (!address)
    {
        SYLAR_LOG_ERROR(g_logger) << " address is null";
    }
    // 服务器绑定地址
    while (!server->bind(address))
    {
        sleep(2);
    }
    // 设置路由
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
        res->setBody("Hello!\n");
        auto id = req->getParam("param1"); // 提取动态参数 
        res->appendBody("UserDetail ID: " + id);
        return 0;
    }));

    server->start();
}

int main()
{
    sylar::IOManager iom(1);
    iom.schedule(&test_ssl);
}
