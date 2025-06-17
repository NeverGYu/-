#pragma once

#include <thread>
#include <functional>
#include <memory>
#include <pthread.h>
#include <semaphore.h>
#include <cstdint>
#include <atomic>
#include <list>
#include "noncopyable.h"

namespace sylar
{
class Semaphore
{
public:
    /**
     *  @brief 构造函数
     */
    Semaphore(uint32_t count = 0);

    /**
     * @brief 析构函数 
     */
    ~Semaphore();

    /**
     * @brief 获取信号量 
     */
    void wait();

    /**
     * @brief 释放信号量 
     */
    void notify();

private:
    sem_t m_semaphore;
};


/**
 * @brief 局部锁的模板实现
 */
template<typename T>
class ScopeLockImpl
{
public:
    /**
     *  @brief 构造函数 
     */
    ScopeLockImpl(T& mutex)
        : m_mutex(mutex)
    {
        m_mutex.lock();
        m_locked = true;
    }

    /**
     *  @brief 析构函数
     */
    ~ScopeLockImpl()
    {
        m_mutex.unlock();
    }

    /**
     * @brief 加锁 
     */
    void lock()
    {
        if (!m_locked)
        {
            m_mutex.lock();
            m_locked = true;
        }
    }

    /**
     * @brief 释放锁 
     */
    void unlock()
    {
        if (m_locked)
        {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;       // 互斥量
    bool m_locked;  // 判断是否被锁
};

/**
 * @brief 局部读锁模板实现
 */
template<class T>
class ReadScopedLockImpl {
public:
    /**
     * @brief 构造函数
     * @param[in] mutex 读写锁
     */
    ReadScopedLockImpl(T& mutex)
        : m_mutex(mutex) {
        m_mutex.rdlock();
        m_locked = true;
    }

    /**
     * @brief 析构函数,自动释放锁
     */
    ~ReadScopedLockImpl() {
        unlock();
    }

    /**
     * @brief 上读锁
     */
    void lock() {
        if(!m_locked) {
            m_mutex.rdlock();
            m_locked = true;
        }
    }

    /**
     * @brief 释放锁
     */
    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    /// mutex
    T& m_mutex;
    /// 是否已上锁
    bool m_locked;
};

/**
 * @brief 局部写锁模板实现
 */
template<class T>
class WriteScopedLockImpl {
public:
    /**
     * @brief 构造函数
     * @param[in] mutex 读写锁
     */
    WriteScopedLockImpl(T& mutex)
        :m_mutex(mutex) {
        m_mutex.wrlock();
        m_locked = true;
    }

    /**
     * @brief 析构函数
     */
    ~WriteScopedLockImpl() {
        unlock();
    }

    /**
     * @brief 上写锁
     */
    void lock() {
        if(!m_locked) {
            m_mutex.wrlock();
            m_locked = true;
        }
    }

    /**
     * @brief 解锁
     */
    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    /// Mutex
    T& m_mutex;
    /// 是否已上锁
    bool m_locked;
};

/**
 * @brief 互斥量 
 */
class Mutex : Noncopyable
{
public:
    using Lock = ScopeLockImpl<Mutex>;
    /**
     *  @brief 构造函数 
     */
    Mutex()
    {
        pthread_mutex_init(&mutex, nullptr);
    }

    /**
     * @brief 析构函数 
     */
    ~Mutex()
    {
        pthread_mutex_destroy(&mutex);
    }

    /**
     * @brief 对互斥量加锁 
     */
    void lock()
    {
        pthread_mutex_lock(&mutex);
    }

    /**
     * @brief 对互斥量解锁 
     */
    void unlock()
    {
        pthread_mutex_unlock(&mutex);
    }
private:
    pthread_mutex_t mutex;
};


/**
 * @brief 读写互斥量 
 */
class RWMutex : Noncopyable
{
public:
    using ReadLock = ReadScopedLockImpl<RWMutex>;
    using WriteLock = WriteScopedLockImpl<RWMutex>;

    /**
     *  @brief 构造函数 
     */
    RWMutex()
    {
        pthread_rwlock_init(&m_lock,nullptr);
    }

    /**
     * @brief 析构函数 
     */
    ~RWMutex()
    {
        pthread_rwlock_destroy(&m_lock);
    }

    /**
     * @brief 上读锁 
     */
    void rdlock()
    {
        pthread_rwlock_rdlock(&m_lock);
    }

    /**
     *  @brief 上写锁 
     */
    void wrlock()
    {
        pthread_rwlock_wrlock(&m_lock);
    }

    /**
     * @brief 释放锁 
     */
    void unlock()
    {
        pthread_rwlock_unlock(&m_lock);
    }
private:
    pthread_rwlock_t m_lock;    // 读写锁
};


/**
 * @brief 自旋锁
 */
class Spinlock : Noncopyable
{
public:
    using Lock = ScopeLockImpl<Spinlock>;
    /**
     *  @brief 构造函数 
     */
    Spinlock()
    {
        pthread_spin_init(&m_lock, 0 );
    }

    /**
     * @brief 析构函数 
     */
    ~Spinlock()
    {
        pthread_spin_destroy(&m_lock);
    }

    /**
     * @brief 加锁
     */
    void lock()
    {
        pthread_spin_lock(&m_lock);
    }

     /**
     * @brief 释放锁
     */
    void unlock()
    {
        pthread_spin_unlock(&m_lock);
    }

private:
    pthread_spinlock_t m_lock;
};


/**
 * @brief 原子锁
 */
class CASLock : Noncopyable
{
public:
    using Lock = ScopeLockImpl<CASLock>;
    /**
     *  @brief 构造函数 
     */
    CASLock()
    {
        m_lock.clear();
    }

    /**
     * @brief 析构函数 
     */
    ~CASLock()
    {}

    /**
     * @brief 加锁
     */
    void lock()
    {
        while(std::atomic_flag_test_and_set_explicit(&m_lock, std::memory_order_acquire));
    }

     /**
     * @brief 释放锁
     */
    void unlock()
    {
        std::atomic_flag_test_and_set_explicit(&m_lock, std::memory_order_release);
    }

private:
    volatile std::atomic_flag m_lock;
};


/*---------------------用于调式的空锁---------------------------*/
/**
 * @brief 空锁(用于调试)
 */
class NullMutex : Noncopyable{
public:
    /// 局部锁
    using Lock = ScopeLockImpl<NullMutex>;

    /**
     * @brief 构造函数
     */
    NullMutex() {}

    /**
     * @brief 析构函数
     */
    ~NullMutex() {}

    /**
     * @brief 加锁
     */
    void lock() {}

    /**
     * @brief 解锁
     */
    void unlock() {}
};
    

/**
 * @brief 空读写锁(用于调试)
 */
class NullRWMutex : Noncopyable {
public:
    // 局部读锁
    using ReadLock = ReadScopedLockImpl<NullMutex>;
    // 局部写锁
    using WriteLock = WriteScopedLockImpl<NullMutex>;

    /**
     * @brief 构造函数
     */
    NullRWMutex() {}

    /**
     * @brief 析构函数
     */
    ~NullRWMutex() {}

    /**
     * @brief 上读锁
     */
    void rdlock() {}

    /**
     * @brief 上写锁
     */
    void wrlock() {}

    /**
     * @brief 解锁
     */
    void unlock() {}
};













}


