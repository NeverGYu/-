#pragma once
#include <cxxabi.h>
#include <sys/types.h>
#include <sys/time.h>
#include <cstdint>
#include <string>
#include <vector>

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













} 


