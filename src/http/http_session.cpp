#include "http_session.h"
#include "http_parser.h"

namespace sylar{
namespace http{

HttpSession::HttpSession(Socket::ptr sock, bool owner)
    : SocketStream(sock, owner)
{}

HttpRequest::ptr HttpSession::recvRequest() 
{
    // 定义Http解析器
    HttpRequestParser::ptr parser(new HttpRequestParser);
    // 获得最大可解析的长度
    uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
    // 定义一个字符串数组
    std::shared_ptr<char> buffer(new char[buff_size], [](char* ptr) { delete[] ptr; }); 
    // 获得原始数组的指针
    char* data = buffer.get();
    // 定义读取数据的偏移量
    int offset = 0;
    // 通过do-while循环来解析请求
    do
    {
        // read函数的调用过程: 此处的read -> SocketStream的read -> Socket的recv -> 此处的Socket是accept返回的socket -> 用来接受对方发送的请求
        int len = read(data + offset, buff_size - offset);
        // 如果没有读取到数据就关闭连接
        if (len <= 0)
        {
            close();
            return nullptr;
        }
        // 此处代表读取到了数据
        len += offset;
        // 调用解析器来进行解析: nparse表示解析了的长度
        size_t nparse = parser->execute(data, len);
        // 判断解析结果
        if (parser->hasError())
        {
            close();
            return nullptr;
        }
        // 此时的offset代表已经解析了的数据长度
        offset = len - nparse;
        // 这表示解析长度超过了最大解析长度
        if (offset == (int)buff_size)
        {
            close();
            return nullptr;
        }
        // 这表示解析终止
        if (parser->isFinished()) {
            break;
        }
    } while (true);
    // 设置连接状态
    parser->getHttpRequest()->init();
    return parser->getHttpRequest();
}

int HttpSession::sendResponse(HttpResponse::ptr rsp) 
{
    std::stringstream ss;
    ss << *rsp;
    std::string data = ss.str();
    return writeFixSize(data.c_str(), data.size());
}



}
}