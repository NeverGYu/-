#include "sylar.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

static int timeout = 1000;
static sylar::Timer::ptr s_timer;

void timer_callback() {
    SYLAR_LOG_INFO(g_logger) << "timer callback, timeout = " << timeout;
    timeout += 1000;
    if(timeout < 5000) {
        s_timer->reset(timeout, true);
    } else {
        s_timer->cancel();
    }
}

void test_timer() {
    sylar::IOManager iom;

    // 循环定时器
    s_timer = iom.addTimer(1000, true,timer_callback);
    
    // 单次定时器
    iom.addTimer(500, false,[]{
        SYLAR_LOG_INFO(g_logger) << "500ms timeout";
    });
    iom.addTimer(5000, false, []{
        SYLAR_LOG_INFO(g_logger) << "5000ms timeout";
    });
}

int main(int argc, char *argv[]) {
    test_timer();

    SYLAR_LOG_INFO(g_logger) << "end";

    return 0;
}