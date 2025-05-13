#include <time.h>
#include <unistd.h>
#include <cstdint>
#include <syscall.h>
#include <pthread.h>
#include <string>
#include <sstream>
#include <execinfo.h>
#include "util.h"
#include "log.h"
#include "fiber.h"

namespace sylar
{

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

pid_t GetThreadId()
{
    return gettid();
}

uint64_t GetFiberId()
{
    return Fiber::GetFiberId();
}

std::string GetThreadName()
{
    char threadName[16] = {0};
    pthread_getname_np(pthread_self(),threadName, sizeof(threadName));
    return threadName;
}

uint64_t GetElapsedMS() {
    struct timespec ts = {0};
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}




/*---------------------  macro.h  ------------------------*/

/**
 * @brief 获得当前的调用栈
 *   
 */
void Backtrace(std::vector<std::string>& bt, int size, int skip)
{
    void **array = (void**)malloc(sizeof(void*) * size);
    size_t s = ::backtrace(array, size);

    char** str = ::backtrace_symbols(array,s);

    if (str == NULL)
    {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "backtrace str erroor";
    }
    for (size_t i = skip; i < s; i++)
    {
        bt.push_back(str[i]);
    }
    free(array);
    free(str);
}

/**
 * @brief 获取当前栈信息的字符串
 * @param[in] size 栈的最大层数
 * @param[in] skip 跳过栈顶的层数
 * @param[in] prefix 栈信息前输出的内容
 */
std::string BacktraceToString(int size, int skip, const std::string prefix)
{
    std::vector<std::string> bt;
    Backtrace(bt,size,skip);
    std::stringstream ss;
    for (size_t i = 0; i < bt.size(); i++)
    {
        ss << prefix << bt[i] << std::endl;
    }
    return ss.str();
}



/*---------------------  timer.h  ------------------------*/


uint64_t GetCurrentMS() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000ul + tv.tv_usec / 1000;
}

uint64_t GetCurrentUS() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 * 1000ul + tv.tv_usec;
}




} // namespace sylar
