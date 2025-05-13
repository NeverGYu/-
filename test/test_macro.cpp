#include "log.h"
#include "util.h"
#include "macro.h"


void test_backtrace()
{
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << sylar::BacktraceToString(10, 2, "     ");
    SYLAR_ASSERT(0 == 1);
}

int main()
{
    test_backtrace();
}