#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>
#include <iostream>
using namespace std;

#include "locker.h"
#include "threadpool.h"
#include "http_conn.h"

#define MAX_FD 65536
#define MAX_EVENT_NUMBER 10000

extern int addfd(int epollfd,int fd,bool one_shot);
extern int removefd(int epollfd,int fd);

void addsig(int sig,void( handler )(int),bool restart=true){
    struct sigaction sa;
    memset(&sa,'\0',sizeof(sa));
    sa.sa_handler=handler;
    if(restart){
        sa.sa_flags |= SA_RESTART;
    }
}

void show_error(int connfd,const char* info){
    printf("%s",info);
    send(connfd,info,strlen(info),0);
    close(connfd);
}

int main(int argc,char* argv[]){
   
    // const char* ip= argv[1];
    int port=6001;

    //忽略SIGPIPE信号
    addsig(SIGPIPE,SIG_IGN);

    //创建线程池
    threadpool<http_conn>* pool = NULL;
    try{
        pool=new threadpool<http_conn>;
    }catch( ... ){
        return 1;
    }

    //预先为每个可能的客户连接分配一个http_conn对象
    http_conn* users=new http_conn[MAX_FD];
    assert(users);
    int user_count=0;

    int listenfd=socket(AF_INET,SOCK_STREAM,0);
    if(listenfd<0){
        perror("socket create error\n");
    }

    /*防止重启服务器端口绑定失败 */
    int reuse = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0){
        perror("setsockopet error\n");
        return -1;
    }

    int ret=0;
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family=AF_INET;
    // inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port=htons(port);
    address.sin_addr.s_addr=INADDR_ANY;

    ret=bind(listenfd,(struct sockaddr*)&address,sizeof(address));
    if(ret<0){
        perror("bind error\n");
    }

    ret=listen(listenfd,5);
    if(ret<0){
        perror("listen error\n");
    }

    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd=epoll_create(5);
    if(epollfd<0){
        perror("epoll_create error\n");
    }
    addfd(epollfd,listenfd,true);
    http_conn::m_epollfd=epollfd;

    while(true){
        // cout<<"get here"<<endl;
        printf("wait for connection...\n");
        int number=epoll_wait(epollfd,events,MAX_EVENT_NUMBER,-1);
        if((number<0)&&(errno!=EINTR)){
            perror("epoll failure\n");
            break;
        }
        // cout<<"number="<<number<<endl;
        for(int i=0;i<number;i++){
            int sockfd=events[i].data.fd;
            if(sockfd==listenfd){
                struct sockaddr_in client_address;
                socklen_t client_addrlength=sizeof(client_address);
                int conn=accept(listenfd,(struct sockaddr*)&client_address,&client_addrlength);
                if(conn<0){
                    perror("accept error\n");
                    continue;
                }
                if(http_conn::m_user_count>=MAX_FD){
                    show_error(conn,"Internal server busy");
                    continue;
                }
                // cout<<inet_net_ntop(client_address.sin_addr.s_addr)<<endl;
                users[conn].init(conn,client_address);
            }
            else if(events[i].events&(EPOLLRDHUP|EPOLLHUP|EPOLLERR)){
                //如果有异常，直接关闭客户端连接
                users[sockfd].close_conn();
            }
            else if(events[i].events&EPOLLIN){
                //根据读的结果，决定将任务添加到线程池还是关闭连接
                
                if(users[sockfd].read()){
                    // cout<<"get epollin"<<endl;
                    pool->append(users+sockfd);
                }else{
                    users[sockfd].close_conn();
                }
            }
            else if(events[i].events&EPOLLOUT){
                //根据写的结果，决定是否关闭连接
                if(!users[sockfd].write()){
                    users[sockfd].close_conn();
                }
            }
        }
    }
    close(epollfd);
    close(listenfd);
    delete [] users;
    delete pool;
    return 0;
}