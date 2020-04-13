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

int create_epoll(){
    int nready,efd,res;

    /*tep:epoll_event参数 ep[]:epoll_wait参数 */
    struct epoll_event tep,ep[OPEN_MAX];

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
    listen(sockfd,20);
    
    /*创建epoll模型，efd指向红黑树根节点 */
    int efd=epoll_create(OPEN_MAX);
    if(efd<0){
        perror("epoll_create error");
    }

    /*指定lfd的监听时间为“读” */
    tep.events=EPOLLIN;tep.data.fd=sockfd;
    /*讲lfd及对应的结构体设置到树上，efd可以找到该树 */
    res=epoll_ctl(efd,EPOLL_CTL_ADD,sockfd,&tep);
    if(res<0){
        perror("epoll_ctl error");
    }

    return efd;
}

void handle_request(int fd){
    char buff[1000*1000]={0};
    int n=read(fd,buff,sizeof(buff));
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
        }

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