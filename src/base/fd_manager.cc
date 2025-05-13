#include "fd_manager.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>

namespace sylar
{

FdCtx::FdCtx(int fd)
    : m_isInit(false)
    , m_isSocket(false)
    , m_userNonblock(false)
    , m_sysNonblock(false)
    , m_isClosed(false)
    , m_fd(fd)
    , m_recvTimeout(-1)
    , m_sendTimeout(-1)
{
    init();
}

FdCtx::~FdCtx()
{}

bool FdCtx::init()
{
    // 首先判断当前fd是否已经被初始化
    if (m_isInit)
    {
        return true;
    }
    // 这说明没有被初始化
    m_recvTimeout = -1;
    m_sendTimeout = -1;

    // 定义文件详细信息结构体
    struct stat fd_state;
    // 调用函数fstat通过一个已打开的文件句柄 fd 来获取该文件的属性信息，并填充到 fd_state 指向的结构体中
    if (fstat(m_fd, &fd_state) == -1)
    {
        // 这表示打开失败
        m_isInit = false;
        m_isSocket = false;
    }
    else
    {
        // 这表示获取该文件的属性成功
        m_isInit = true;
        m_isSocket = S_ISSOCK(fd_state.st_mode);   // 通过文件结构体中的属性st_mode来判断是否是socket 
    }
    
    // 如果文件句柄的类型是 socket
    if (m_isSocket)
    {
        int flags = fcntl(m_fd, F_GETFL, 0);
        if (!(flags & O_NONBLOCK))
        {
            fcntl(m_fd, F_SETFL, flags | O_NONBLOCK);
        }
        m_sysNonblock = true;
    }
    else
    {
        m_sysNonblock = false;
    }
    
    m_userNonblock = false;
    m_isClosed = false;
    return m_isInit;
}

void FdCtx::setTimeOut(int type, uint64_t v)
{
    if (type == SO_RCVTIMEO)
    {
        m_recvTimeout = v;
    }
    else
    {
        m_sendTimeout = v;
    }
}

uint64_t FdCtx::getTimeout(int type)
{
    if (type == SO_RCVTIMEO)
    {
        return m_recvTimeout;
    }
    else
    {
        return m_sendTimeout;
    }
    
}

FdManager::FdManager()
{
    m_fdCtxs.resize(64);
}

FdCtx::ptr FdManager::get(int fd, bool auto_create)
{
    if (fd == -1)
    {
        return nullptr;
    }
    RWMutexType::ReadLock lock(m_mutex);
    if (static_cast<int>(m_fdCtxs.size()) < fd)
    {
        if (auto_create == false)
        {
            return nullptr;
        }
    }
    else
    {
        // 如果fd对应的FdCtx存在，或者不允许自动创建FdCtx，则返回现有的FdCtx（有可能为空 -->  FdCtx 不存在，且 auto_create == false）
        if (m_fdCtxs[fd] || !auto_create)
        {
            return m_fdCtxs[fd];
        }
    }

    lock.unlock();
    
    /**
     * 到这里说明:
     *      要么 m_fdCtx.size() > fd & auto_create == true
     *      要么 m_fdCtx.size() <= fd & m_fdCtx 不存在fd & auto_create == true
     */ 
    RWMutexType::WriteLock lock2(m_mutex);
    FdCtx::ptr ctx(new FdCtx(fd));
    if (fd >= static_cast<int>(m_fdCtxs.size()))
    {
        m_fdCtxs.resize(fd * 1.5);
    }
    m_fdCtxs[fd] = ctx;
    return ctx;
}


void FdManager::del(int fd) {
    RWMutexType::WriteLock lock(m_mutex);
    if(static_cast<int>(m_fdCtxs.size()) <= fd) {
        return;
    }
    // 此处使用 reset 而不是 erase，因为使用erase会改变索引（索引和 fd 一一对应）
    m_fdCtxs[fd].reset();
}

}