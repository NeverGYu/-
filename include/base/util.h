#pragma once
#include <cstdint>
#include <sys/time.h>
#include <cxxabi.h> // for abi::__cxa_demangle()
#include <string>
#include <vector>
#include <iostream>

namespace sylar
{

/*------------------------- log.h  -----------------------------*/

/**
 * @brief 获取线程id
 * @note 这里不要把pid_t和pthread_t混淆，关于它们之的区别可参考gettid(2)
 */
pid_t GetThreadId();

/**
 * @brief 获取当前启动的毫秒数，参考clock_ge ttime(2)，使用CLOCK_MONOTONIC_RAW
 */
uint64_t GetElapsedMS(); 

/**
 * @brief 获取协程id
 * @todo 桩函数，暂时返回0，等协程模块完善后再返回实际值
 */
uint64_t GetFiberId();

/**
 * @brief 获取线程名称，参考pthread_getname_np(3)
 */
std::string GetThreadName();

/**
 * @brief 日期时间转字符串
 */
std::string TimeToStr(time_t ts = time(0), const std::string &format = "%Y-%m-%d %H:%M:%S");

/*---------------------  config.h  ------------------------*/

/**
 * @brief 获取T类型的类型字符串
 */
template<class T>
const char* TypeToName()
{
    static const char* s_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
    return s_name;
}


/*---------------------  macro.h  ------------------------*/

/**
 * @brief 获得当前的调用栈
 * @param[in] size 栈的最大层数
 * @param[in] skip 跳过栈顶的层数
 * @param[in] prefix 栈信息前输出的内容
 */
void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);

/**
 * @brief 获取当前栈信息的字符串
 * @param[in] size 栈的最大层数
 * @param[in] skip 跳过栈顶的层数
 * @param[in] prefix 栈信息前输出的内容
 */
std::string BacktraceToString(int size, int skip, const std::string prefix="");



/*---------------------  timer.h  ------------------------*/

/**
 *  @brief 获取当前时间的毫秒
 */
uint64_t GetCurrentMS();


/**
 *  @brief 获取当前时间的微妙
 */
uint64_t GetCurrentUS();

/*---------------------  http.h  ------------------------*/
/**
 * @brief 字符串辅助类
 */
class StringUtil 
{
public:
    /**
    * @brief printf风格的字符串格式化，返回格式化后的string
    */
    static std::string Format(const char* fmt, ...);
    
    /**
     * @brief vprintf风格的字符串格式化，返回格式化后的string
    */
    static std::string Formatv(const char* fmt, va_list ap);
    
    /**
     * @brief url编码
     * @param[in] str 原始字符串
     * @param[in] space_as_plus 是否将空格编码成+号，如果为false，则空格编码成%20
     * @return 编码后的字符串
     */
    static std::string UrlEncode(const std::string& str, bool space_as_plus = true);

    /**
     * @brief url解码
     * @param[in] str url字符串
     * @param[in] space_as_plus 是否将+号解码为空格
     * @return 解析后的字符串
     */
    static std::string UrlDecode(const std::string &str, bool space_as_plus = true);

    /**
     * @brief 移除字符串首尾的指定字符串
     * @param[] str 输入字符串
     * @param[] delimit 待移除的字符串
     * @return  移除后的字符串
     */
    static std::string Trim(const std::string &str, const std::string &delimit = " \t\r\n");

    /**
     * @brief 移除字符串首部的指定字符串
     * @param[] str 输入字符串
     * @param[] delimit 待移除的字符串
     * @return  移除后的字符串
     */
    static std::string TrimLeft(const std::string &str, const std::string &delimit = " \t\r\n");

    /**
     * @brief 移除字符尾部的指定字符串
     * @param[] str 输入字符串
     * @param[] delimit 待移除的字符串
     * @return  移除后的字符串
     */
    static std::string TrimRight(const std::string &str, const std::string &delimit = " \t\r\n");

    /**
     * @brief 宽字符串转字符串
     */
    static std::string WStringToString(const std::wstring &ws);

    /**
     * @brief 字符串转宽字符串
     */
    static std::wstring StringToWString(const std::string &s);
};










} 


