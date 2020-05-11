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


