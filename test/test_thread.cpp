#include "sylar.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

int count = 0;

sylar::Mutex s_mutex;

void func(void* arg)
{
    SYLAR_LOG_INFO(g_logger) 
        << " name:" << sylar::Thread::GetName()
        << " this.name:" << sylar::Thread::GetThis()->getName()
        << " id:" << sylar::GetThreadId()
        << " this.id:" << sylar::Thread::GetThis()->getId();
    SYLAR_LOG_INFO(g_logger) << " args: " << *(int*)arg;
    
    for (size_t i = 0; i < 10000; i++)
    {
        sylar::Mutex::Lock lock(s_mutex);
        ++count;
    }
    
}

int main(int argc, char* argv[])
{
    std::vector<sylar::Thread::ptr> thrs;
    int arg = 123456;
    for (size_t i = 0; i < 3; i++)
    {
        // 带参数的线程用std::bind绑定
        sylar::Thread::ptr thr(new sylar::Thread(std::bind(func,&arg),"thread_" + std::to_string(i)));
        thrs.push_back(thr);
    }
    
    for(int i = 0; i < 3; i++) {
        thrs[i]->join();
    }
    
    SYLAR_LOG_INFO(g_logger) << "count = " << count;

    return 0;
}