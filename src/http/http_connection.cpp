#include "../http/base/http_connection.h"
#include "../http/base/http_parser.h"
#include "../base/log.h"

namespace sylar{
namespace http{

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

/*---------------------------------  HtpResult  -------------------------------------*/
std::string HttpResult::toString() const {
    std::stringstream ss;
    ss << "[HttpResult result=" << result_
       << " error=" << error_
       << " response=" << (rsp_ ? rsp_->toString() : "nullptr")
       << "]";
    return ss.str();
}


/*---------------------------------  HttpConnection  -------------------------------------*/
HttpConnection::HttpConnection(Socket::ptr sock, bool owner)
    :SocketStream(sock, owner) 
{}

HttpConnection::~HttpConnection() 
{
    SYLAR_LOG_DEBUG(g_logger) << "HttpConnection::~HttpConnection";
}


HttpResponse::ptr HttpConnection::recvResponse() 
{
    HttpResponseParser::ptr parser(new HttpResponseParser);
    uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
    std::shared_ptr<char> buffer(new char[buff_size + 1], [](char* ptr){ delete[] ptr; });
    char* data = buffer.get();
    int offset = 0;
    do {
        int len = read(data + offset, buff_size - offset);
        if(len <= 0) 
        {
            close();
            return nullptr;
        }
        len += offset;
        data[len] = '\0';
        size_t nparse = parser->execute(data, len);
        if(parser->hasError())
        {
            close();
            return nullptr;
        }
        offset = len - nparse;
        if(offset == (int)buff_size) 
        {
            close();
            return nullptr;
        }
        if(parser->isFinished()) 
        {
            break;
        }
    } while(true);

    SYLAR_LOG_DEBUG(g_logger) << parser->getData();
    return parser->getData();
}

int HttpConnection::sendRequest(HttpRequest::ptr req) 
{
    std::stringstream ss;
    ss << *req;
    std::string data = ss.str();
    std::cout << "data: " << '\n' << data << std::endl;
    return writeFixSize(data.c_str(), data.size());
}

HttpResult::ptr HttpConnection::DoGet(const std::string& url, uint64_t timeout_ms, const std::map<std::string,std::string>& headers , const std::string& body)
{
    Url::ptr url_ = Url::Create(url);
    if(!url_) 
    {
        return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_URL
                , nullptr, "invalid url: " + url);
    }
    return DoGet(url_, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoGet(Url::ptr url, uint64_t timeout_ms, const std::map<std::string, std::string>& headers, const std::string& body) 
{
    return DoRequest(HttpMethod::GET, url, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoPost(const std::string& url, uint64_t timeout_ms, const std::map<std::string, std::string>& headers, const std::string& body) 
{
    Url::ptr url_ = Url::Create(url);
    if (!url_)
    {
        return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_URL, nullptr, "invalid url: " + url);
    }
    return DoPost(url_, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoPost(Url::ptr url, uint64_t timeout_ms, const std::map<std::string, std::string>& headers, const std::string& body) 
{
    return DoRequest(HttpMethod::POST, url, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoRequest(HttpMethod method, const std::string &url, uint64_t timeout_ms, const std::map<std::string, std::string> &headers, const std::string &body)
{
    Url::ptr url_ = Url::Create(url);
    if (!url_)
    {
        return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_URL, nullptr, "invalid url: " + url);
    }
    return DoRequest(method, url_, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoRequest(HttpMethod method, Url::ptr url, uint64_t timeout_ms, const std::map<std::string, std::string> &headers, const std::string &body)
{
    HttpRequest::ptr req = std::make_shared<HttpRequest>();
    req->setPath(url->getPath());
    req->setQuery(url->getQuery());
    req->setFragment(url->getFragment());
    req->setMethod(method);
    bool has_host = false;
    for(auto& i : headers) 
    {
        if(strcasecmp(i.first.c_str(), "connection") == 0) 
        {
            if(strcasecmp(i.second.c_str(), "keep-alive") == 0) 
            {
                req->setClose(false);
            }
            continue;
        }

        if(!has_host && strcasecmp(i.first.c_str(), "host") == 0) 
        {
            has_host = !i.second.empty();
        }

        req->setHeader(i.first, i.second);
    }
    if(!has_host) 
    {
        req->setHeader("Host", url->getHost());
    }
    req->setBody(body);
    return DoRequest(req, url, timeout_ms);
}

HttpResult::ptr HttpConnection::DoRequest(HttpRequest::ptr req, Url::ptr url, uint64_t timeout_ms)
{
    Address::ptr addr = url->createAddress();
    addr->toString();
    if (!addr)
    {
        return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_HOST, nullptr, "invalid host: " + url->getHost());
    }

    Socket::ptr sock = Socket::CreateTCP(addr);
    if (!sock)
    {
        return std::make_shared<HttpResult>((int)HttpResult::Error::CREATE_SOCKET_ERROR, nullptr, "create socket fail: " + addr->toString() + " errno=" + std::to_string(errno) + " errstr=" + std::string(strerror(errno)));
    }

    if (!sock->connect(addr))
    {
        return std::make_shared<HttpResult>((int)HttpResult::Error::CONNECT_FAIL, nullptr, "connect fail: " + addr->toString());
    }
    sock->setRecvTimeout(timeout_ms);

    HttpConnection::ptr conn = std::make_shared<HttpConnection>(sock);
    int rt = conn->sendRequest(req);
    if (rt == 0)
    {
        return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_CLOSE_BY_PEER, nullptr, "send request closed by peer: " + addr->toString());
    }
    if (rt < 0)
    {
        return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_SOCKET_ERROR, nullptr, "send request socket error errno=" + std::to_string(errno) + " errstr=" + std::string(strerror(errno)));
    }

    auto rsp = conn->recvResponse();
    if (!rsp)
    {
        return std::make_shared<HttpResult>((int)HttpResult::Error::TIMEOUT, nullptr, "recv response timeout: " + addr->toString() + " timeout_ms:" + std::to_string(timeout_ms));
    }
    return std::make_shared<HttpResult>((int)HttpResult::Error::OK, rsp, "ok");
}


/*---------------------------------  HttpConnectionPool  -------------------------------------*/
HttpConnectionPool::HttpConnectionPool(const std::string &host, const std::string &vhost, uint32_t port, uint32_t max_size, uint32_t max_alive_time, uint32_t max_request)
    : m_host(host)
    , m_vhost(vhost)
    , m_port(port)
    , m_maxSize(max_size)
    , m_maxAlivetime(max_alive_time)
    , m_maxRequest(max_request)
{}

HttpConnection::ptr HttpConnectionPool::getConnection()
{
    // 获取当前时间的毫秒数，用于判断连接是否过期
    uint64_t now_ms = sylar::GetCurrentMS();
    // 用于判断无效的连接
    std::vector<HttpConnection *> invalid_conns;
    // 定义一个指针变量 ptr，用来存储最终的有效的连接
    HttpConnection *ptr = nullptr;
    // 对连接池的互斥锁加锁，保证操作线程安全，避免并发访问出现问题
    MutexType::Lock lock(m_mutex);
    // 通过遍历查看连接池中是否有连接
    while (!m_conns.empty())
    {
        // 从连接池的连接列表 m_conns 中取出第一个连接，弹出出队列，准备判断它是否有效
        auto conn = *m_conns.begin();
        m_conns.pop_front();
        // 这表示该连接正在使用
        if (!conn->isConnected())
        {
            invalid_conns.push_back(conn);
            continue;
        }
        // 这表示该连接已经超时
        if ((conn->m_createTime + m_maxAlivetime) > now_ms)
        {
            invalid_conns.push_back(conn);
            continue;
        }
        // 下面表示该连接有效，获得该连接
        ptr = conn;
        break;
    }
    // 释放锁
    lock.unlock();
    // 删除已经超时的连接
    for (auto i : invalid_conns)
    {
        delete i;
    }
    // 删除超时连接的数量
    m_total -= invalid_conns.size();
    // 判断连接是否为空
    if (!ptr)
    {
        // 如果不为空，就获得域名的ip地址
        IPAddress::ptr addr = Address::LookupAnyIPAddress(m_host);
        if (!addr)
        {
            SYLAR_LOG_ERROR(g_logger) << "get addr fail: " << m_host;
            return nullptr;
        }
        // 设置域名的端口
        addr->setPort(m_port);
        // 创建一个TCP的socket
        Socket::ptr sock = Socket::CreateTCP(addr);
        if (!sock)
        {
            SYLAR_LOG_ERROR(g_logger) << "create sock fail: " << *addr;
            return nullptr;
        }
        // 使用该socket连接服务器地址
        if (!sock->connect(addr))
        {
            SYLAR_LOG_ERROR(g_logger) << "sock connect fail: " << *addr;
            return nullptr;
        }
        // 根据该sock创建HttpConnection对象
        ptr = new HttpConnection(sock);
        // 连接池数量加一
        ++m_total;
    }
    // 返回一个智能指针，使用自定义的删除器 ReleasePtr，该删除器用于把连接放回连接池（而不是直接销毁），保证连接的复用。
    return HttpConnection::ptr(ptr, std::bind(&HttpConnectionPool::ReleasePtr, std::placeholders::_1, this));
}

void HttpConnectionPool::ReleasePtr(HttpConnection *ptr, HttpConnectionPool *pool)
{
    // 复用连接的次数
    ++ptr->m_request;
    /**
     * 必须满足三个条件之一
     * 1. 连接关闭
     * 2. 连接超时
     * 3. 连接请求的次数已到达最大
     */ 
    if (!ptr->isConnected() || ((ptr->m_createTime + pool->m_maxAlivetime) >= sylar::GetCurrentMS()) || (ptr->m_request >= pool->m_maxRequest))
    {
        delete ptr;
        --pool->m_total;
        return;
    }
    MutexType::Lock lock(pool->m_mutex);
    pool->m_conns.push_back(ptr);
}

HttpResult::ptr HttpConnectionPool::doGet(const std::string &url, uint64_t timeout_ms, const std::map<std::string, std::string> &headers, const std::string &body)
{
    return doRequest(HttpMethod::GET, url, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnectionPool::doGet(Url::ptr uri, uint64_t timeout_ms, const std::map<std::string, std::string> &headers, const std::string &body)
{
    std::stringstream ss;
    ss << uri->getPath()
       << (uri->getQuery().empty() ? "" : "?")
       << uri->getQuery()
       << (uri->getFragment().empty() ? "" : "#")
       << uri->getFragment();
    return doGet(ss.str(), timeout_ms, headers, body);
}

HttpResult::ptr HttpConnectionPool::doPost(const std::string &url, uint64_t timeout_ms, const std::map<std::string, std::string> &headers, const std::string &body)
{
    return doRequest(HttpMethod::POST, url, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnectionPool::doPost(Url::ptr url, uint64_t timeout_ms, const std::map<std::string, std::string> &headers, const std::string &body)
{
    std::stringstream ss;
    ss << url->getPath()
       << (url->getQuery().empty() ? "" : "?")
       << url->getQuery()
       << (url->getFragment().empty() ? "" : "#")
       << url->getFragment();
    return doPost(ss.str(), timeout_ms, headers, body);
}

HttpResult::ptr HttpConnectionPool::doRequest(HttpMethod method, const std::string &url, uint64_t timeout_ms, const std::map<std::string, std::string> &headers, const std::string &body)
{
    HttpRequest::ptr req = std::make_shared<HttpRequest>();
    req->setPath(url);
    req->setMethod(method);
    // req->setClose(false);
    bool has_host = false;
    for (auto &i : headers)
    {
        if (strcasecmp(i.first.c_str(), "connection") == 0)
        {
            if (strcasecmp(i.second.c_str(), "keep-alive") == 0)
            {
                req->setClose(false);
            }
            continue;
        }

        if (!has_host && strcasecmp(i.first.c_str(), "host") == 0)
        {
            has_host = !i.second.empty();
        }

        req->setHeader(i.first, i.second);
    }
    if (!has_host)
    {
        if (m_vhost.empty())
        {
            req->setHeader("Host", m_host);
        }
        else
        {
            req->setHeader("Host", m_vhost);
        }
    }
    req->setBody(body);
    return doRequest(req, timeout_ms);
}

HttpResult::ptr HttpConnectionPool::doRequest(HttpMethod method, Url::ptr url, uint64_t timeout_ms, const std::map<std::string, std::string> &headers, const std::string &body)
{
    std::stringstream ss;
    ss << url->getPath()
       << (url->getQuery().empty() ? "" : "?")
       << url->getQuery()
       << (url->getFragment().empty() ? "" : "#")
       << url->getFragment();
    return doRequest(method, ss.str(), timeout_ms, headers, body);
}

HttpResult::ptr HttpConnectionPool::doRequest(HttpRequest::ptr req, uint64_t timeout_ms)
{
    auto conn = getConnection();
    if (!conn)
    {
        return std::make_shared<HttpResult>((int)HttpResult::Error::POOL_GET_CONNECTION, nullptr, "pool host:" + m_host + " port:" + std::to_string(m_port));
    }
    auto sock = conn->getSocket();
    if (!sock)
    {
        return std::make_shared<HttpResult>((int)HttpResult::Error::POOL_INVALID_CONNECTION, nullptr, "pool host:" + m_host + " port:" + std::to_string(m_port));
    }
    sock->setRecvTimeout(timeout_ms);
    int rt = conn->sendRequest(req);
    if (rt == 0)
    {
        return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_CLOSE_BY_PEER, nullptr, "send request closed by peer: " + sock->getRemoteAddress()->toString());
    }
    if (rt < 0)
    {
        return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_SOCKET_ERROR, nullptr, "send request socket error errno=" + std::to_string(errno) + " errstr=" + std::string(strerror(errno)));
    }
    auto rsp = conn->recvResponse();
    if (!rsp)
    {
        return std::make_shared<HttpResult>((int)HttpResult::Error::TIMEOUT, nullptr, "recv response timeout: " + sock->getRemoteAddress()->toString() + " timeout_ms:" + std::to_string(timeout_ms));
    }
    return std::make_shared<HttpResult>((int)HttpResult::Error::OK, rsp, "ok");
}

}
}