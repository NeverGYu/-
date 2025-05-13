#pragma once
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

namespace sylar
{
    /**
     *  @brief 判断当前线程是否hook 
     */
    bool is_hook_enable();

    /**
     *  @brief 设置当前线程的hook状态 
     */
    void set_hook_enable(bool flag);








/*  <-------------------------------------------------------------------->*/


/**
 *  @brief extern "C" {} 是 C++ 中的一种链接方式，用于告知编译器按照 C 语言的方式来处理其中的函数或变量。 
 *  @details    1、C++ 与 C 的链接兼容性：
                   C++ 编译器对函数名进行 名称修饰（name mangling），即为了支持函数重载，它会修改函数名。
                   使用 extern "C"，可以告诉编译器不要进行名称修饰，而是使用 C 语言的规则来处理符号，
                   这样 C++ 和 C 代码可以进行互相调用，保证它们的符号在链接时是兼容的。

                2、使 C++ 代码可被 C 调用：
                   例如，当你在 C++ 中编写一些函数，想让它们被 C 代码或 C 库调用时，就可以使用 extern "C" 来防止名称修饰，确保 C 代码能够正确找到这些函数。

                3、常用于 C++ 中的头文件：
                   如果在 C++ 的头文件中，定义了一些 C 函数的声明，并且这些函数可能会在 C 代码中使用，可以将这些声明包裹在 extern "C" {} 中，以便 C 代码链接时使用 C 的方式。
 */


}


extern "C"
{

/*-----------------  sleep  ----------------------------*/

using sleep_fun = unsigned int (*)(unsigned int seconds);
extern sleep_fun sleep_f;
 
using usleep_fun = int (*)(useconds_t usec);
extern usleep_fun usleep_f;

using nanosleep_fun = int (*)(const struct timespec* req, struct timespec* rem);
extern nanosleep_fun nanosleep_f;


/*------------------  socket  ----------------------------*/

using socket_fun = int (*)(int domain, int type, int protocol);
extern socket_fun socket_f;

using connect_fun = int (*)(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
extern connect_fun connect_f;

using accept_fun = int (*)(int sockfd, struct sockaddr* addr, socklen_t* addrlen);
extern accept_fun accept_f;


/*------------------  read  ----------------------------*/

using read_fun = ssize_t (*)(int fd, void* buf, size_t count);
extern read_fun read_f;

using readv_fun = ssize_t (*)(int fd, const struct iovec* iov, int iovcnt);
extern readv_fun readv_f;

using recv_fun = ssize_t (*)(int sockfd, void* buf, size_t len, int flags);
extern recv_fun recv_f;

using recvfrom_fun = ssize_t (*)(int sockfd, void* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen);
extern recvfrom_fun recvfrom_f;

using recvmsg_fun = ssize_t (*)(int sockfd, struct msghdr* msg, int flags);
extern recvmsg_fun recvmsg_f;


/*------------------  write  ----------------------------*/

using write_fun = ssize_t (*)(int sockfd, const void* buf, size_t count);
extern write_fun write_f;

using writev_fun = ssize_t (*)(int fd, const struct iovec* iov, int iovcnt);
extern writev_fun writev_f;

using send_fun = ssize_t (*)(int sockfd, const void* msg, size_t len, int flags);
extern send_fun send_f;

using sendto_fun = ssize_t (*)(int sockfd, const void* msg, size_t len, int flags, const struct sockaddr* to, socklen_t tolen);
extern sendto_fun sendto_f;

using sendmsg_fun = ssize_t (*)(int sockfd, const struct msghdr*, int flags);
extern sendmsg_fun sendmsg_f;

using close_fun = int (*)(int fd);
extern close_fun close_f;



/*------------------  other  ----------------------------*/

using fcntl_fun = int (*)(int fd, int cmd, ...);
extern fcntl_fun fcntl_f;

using ioctl_fun = int (*)(int fd, unsigned long int request, ...);
extern ioctl_fun ioctl_f;

using getsockopt_fun = int (*)(int sockfd, int level, int optname, const void* optval, socklen_t* optlen);
extern getsockopt_fun getsockopt_f;

using setsockopt_fun = int (*)(int sockfd, int level,  int optname, const void* optval, socklen_t optlen);
extern setsockopt_fun setsockopt_f;

extern int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms);
}
