#pragma once
#include <assert.h>
#include "util.h"
#include "log.h"

/*-------------------- 断言宏封装 --------------------*/
#define SYLAR_ASSERT(x) \
    if (!(x))\
    { \
         SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERT " #x \
                                           << "\nbacktrace\n" \
                                           << sylar::BacktraceToString(100,2,"    "); \
    assert(x); \
    }

#define SYLAR_ASSERT2(x,w) \
   if (!(x)) \
    { \
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERT " #x \
                                          << "\n" \
                                          << #w \
                                          << "\nbacktrace\n" \
                                          << sylar::BacktraceToString(100,2,"    "); \
        assert(x); \
    }
    

    