#pragma once
#include <memory>

namespace sylar
{

/**
 * @brief 单例模式封装类
 * @details T 类型
 *          X 为了创造多个实例对应的Tag
 *          N 同一个Tag创造多个实例索引
 */
template<typename T, typename X = void, int N = 0>
class SingleTon
{
public:
    static T* GetInstance()
    {
        static T value;
        return &value;
    }
};

/**
 * @brief 单例模式智能指针封装类
 * @details T 类型
 *          X 为了创造多个实例对应的Tag
 *          N 同一个Tag创造多个实例索引
 */
template<typename T, typename X = void, int N = 0>
class SingleTonPtr
{
public:
    static std::shared_ptr<T> GetInstance()
    {
        static std::shared_ptr<T> value(new T);
        return value;
    }
};


}
