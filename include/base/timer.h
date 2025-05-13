#pragma once

#include "mutex.h"

#include <memory>
#include <functional>
#include <set>
#include <vector>

namespace sylar
{

class TimerManager;

class Timer : public std::enable_shared_from_this<Timer>
{
    friend class TimerManager;
public:
    using ptr = std::shared_ptr<Timer>;

    /**
     *  @brief 取消定时器 
     */
    bool cancel();

    /**
     *  @brief 刷新定时器 
     */
    bool refresh();

    /**
     *  @brief 重置定时器 
     *  @param[in] ms 定时器执行间隔时间(毫秒)
     *  @param[in] from_now 是否从当前时间开始计算
     */
    bool reset(uint64_t ms, bool from_now);

private:
    /**
     *  @brief 构造函数
     *  @param[in] recurring
     *  @param[in] m_ms
     *  @param[in] cb
     *  @param[in] TimerManager
     */
    Timer(bool recurring, uint64_t ms, std::function<void()> cb, TimerManager* manager);

    /**
     *  @brief 构造函数
     *  @param[in] 准确的执行时间(毫秒)
     */
    Timer(uint64_t next);

private:
    /**
     *  @brief 定时器比较仿函数 
     */
    struct Comparator
    {
        bool operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) const;
    };
    
private:
    bool recurring = false;             // 判断是否是循环计时器
    uint64_t m_ms = 0;                  // 执行周期
    uint64_t m_next = 0;                // 精确的执行时间
    std::function<void()> m_cb;         // 定时器绑定的回调任务
    TimerManager* m_manager = nullptr;  // 定时器管理类
};


class TimerManager
{
    friend class Timer;
public:
    using RWMutexType = RWMutex;

    /**
     *  @brief 构造函数 
     */
    TimerManager();

    /**
     *  @brief 析构函数 
     */
    ~TimerManager();

    /**
     *  @brief 添加定时器到定时器列表中
     *  @param[in]  ms  定时器需要多久执行
     *  @param[in]  recurring   是否是循环定时器
     *  @param[in]  cb   定时器绑定的回调函数
     */
    Timer::ptr addTimer(uint64_t ms, bool recurring, std::function<void()> cb);

    /**
     *  @brief 添加条件定时器到定时器列表中
     *  @param[in]  ms  定时器需要多久执行
     *  @param[in]  recurring   是否是循环定时器
     *  @param[in]  cb   定时器绑定的回调函数
     *  @param[in]  weak_cond 执行条件
     */
    Timer::ptr addTimerCondition(uint64_t ms, bool recurring, std::function<void()> cb, std::weak_ptr<void> weak_cond); 

    /**
     *  @brief 得到最近的定时器的时间间隔 
     */
    uint64_t getNextTimer();

    /**
     *  @brief 获得需要执行的定时器的回调函数列表
     *  @param[in]  cbs 
     */
    void listExpireCb(std::vector<std::function<void()>>& cbs);

    /**
     *  @brief 判断是否有定时器 
     */
    bool hasTimer();

protected:
    /**
     *  @brief 当有新的定时器到来时，将其插入到定时器列表中的首部 
     */
    virtual void onTimerInsertAtFront() = 0;

    /**
     *  @brief 将定时器添加到管理器
     *  @param[in] Timer::ptr
     *  @param[in] WriteLock 写锁，因为会对定时器进行修改，不可以用读锁
     *  @details 避免重复加锁
     */
    void addTimer(Timer::ptr val, RWMutexType::WriteLock& lock);

private:
    /**
     *  @brief 检测服务器时间是否被调整 
     */
    bool detectClockRollover(uint64_t now_ms);
private:
    RWMutexType m_mutex;                                // 读写锁互斥量
    std::set<Timer::ptr, Timer::Comparator> m_timers;   // 定时器集合
    uint64_t m_previousTime = 0;                        // 上次定时器执行时间
    bool m_tickled = false;                             // 避免了在插入新的最早定时器时，每次都重新触发通知，而只会在第一次插入时处理它
};

}