#pragma once

#define SYLAR_LITTLE_ORDER 1
#define SYLAR_BIG_ORDER 2

#include <type_traits>
#include <byteswap.h>
#include <stdint.h>

namespace sylar
{

/**
 *  @brief 8字节类型的字节序转化成网络序
 *  @details  std::enable_if 用于编译时做条件判断，目的是只有 T类型大小为 uint64_t 才会允许编译此函数
 *            该条件为真，则返回类型为 T; 若为假，则编译时编译器会忽略此函数，不会发出报错信息
 *  @details  bswap_64函数用来对字节序进行反转的函数，它的作用是将一个8字节的数据按照字节进行反转
 */
template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
byteswap(T value)
{
    return static_cast<T>(bswap_64((uint64_t)value));
}

/**
 *  @brief 4字节类型的字节序转换成网络序 
 */
template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
byteswap(T value)
{
    return static_cast<T>(bswap_32((uint32_t)value));
}

/**
 *  @brief 2字节类型的字节序转换成网络序 
 */
template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
byteswap(T value)
{
    return static_cast<T>(bswap_16((uint16_t)value));
}

#if BYTE_ORDER == BIG_ENDIAN
#define SYLAR_BYTE_ORDER SYLAR_BIG_ORDER
#else
#define SYLAR_BYTE_ORDER SYLAR_LITTLE_ORDER
#endif


/*---------------  这说明是大端机器  --------------------*/ 

#if SYLAR_BYTE_ORDER == SYLAR_BIG_ORDER

/**
 *  @brief 只在小端机器上执行字节序反转，在大端机器上什么都不做
 */
template<class T>
T byteswapOnLittleEndian(T t)
{
    return t;
}

/**
 *  @brief 大端机器本身就是 big endian，无需交换
 */
template<class T>
T byteswapOnBigEndian(T t)
{
    return byteswap(t);
}

/*---------------  这说明是小端机器  --------------------*/ 
#else

/**
 *  @brief 小端机器上什么都不做 
 */
template<class T>
T byteswapOnLittleEndian(T t)
{
    return byteswap(t);
}

/**
 *  @brief 只在大端机器上转换，小端机器不变
 */
template<class T>
T byteswapOnBigEndian(T t)
{
    return t;

}

#endif
}
