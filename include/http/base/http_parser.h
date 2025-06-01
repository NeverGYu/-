#pragma once

#include "http.h"
#include "http_parser_template.h"

namespace sylar{
namespace http{

/**
 *  @brief HTTP请求解析类 
 */
class HttpRequestParser
{
public:
    using ptr = std::shared_ptr<HttpRequestParser>;

    /**
     *  @brief 构造函数 
     */
    HttpRequestParser();

    /**
     *  @brief 解析协议
     *  @param[in, out] data 协议文本
     *  @param[in] length 协议文本长度
     *  @return 返回实际解析的长度，并且将已解析的长度移除  
     */
    size_t execute(char* data, size_t len);

    /**
     *  @brief 是否解析完成 
     */
    bool isFinished() const { return m_finished; }

    /**
     *  @brief 设置是否解析完成 
     */
    void setFinished(bool v) { m_finished = v; }

    /**
     *  @brief 是否有错误 
     */
    bool isError() const { return m_error; }

    /**
     *  @brief 设置错误码 
     */
    void setError(int v) { m_error = v; }

    /**
     * @brief 是否有错误
     */
    bool hasError() const { return !!m_error; }
    /**
     *  @brief 返回http_parser结构体
     */
    http_parser getParser() const { return m_parser; }

    /**
     *  @brief 返回HttpRequest请求体 
     */
    HttpRequest::ptr getHttpRequest() const { return m_data; }

    /**
     *  @brief 获取当前的HTTP头部的field 
     */
    std::string getField() const { return m_field; }

    /**
     *  @brief 设置当前的HTTP头部的field 
     */
    void setField(const std::string& v) { m_field = v; } 

    /**
     *  @brief 获取HttpRequest协议解析的缓存大小 
     */
    static uint64_t GetHttpRequestBufferSize();

    /**
     *  @brief  获取HttpRequest协议的最大消息体大小
     */
    static uint64_t GetHttpRequestMaxBodySize();

private: 
    http_parser m_parser;       // http请求解析器
    HttpRequest::ptr m_data;    // http请求体
    int m_error;                // 错误码
    bool m_finished;            // 是否解析结束
    std::string m_field;        /* 当前HTTP头部field，nodejs中的http-parser解析HTTP头部是分field和value两次返回
                                 * <Host, www.baidu.com> --> field: Host || value: www.baidu.com
                                 */ 
};

class HttpResponseParser
{
public:
    using ptr = std::shared_ptr<HttpResponseParser>;

     /**
     * @brief 构造函数
     */
    HttpResponseParser();

    /**
     * @brief 解析HTTP响应协议
     * @param[in, out] data 协议数据内存
     * @param[in] len 协议数据内存大小
     * @param[in] chunck 是否在解析chunck
     * @return 返回实际解析的长度,并且移除已解析的数据
     */
    size_t execute(char *data, size_t len);

    /**
     * @brief 是否解析完成
     */
    int isFinished() const { return m_finished; }

    /**
     * @brief 设置是否解析完成
     */
    void setFinished(bool v) { m_finished = v; }

    /**
     * @brief 是否有错误
     */
    int hasError() const { return !!m_error; }

    /**
     * @brief 设置错误码
     * @param[in] v 错误码
     */
    void setError(int v) { m_error = v; }

    /**
     * @brief 返回HttpResponse
     */
    HttpResponse::ptr getData() const { return m_data; }

    /**
     * @brief 返回http_parser
     */
    const http_parser &getParser() const { return m_parser; }

    /**
     * @brief 获取当前的HTTP头部field
     */
    const std::string &getField() const { return m_field; }

    /**
     * @brief 设置当前的HTTP头部field
     */
    void setField(const std::string &v) { m_field = v; }

    /**
     * @brief 返回HTTP响应解析缓存大小
     */
    static uint64_t GetHttpResponseBufferSize();

    /**
     * @brief 返回HTTP响应最大消息体大小
     */
    static uint64_t GetHttpResponseMaxBodySize();

private:
    http_parser m_parser;        // http响应解析器
    HttpResponse::ptr m_data;   // http响应体
    int m_error;                // 错误码
    bool m_finished;            // 是否解析完成
    std::string m_field;        // 当前HTTP头部的field
};

}
}