#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include "locker.h"

class http_conn{
    public:
        static const int FILENAME_LEN=200;          //文件名最大长度
        static const int READ_BUFFER_SIZE=2048;     //读缓存区的大小
        static const int WRITE_BUFFER_SIZE=1024;    //写缓存区的大小

        /*HTTP请求方法，仅支持GET*/
        enum METHOD{
            GET=0,POST,HEAD,PUT,DELETE,
            TRACE,OPTIONS,CONNECT,PATCH
        };

        /*解析客户请求时，主状态机所处的状态*/
        enum CHECK_STATE{
            CHECK_STATE_REQUESTLINE=0,  //正在分析请求行
            CHECK_STATE_HEADER,         //正在分析头部字段
            CHECK_STATE_CONTENT         //正在分析内容段
        };

        /*服务器处理HTTP请求可能的结果*/
        enum HTTP_CODE{
            NO_REQUEST,             //请求不完整，需要继续读；
            GET_REQUEST,            //获取一个完整的请求；
            BAD_REQUEST,            //请求有语法错误
            NO_RESOURCE,            //请求的资源不存在
            FORBIDDEN_RESOURCE,     //对资源没有足够的权限
            FILE_RESOURCE,          //请求的资源存在
            INTERNAL_ERROR,         //内部错误
            CLOSED_CONNECTION,      //客户机已经关闭连接
            NOT_IMPLEMENT           //请求未实现
        };

        /*行的读取状态，分别表示：读取到一个完整的行、行出错、行尚且不完整*/
        enum LINE_STATUS {LINE_OK=0,LINE_BAD,LINE_OPEN};

    public:
        http_conn(){}
        ~http_conn(){}

        /*初始化新接受的连接*/
        void init(int sockfd,const sockaddr_in& addr);
        /*关闭连接*/
        void close_conn(bool real_close=true);
        /*处理客户机请求*/
        void process();
        /*非阻塞读操作*/
        bool read();
        /*非阻塞写操作*/
        bool write();
        /*初始化连接*/
        void init();
        /*解析HTTP请求*/
        HTTP_CODE process_read();
        /*填充HTTP应答*/
        bool process_write(HTTP_CODE ret);

        /*下面这组函数用于HTTP请求的分析*/
        HTTP_CODE parse_request_line(char* text);
        HTTP_CODE parse_header(char* text);
        HTTP_CODE parse_content(char* text);
        HTTP_CODE do_request();
        char* get_line(){
            return m_read_buf+m_start_line;
        }
        LINE_STATUS parse_line();

        /*下面这组函数用于填充HTTP应答*/
        void unmap();
        bool add_response(const char* format, ... );
        bool add_content(const char* content);
        bool add_status_line(int status,const char* title);
        bool add_headers(int content_length);
        bool add_content_length(int content_length);
        bool add_linger();
        bool add_blank_line();

    public:
        /*
        所有socket上的事件都注册到一个epoll内核事件表中
        所以讲epoll文件描述符设置为静态的
        */
        static int m_epollfd;
        /*估计用户数量*/
        static int m_user_count;

    private:
        /*该http连接的socket和对方的socket地址*/
        int m_sockfd;
        sockaddr_in m_address;

        /*读缓存区*/
        char m_read_buf[READ_BUFFER_SIZE];
        /*标识读缓存区已经进读入的客户数据的最后一个字节的下一个位置*/
        int m_read_idx;
        /*当前正在分析的字符在读缓存区的位置*/
        int m_check_idx;
        /*当前正在解析的行的起始位置*/
        int m_start_line;
        /*写缓存区*/
        char m_write_buf[WRITE_BUFFER_SIZE];
        /*写缓存区中待发送的字节数*/
        int m_write_idx;

        /*主状态机当前所处的状态*/
        CHECK_STATE m_check_state;
        /*请求方法*/
        METHOD m_method;

        /*客户请求的目标文件的完整路径，其内容等于doc_root+m_url,doc_root是根目录*/
        char m_real_file[FILENAME_LEN];
        /*客户机请求的目标文件的文件名*/
        char* m_url;
        /*HTTP协议版本号，我们仅支持HTTP/1.1*/
        char* m_version;
        /*主机名*/
        char* m_host;
        /*http请求的消息体的长度*/
        int m_content_length;
        /*HTTP请求是否要求保持连接*/
        bool m_linger;

        /*客户机请求的目标文件被mmap到内存中的起始位置*/
        char* m_file_address;
        /*目标文件的状态。通过它可以判断文件是否存在，是否为目录，是否可读，并获取文件大小等信息*/
        struct stat m_file_stat;

        /*使用writev来执行写操作，m_iv_count表示内存块的数量*/
        struct iovec m_iv[2];
        int m_iv_count;
        
};
#endif