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

    /*绑定服务器端口和套接字 */
    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0){
        perror("setsockopet error\n");
        return -1;
    }
    if(bind(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0){
        perror("bind error");
        exit(1);
    }

    /*监听请求 */
    listen(sockfd,666);
    return sockfd;

}
void handle_request(int fd){
    char buff[1000*1000]={0};
    int n=read(fd,buff,sizeof(buff));
    // cout<<"n="<<n<<endl;
    cout<<"接收到的信息为:"<<endl;
    cout<<buff<<endl;
}