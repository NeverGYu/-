#include <sylar.h>

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_tcp_client()
{
    int ret;

    auto socket = sylar::Socket::CreateTCPSocket();
    SYLAR_ASSERT(socket);

    auto addr = sylar::IPAddress::Create("127.0.0.1", 12345);
    SYLAR_ASSERT(addr);

    ret = socket->connect(addr);
    SYLAR_ASSERT(ret);

    SYLAR_LOG_INFO(g_logger) << "connect success, peer address: " << socket->getRemoteAddress()->toString();

    std::string buffer;
    buffer.resize(1024);
    socket->recv(&buffer[0], buffer.size());
    SYLAR_LOG_INFO(g_logger) << "recv: " << buffer;
    socket->close();

    return;
}

int main()
{
    sylar::IOManager iom;
    iom.schedule(&test_tcp_client);
    return 0;
}