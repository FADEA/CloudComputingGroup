# 套接字概念
1. Linux文件的类型（伪文件，不占有实际的存储空间）。
2. 两个缓冲区(4k大小左右)用一个文件描述符来描述，双向全双工，内核中用两个管道实现。

# 网络字节序
1. 网络数据流应采用大端字节序，所以需要将主机字节序转化成网络字节序
2. 转换函数如下

函数类型 | 函数名称 | 函数作用
:-|:-|:-
uint32_t | htonl | 主机到网络转换ip地址
uint16_t | htons | 主机到网络转换端口号
uint32_t | ntohl | 网络到主机转换ip地址
uint16_t | ntohs | 网络到主机转换端口号

# ip地址转换函数
1. linux操作系统中除了提供以上函数之外，还提供了一下两个函数
2. ip地址------>网络字节序 inet_pton()
3. 网络字节序-->点分十进制字符串 inet_ntop()

# sockaddr数据结构
1. struct sockaddr（诞生于IPV4协议）
	1. 16位地址类型
	2. 14字节地址数据
2. struct sockaddr_in（细分sockaddr而得）
	1. 16位地址类型
	2. 16位端口号
	3. 32位ip地址
	4. 8字节填充
3. 在使用bind，accept，connect函数时，注意将sockaddr_in强转成sockaddr类型

# 端口复用
1. setsockopt()
2. getsockopt()

# 多路IO转接
1. 不再由应用程序自己监听客户端的连接，取而代之由内核替应用程序监视文件，主要使用的方法有三种
	1. select
	2. poll
	3. epoll

# select函数
1. int select(int nfds, fd_set *readfds, fd_set *writefds,fd_set *exceptfds, struct timeval *timeout);
2. 参数一：所监听的所有文件描述符中，最大的文件描述符+1
3. 参数二三四：所监听的文件描述符可读事件，可写事件，异常事件（使用位图来描述），都是传入传出参数
4. 最后一个参数：超时的时间，如果所监听的事件规定时间内没有返回，则select直接返回
5. 返回值为int类型，成功则返回的是所监听的所有的监听集合中满足条件的总数
6. 如何把事件加到集合中，如何从返回的总数的找到每个集合中的满足事件，通过一下几个函数时间
	1. void FD_CLR(int fd, fd_set *set);//将fd从set清除出去，即将set位图中对应位置设置成0
    2. int  FD_ISSET(int fd, fd_set *set);//将fd设置到set中
    3. void FD_SET(int fd, fd_set *set);//判断fd是否在set中
    4. void FD_ZERO(fd_set *set);//将集合清空为0值
7. 弊端
	1. 文件描述符上限（进程默认只能开1024个文件描述符），所以select同时最大只能监听1024个
	2. 返回的只是一个数，如果监听的事件不多，需要循环遍历1024，浪费
	3. 监听集合和满足监听条件的集合，所以需要不断保存原有的集合，否则使用select函数后会被覆盖

# poll函数
1. int poll(struct pollfd *fds, nfds_t nfds, int timeout);
2. struct pollfd {
               int   fd;         /* file descriptor */
               short events;     /* requested events */
               short revents;    /* returned events */
           };
3. 参数一：数组的首地址


# epool函数
1. epoll是linux下多路复用IO接口select/poll的增强版本，它能够显著的提高程序在大量并发连接中只有少数活跃的情况下系统cpu的利用率
2. int epoll_create(int size);
	1. 该函数返回一个红黑树的树根
	2. 调用该函数时，操作系统会根据size创建这么一颗红黑树
3.  int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
	1. 参数一：epoll_create的返回值
	2. 参数二：控制选项
	3. 某一个具体的文件描述符
	4. struct epoll_event {
               uint32_t     events;      /* Epoll events */
               epoll_data_t data;        /* User data variable */
           };
	5.  typedef union epoll_data {
               void        *ptr;
               int          fd;
               uint32_t     u32;
               uint64_t     u64;
           } epoll_data_t;
4.  int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
	1. 参数二是一个数组，是一个传出参数
	3. 参数三描述数组有多大

