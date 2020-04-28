#include "server.h"

int create_sockfd(){
    /*创建套接字 */
    int sockfd;
    sockfd=socket(AF_INET,SOCK_STREAM,0);

    /*设置服务器sockaddr_in结构 */
    sockaddr_in servaddr;
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(6001);       
    servaddr.sin_addr.s_addr=INADDR_ANY; //监听任意网卡

    /*防止重启服务器端口绑定失败 */
    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0){
        perror("setsockopet error\n");
        return -1;
    }
    /*绑定服务器端口和套接字 */
    if(bind(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0){
        perror("bind error");
        exit(1);
    }

    /*监听请求 */
    listen(sockfd,666);
    return sockfd;

}

/*创建epoll模型 */
int create_epoll(int sockfd){
    /*tep:epoll_event参数*/
    struct epoll_event tep;

    /*设置服务器sockaddr_in结构 */
    sockaddr_in servaddr;
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(6001);       
    servaddr.sin_addr.s_addr=INADDR_ANY; //监听任意网卡

    /*防止重启服务器端口绑定失败 */
    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0){
        perror("setsockopet error\n");
        return -1;
    }
    /*绑定服务器端口和套接字 */
    if(bind(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0){
        perror("bind error");
        exit(1);
    }

    /*监听请求 */
    listen(sockfd,20);
    
    /*创建epoll模型，efd指向红黑树根节点 */
    int efd=epoll_create(OPEN_MAX);
    if(efd<0){
        perror("epoll_create error");
    }

    /*指定sockfd的监听事件为“读” */
    tep.events=EPOLLIN;tep.data.fd=sockfd;

    /*将sockd及对应的结构体设置到树上，efd可以找到该树 */
    if(epoll_ctl(efd,EPOLL_CTL_ADD,sockfd,&tep)<0){
        perror("epoll_ctl error");
    }

    return efd;
}

int setnonblocking(int fd){
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

/*
 将文件描述符fd上的EPOLLIN注册到epollfd指示的epoll内核
 事件表中，参数oneshot指定是否注册fd上的EPOLLONESHOT事件
*/
// void addfd(int epollfd,int fd,bool oneshot){
//     epoll_event event;
//     event.data.fd=fd;
//     event.events=EPOLLIN | EPOLLET;
//     if(oneshot){
//         event.events |= EPOLLONESHOT;
//     }
//     epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
//     setnonblocking(fd);
// }

/*
重置fd上的事件，这样操作后，尽管fd上的EPOLLONESHOT事件被注册，但
是操作系统仍然会触发fd上的EPOLLIN事件，且只触发一次
*/
void reset_oneshot(int epollfd,int fd){
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&event);
}

/*ET模式的工作流程*/
void et(epoll_event* events,int number,int epollfd,int listenfd){
    char buf[BUFFER_SIZE];
    for(int i=0;i<number;i++){
        int sockfd=events[i].data.fd;
        if(sockfd==listenfd){
            int connfd=accept(listenfd,NULL,NULL);
            addfd(epollfd,connfd,true);
        }else if(events[i].events & EPOLLIN){
            /*这段代码不会被重复触发*/
            handle_request(sockfd);
            close(sockfd);
        }else{
            perror("something else happend\n");
        }
    }
}

void worker(void* arg){
    fds* fd = (fds*)arg;
    int sockfd=fd->sockfd;
    int epollfd=fd->epollfd;
    handle_request(sockfd);
}

/*请求处理函数*/
void handle_request(int fd){
    char buff[1000*1000]={0};
    int n=recv(fd,buff,sizeof(buff),0);
    // cout<<"n="<<n<<endl;
    cout<<"接收到的信息为:"<<endl;
    cout<<buff;

    /*获取请求方法 */
    string method;
    for(int i=0;buff[i]!=' ';i++){
        method.push_back(buff[i]);
    }
    
    /*响应报文缓存 */
    char response[1024*1024]={0};

    // cout<<"请求方法为："<<method<<"hhhh"<<endl;
    if(method=="GET"||method=="POST"){
        /*获取文件名 */
        char filename[100]={0};
        int count=0;
        for(int i=0;i<strlen(buff);i++){
            if(buff[i]=='/'){
                while(buff[i]!=' '){
                    i++;
                    filename[count++]=buff[i];
                }
                break;
            }
        }
        filename[count-1]='\0';
        // cout<<"文件名为："<<filename<<"hhhhh"<<endl;

        /*确定文件类型 */
        string fileType;
        if(strstr(filename,".html")){
            fileType="text/html"; //文件类型
        }else if(strstr(filename,".jpg")){
            fileType="image/jpg"; //图片类型
        }int n=read(fd,buff,sizeof(buff));
    // cout<<"n="<<n<<endl;

        /*读取文件 */
        // cout<<filename<<endl;
        int filefd=open(filename,O_RDONLY);
        if(filefd<0){
            perror("open error");
            /*设置响应头 */
            sprintf(response,"HTTP/1.1 404 Not Found\r\n");
        }else{
            /*设置响应头 */
            sprintf(response,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n",fileType.c_str());
            int headlen=strlen(response);

            /*读取文件 */
            int filelen=read(filefd,response+headlen,sizeof(response)-headlen);
            if(filelen<0){
                perror("read error");
            }
        }
        close(filefd);
    }else{
        /*设置响应头 */
        sprintf(response,"HTTP/1.1 501 Not Implemented\r\n");
    }
    
    /*讲响应报文写入套接字 */
    if(write(fd,response,strlen(response))<0){
        perror("write error");
    }
    
}