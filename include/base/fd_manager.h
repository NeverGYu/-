#pragma once
#include <memory>
#include <vector>
#include "thread.h"
#include "singleton.h"

namespace sylar
{

/**
 *  @brief 文件句柄上下文类
 *  @details 管理文件句柄的类型 |---> 是否socket、是否阻塞、是否关闭、读写超时时间 
 */
class FdCtx : public std::enable_shared_from_this<FdCtx>
{
public:
    using ptr = std::shared_ptr<FdCtx>;

    /**
     *  @brief 构造函数
     *  @details 根据文件句柄fd来构造FdCtx 
     */
    FdCtx(int fd);

    /**
     *  @brief 析构函数 
     */
    ~FdCtx();

    /**
     *  @brief 是否初始化完成 
     */
    bool isInit() const { return m_isInit; }

    /**
     *  @brief 是否是socket 
     */
    bool isSocket() const { return m_isSocket; }

    /**
     *  @brief  是否关闭
     */
    bool isClosed() const { return m_isClosed; }

    /**
     *  @brief 设置用户主动设置非阻塞 
     */
    void setUserNonblock(bool v) { m_userNonblock = v; }

    /**
     *  @brief 获取用户设置非阻塞的状态 
     */
    bool getUserNonblock() const { return m_userNonblock; }

    /**
     * @brief 设置系统非阻塞
     */
    void setSysNonblock(bool v) { m_sysNonblock = v; }
    
    /**
     *   获取系统非阻塞的状态
     */
    bool getSysNonblock() const { return m_sysNonblock; }

    /**
     *  @brief 设置超时时间
     *  @param[in] type |--> 类型：SO_RCVTIMEO(读超时)，SO_SNDTIMEO(写超时)
     *  @param[in] v 时间毫秒 
     */
    void setTimeOut(int type, uint64_t v);

     /**
     * @brief 获取超时时间
     * @param[in] type |--> 类型：SO_RCVTIMEO(读超时), SO_SNDTIMEO(写超时)
     * @return 超时时间毫秒
     */
    uint64_t getTimeout(int type);
private: 
    /**
     *  @brief 初始化 
     */
    bool init();

private:
    bool m_isInit = 1;          // 是否初始化
    bool m_isSocket = 1;        // 是否socket
    bool m_sysNonblock = 1;     // 是否是hook非阻塞
    bool m_userNonblock = 1;    // 是否是用户主动设置非阻塞
    bool m_isClosed = 1;        // 是否关闭
    int m_fd;                   // 文件句柄
    uint64_t m_recvTimeout;     // 读超时时间毫秒
    uint64_t m_sendTimeout;     // 写超时时间毫秒 
};


/**
 *  @brief 文件句柄管理类 
 */
class FdManager
{
public:
    using RWMutexType = RWMutex;
    
    /**
     *  @brief 无参构造函数 
     */
    FdManager();

    /**
     *  @brief 获取/创建文件句柄类FdCtx
     *  @param[in] fd 文件句柄
     *  @param[in] auto_create 是否自动创建
     *  @return 返回对应文件句柄类FdCtx::ptr
     */
    FdCtx::ptr get(int fd, bool auto_create = false);

    /**
     *  @brief 根据文件句柄删除 FdCtx 
     *  @param[in] fd 文件句柄
     */
    void del(int fd);
private:
    RWMutexType m_mutex;            // 读写锁
    std::vector<FdCtx::ptr> m_fdCtxs;    // 文件句柄数组    
};

using FdMgr = sylar::SingleTon<FdManager>;

}