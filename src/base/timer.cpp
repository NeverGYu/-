#include "timer.h"
#include "util.h"

namespace sylar
{

bool Timer::Comparator::operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) const
{
    if (!lhs && !rhs)
    {
        return false;
    }

    if (!lhs)
    {
        return true;
    }

    if (!rhs)
    {
        return false;
    }
    
    if (lhs->m_next < rhs->m_next)
    {
        return true;
    }

    if (lhs->m_next > rhs->m_next)
    {
        return false;
    }
    
    return lhs.get() < rhs.get();
}

Timer::Timer(bool recurring, uint64_t ms, std::function<void()> cb, TimerManager* manager)
    : recurring(recurring)
    , m_ms(ms)
    , m_cb(cb)
    , m_manager(manager)
{
    m_next = sylar::GetCurrentMS() + m_ms ;
}


Timer::Timer(uint64_t next)
    : m_next(next)
{}

bool Timer::cancel()
{
    // 在多线程环境下，可能有多个线程会对TimerManager进行操作，所以需要加锁
    TimerManager::RWMutexType::WriteLock Lock(m_manager->m_mutex);
    if (m_cb)   // 判断定时器是否有回调任务，没有直接返回false
    {
        // 如果有回调任务，将其置空
        m_cb = nullptr;
        // 在TimerManager中寻找Timer，将其删除
        auto it = m_manager->m_timers.find(shared_from_this());
        if (it != m_manager->m_timers.end())
        {
            // 这说明找到了，将其从中删除
            m_manager->m_timers.erase(it);
        }
        return true;   
    }
    return false;
}

bool Timer::refresh()
{
    TimerManager::RWMutexType::WriteLock Lock(m_manager->m_mutex);
    if (!m_cb)
    {
        return false;
    }
    // 这说明定时器绑定了回调函数
    auto it = m_manager->m_timers.find(shared_from_this());
    if (it != m_manager->m_timers.end())
    {
         // 这说明定时器列表中存在该定时器，将其从中删除
        m_manager->m_timers.erase(it);
        // 重置执行时间
        m_next = sylar::GetCurrentMS() + m_ms;
        // 将该定时器重新添加到定时器列表中
        m_manager->m_timers.insert(shared_from_this());
        return true;
    }
    return false;
}

bool Timer::reset(uint64_t ms, bool from_now)
{
    // 这说明定时器的时间间隔建通，而且不从现在开始重新及时
    if (ms == m_ms && !from_now)
    {
        return true;
    }
    // 添加锁来进行线程之间的同步
    TimerManager::RWMutexType::WriteLock Lock(m_manager->m_mutex);
    if (!m_cb)
    {
        return false;
    }
    auto it = m_manager->m_timers.find(shared_from_this());
    if (it == m_manager->m_timers.end())
    {
        return false;
    }
    m_manager->m_timers.erase(it);
    // 重新计算时间
    uint64_t start = 0;
    if (from_now)   // 这说明定时器的触发时间是现在
    {
        start = sylar::GetCurrentMS();
    }
    else
    {
        start = m_next - m_ms;  // 获得上次定时器的触发事件 -------> m_next : 是下次定时器的触发时间，m_ms: 是定时器到下次触发时间的时间间隔
    }
    m_ms = ms;
    m_next = m_ms + start;
    m_manager->addTimer(shared_from_this(), Lock);
    return true;
}

TimerManager::TimerManager()
{
    // 记录 TimerManager 被创建时的系统运行时间（以毫秒为单位），用于后续定时器逻辑中检测系统时间是否出现回退、跳变等问题。
    m_previousTime = GetCurrentMS();
}

TimerManager::~TimerManager()
{}

Timer::ptr TimerManager::addTimer(uint64_t ms, bool recurring, std::function<void()> cb)
{
    // 构造一个定时器
    Timer::ptr timer(new Timer(recurring, ms, cb, this));
    // 创建一个写者锁
    TimerManager::RWMutexType::WriteLock Lock(m_mutex);
    // 将定时器添加到定时器列表中
    addTimer(timer,Lock);
    // 返回定时器
    return timer;
}

void TimerManager::addTimer(Timer::ptr val, RWMutexType::WriteLock& Lock)
{
    auto it = m_timers.insert(val).first;
    bool at_front = (it == m_timers.begin()) && !m_tickled;
    if(at_front) 
    {
        m_tickled = true;
    }
    Lock.unlock();

    if(at_front) 
    {
        onTimerInsertAtFront(); 
    }
}

static void onTimer(std::weak_ptr<void> weak_cond, std::function<void()> cb)
{
    // 这用来检测智能指针的指向的对象是否存在(通过弱智能指针weak_ptr不会使引用计数 + 1的特点)
    std::shared_ptr<void> tmp = weak_cond.lock();
    if (tmp)
    {
        cb();
    }
}

Timer::ptr TimerManager::addTimerCondition(uint64_t ms, bool recurring, std::function<void()> cb, std::weak_ptr<void> weak_cond)
{
    return addTimer(ms, recurring, std::bind(&onTimer,weak_cond, cb));
}

uint64_t TimerManager::getNextTimer()
{
    RWMutexType::ReadLock Lock(m_mutex);
    // 将判断定时器重复放入定时器列表的条件置为false
    m_tickled = false;
    // 判断定时器列表是否为空
    if (m_timers.empty())
    {
        return ~0ull;
    }
    // 如果不为空，获得定时器列表中第一个定时器
    const Timer::ptr& next = *m_timers.begin();
    // 判断该定时器是否到期
    uint64_t now = GetCurrentMS();
    // 如果系统启动到现在的时间大于定时器的时间，说明定时器没有执行
    if (now >= next->m_next)
    {
        return 0;
    }
    else
    {
        return next->m_next - now;
    }
}

void TimerManager::listExpireCb(std::vector<std::function<void()>>& cbs)
{
    // 获得系统到现在的总时间
    uint64_t now = GetCurrentMS();
    // 定义一个超时定时器的列表，用来存放时间超过现在的定时器
    std::vector<Timer::ptr> expired;
    // 通过加读锁来判断当前定时器列表是否为空
    {
        RWMutexType::ReadLock Lock(m_mutex);
        if (m_timers.empty())
        {
            return;
        }
    }
    // 因为要对定时器列表进行删除，所以加写锁来进行控制、
    RWMutexType::WriteLock Lock(m_mutex);
    if (m_timers.empty())
    {
        return;
    }
    // 定义回滚变量
    bool rollover = false;
    // 检测是否出现回滚
    if (detectClockRollover(now))
    {
        rollover = true;
    }
    // 这说明没有出现回滚，但是最近的定时器的触发时间都大于当前系统时间
    if (!rollover &&  ((*m_timers.begin())->m_next > now))
    {
        return;
    }
    // 定义一个当前时间的定时器
    Timer::ptr nowTimer(new Timer(now));
    // 根据是否回滚来执行不同的操作
    auto it = rollover ? m_timers.end() : m_timers.lower_bound(nowTimer);
    // 找到第一个大于当前时间的定时器
    while(it != m_timers.end() && (*it)->m_next == now)
    {
        ++it;
    }
    // 将超时定时器全部添加到超时计时器列表中
    expired.insert(expired.begin(), m_timers.begin(), it);
    // 将超时定时器从原来的列表中删除
    m_timers.erase(m_timers.begin(), it);
    // 给回调函数集合扩容
    cbs.resize(expired.size());
    // 遍历超时定时器集合，将超时定时器的回调函数全部添加到回调函数集合中
    for (auto& timer : expired)
    {
        cbs.push_back(timer->m_cb);
        // 判断是不是循环定时器
        if (timer->recurring)
        {
            timer->m_next = now + timer->m_ms;
            m_timers.insert(timer);
        }
        else
        {
            timer->m_cb = nullptr;
        } 
    } 
}

bool TimerManager::detectClockRollover(uint64_t now)
{
    bool rollover = false;
    if (now < m_previousTime && now < (m_previousTime - 60 * 60 * 1000))
    {
        rollover = true;
    }
    m_previousTime = now;
    return rollover;
}

bool TimerManager::hasTimer()
{
    RWMutexType::ReadLock Lock(m_mutex);
    return !m_timers.empty();
}


}