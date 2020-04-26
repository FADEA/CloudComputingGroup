#include "http_conn.h"

/*定义HTTP响应的一些状态信息*/
const char* ok_200_title="OK";
const char* error_400_title="Bad Request";
const char* error_400_form="Your request has bad syntax or is inherently impossible to satisfy.\n";
const char* error_403_title="Forbidden";
const char* error_403_form="You do not have permission to get file from this server.\n";
const char* error_404_title="Not Found";
const char* error_404_form="The requested file was not found on this server.\n";
const char* error_500_title="Internal Error";
const char* error_500_form="There was an unusual problem serving the requested file.\n";

int setnonblocking(int fd){
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

/*
 将文件描述符fd上的EPOLLIN注册到epollfd指示的epoll内核
 事件表中，参数oneshot指定是否注册fd上的EPOLLONESHOT事件
*/
void addfd(int epollfd,int fd,bool oneshot){
    epoll_event event;
    event.data.fd=fd;
    event.events=EPOLLIN | EPOLLET | EPOLLRDHUP;
    if(oneshot){
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    setnonblocking(fd);
}

void removefd(int epollfd,int fd){
    epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,0);
    close(fd);
}

void modfd(int epollfd,int fd,int ev){
    epoll_event event;
    event.data.fd=fd;
    event.events=ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&event);
}

int http_conn::m_user_count=0;
int http_conn::m_epollfd=-1;

void http_conn::close_conn(bool real_close){
    if(real_close&&(m_sockfd!=-1)){
        removefd(m_epollfd,m_sockfd);
        m_sockfd=-1;
        /*将客户数量减1*/
        m_user_count--;
    }
}

void http_conn::init(int sockfd,const sockaddr_in& addr){
    m_sockfd=sockfd;
    m_address=addr;
    addfd(m_epollfd,m_sockfd,true);
    m_user_count++;

    init();
}

void http_conn::init(){
    m_check_state=CHECK_STATE_REQUESTLINE;
    m_linger=false;

    m_method=GET;
    m_url=0;
    m_version=0;
    m_content_length=0;
    m_host=0;
    m_start_line=0;
    m_check_idx=0;
    m_read_idx=0;
    m_write_idx=0;
    memset(m_read_buf,'\0',READ_BUFFER_SIZE);
    memset(m_write_buf,'\0',WRITE_BUFFER_SIZE);
    memset(m_real_file,'\0',FILENAME_LEN);
}

/*从状态机，用于解析出一行命令*/
http_conn::LINE_STATUS http_conn::parse_line(){

    char temp;
    for(;m_check_idx<m_read_idx;m_check_idx++){
        //获取当前要分析的字节
        temp=m_read_buf[m_check_idx];
        //如果当前要分析的是回车符，则说明可能读到一个完整的行
        if(temp=='\r'){
            /*如果\r是目前最后一个已经读入的数据，那么这次可能没有读到完整的行
            返回LINE_OPEN表示还需要继续读*/
            if((m_check_idx+1)==m_read_idx){
                return LINE_OPEN;
            }
            //如果下一个字符是\n，说明读到一个完整的行
            else if(m_read_buf[m_check_idx+1]=='\n'){
                m_read_buf[m_check_idx++]='\0';
                m_read_buf[m_check_idx++]='\0';
                return LINE_OK;
            }
            //否则说明客户机的HTTP存在语法问题
            return LINE_BAD;
        }
        //如果当前读到的是\n，说明读到一个完整的行
        else if(temp=='\n'){
            //再检查一下
            if((m_check_idx>1)&&(m_read_buf[m_check_idx-1]=='\r')){
                m_read_buf[m_check_idx-1]='\0';
                m_read_buf[m_check_idx++]='\0';
                return LINE_OK;
            }
            //如果前一个字符不是\r，说明HTTP语法有问题
            return LINE_BAD;
        }
    }
    /*如果所有内容都分析完也没遇到\r，说明还需要继续读，返回LINE_OPEN*/
    return LINE_OPEN;
}

/*循环读取客户数据，直到无数据可读或者对方连接关闭*/
bool http_conn::read(){
    if(m_read_idx>=READ_BUFFER_SIZE){
        return false;
    }

    int bytes_read=0;
    while(true){
        bytes_read=recv(m_sockfd,m_read_buf+m_read_idx,READ_BUFFER_SIZE-m_read_idx,0);
        if(bytes_read<0){
            if(errno==EAGAIN||errno==EWOULDBLOCK){
                break;
            }
            return false;
        }else if(bytes_read==0){
            return false;
        }
    }
    return true;
}

/*解析http请求行，获取请求方法，目标URL，以及HTTP版本号*/
http_conn::HTTP_CODE http_conn::parse_request_line(char* text){
    //如果请求行中没有空白字符或\t字符，则HTTP请求必有问题
    m_url=strpbrk(text," \t");
    if(!m_url){
        return BAD_REQUEST;
    }
    //将空格转换成'\0'
    *m_url++='\0';

    //获取请求方法
    char* method=text;
    if(strcmp(method,"GET")==0){
        m_method=GET;
    }else{
        return BAD_REQUEST;
    }

    m_url+=strspn(m_url," \t");
    m_version=strpbrk(m_url," \t");
    if(!m_version){
        return BAD_REQUEST;
    }
    *m_version++='\0';
    m_version+=strspn(m_version," \t");
    if(strcasecmp(m_version,"HTTP/1.1")!=0){
        return BAD_REQUEST;
    }
    // if(strncasecmp(m_url,"http://",7)==0){
    //     m_url+=7;
    //     m_url=strchr(m_url,'/');
    // }
    if(!m_url||m_url[0]!='/'){
        return BAD_REQUEST;
    }
    //HTTP请求行处理完毕，状态转移到头部字段的分析
    m_check_state=CHECK_STATE_HEADER;
    return NO_REQUEST;

}

/*解析HTTP请求的一个头部信息*/
http_conn::HTTP_CODE http_conn::parse_header(char* text){
    //遇到空行表示头部字段解析完毕
    if(text[0]=='\0'){
        /*如果HTTP请求有消息体，则还需要读取m_content_length字节的消息体
        状态机转移到CHECK_STATE_CONTENT状态*/
        if(m_content_length!=0){
            m_check_state=CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }

        /*否则说明已经得到一个完整的HTTP请求*/
        return GET_REQUEST;
    }
    //处理Connection头部字段
    else if(strncasecmp(text,"Connection:",11)==0){
        text+=11;
        text+=strspn(text," \t");
        if(strcasecmp(text,"keep-alive")==0){
            m_linger=true;
        }
    }
    //处理Content-Length头部字段
    else if(strncasecmp(text,"Content-Length:",15)==0){
        text+=15;
        text+=strspn(text," \t");
        m_content_length=atol(text);
    }
    //处理Host头部字段
    else if(strncasecmp(text,"Host:",5)==0){
        text+=5;
        text+=strspn(text," \t");
        m_host=text;
    }
    return NO_REQUEST;
}