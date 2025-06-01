#include "sylar.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT(); // 默认INFO级别

int main()
{
    SYLAR_LOG_FATAL(g_logger) << "fatal msg";
    SYLAR_LOG_ERROR(g_logger) << "err msg";
    SYLAR_LOG_INFO(g_logger) << "info msg";
    SYLAR_LOG_DEBUG(g_logger) << "debug msg";
}