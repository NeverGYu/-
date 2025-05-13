#pragma once

class Noncopyable
{
public:
    /**
     *  @brief 允许被继承 
     */
    Noncopyable() = default;
    
    ~Noncopyable() = default;
private:
    /**
     *  @brief 不允许被拷贝 
     */
    Noncopyable(const Noncopyable& rhs) = delete;
    Noncopyable(const Noncopyable&& rhs) = delete;
    /**
     *  @brief 不允许被赋值 
     */
    Noncopyable operator=(const Noncopyable& rhs) = delete;
    Noncopyable operator=(const Noncopyable&& rhs) = delete;
};