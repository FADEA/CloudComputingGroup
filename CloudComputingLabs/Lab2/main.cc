#include "server.h"


int main(){
    /*创建套接字 */
    int sockfd;
    sockfd=socket(AF_INET,SOCK_STREAM,0);

    int listenfd=create_sockfd();
    
    /*tep:epoll_event参数 ep[]:epoll_wait参数 */
    struct epoll_event events[OPEN_MAX];

    int epollfd=epoll_create(5);
    if(epollfd<0){
        perror("epoll_create error\n");
    }
    addfd(epollfd,listenfd,true);
    
    while(true){
        /*阻塞监听事件 */
        int nready=epoll_wait(epollfd,events,OPEN_MAX,-1);
        if(nready==-1){
            perror("epoll_wait errpr");
        }

        et(events,nready,epollfd,listenfd);
    }
}