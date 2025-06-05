#include "sylar.h"
#include <iostream>

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_pool()
{
    sylar::http::HttpConnectionPool::ptr pool(new sylar::http::HttpConnectionPool(
        "www.midlane.top", "", 80, 10, 1000 * 30, 5));

    sylar::IOManager::GetThis()->addTimer(1000, false, [pool](){
        auto r = pool->doGet("/", 300);
        std::cout << r->toString() << std::endl;
    });

}

void run()
{
    sylar::Address::ptr addr = sylar::Address::LookupAny("www.baidu.com:80");
    SYLAR_LOG_DEBUG(g_logger) << addr->toString();
    if (!addr)
    {
        SYLAR_LOG_INFO(g_logger) << "get addr error";
        return;
    }

    SYLAR_LOG_INFO(g_logger) << "connect to: " << *addr;

    sylar::Socket::ptr sock = sylar::Socket::CreateTCP(addr);
    int rt = sock->connect(addr);
    if (!rt)
    {
        SYLAR_LOG_INFO(g_logger) << "connect " << *addr << " failed";
        return;
    }
    
    sylar::http::HttpConnection::ptr conn(new sylar::http::HttpConnection(sock));
    sylar::http::HttpRequest::ptr req(new sylar::http::HttpRequest);
    req->setPath("/");
    req->setHeader("host", "www.baidu.com");
    req->setHeader("connection", "keep-alive");
    req->init();
    std::cout << "req:" << std::endl
              << *req << std::endl;

    rt = conn->sendRequest(req);
    if (rt > 0)
    {
        SYLAR_LOG_DEBUG(g_logger)<< "write " <<  rt << " bytes ";
    }
    else
    {
        SYLAR_LOG_DEBUG(g_logger)<< "fail";
    }

    auto rsp = conn->recvResponse();
    if (!rsp)
    {
        SYLAR_LOG_INFO(g_logger) << "recv response error";
        return;
    }
    std::cout << "rsp:" << std::endl
              << *rsp << std::endl;

    // std::cout << "=========================" << std::endl;

    // auto r = sylar::http::HttpConnection::DoGet("http://www.baidu.com", 30000);
    // std::cout << "result=" << r->result_
    //           << " error=" << r->error_
    //           << " rsp=" << (r->rsp_ ? r->rsp_->toString() : "")
    //           << std::endl;

    // std::cout << "=========================" << std::endl;
    // test_pool();
}

int main(int argc, char **argv)
{
    sylar::IOManager iom(2);
    iom.schedule(run);
    return 0;
}