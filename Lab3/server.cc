#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>
#include "wrap.h"

#define MAXLINE 8192
#define SERV_PORT 6666

struct s_info{//定义一个结构体，将套接字与cfd绑定
	struct sockaddr_in cliaddr;
	int connfd;
};

void *do_work(void *arg){
	int n,i;
	struct s_info *ts =(struct s_info *)arg;//通过它参数提取出连接客户端套接字的信息
	char buf[MAXLINE];
	char str[INET_ADDRSTRLEN];//使用[+d可查看变量，为16
	
	while(1){
		n=Read(ts->connfd,buf,MAXLINE);//读取来自客户端的信息
		if(n==0){
			printf("the client %d is closed!\n",ts->connfd);
			break;
		}
		printf("received from %s at PORT %d\n",
						inet_ntop(AF_INET,&(*ts).cliaddr.sin_addr,str,sizeof(str)),
						ntohs((*ts).cliaddr.sin_port));
		for(i=0;i<n;i++){
			buf[i]=toupper(buf[i]);
		}
		Write(STDOUT_FILENO,buf,n);
		Write(ts->connfd,buf,n);
	}
	close(ts->connfd);
	return (void*)0;
}

int main(int argc,char *argv[]){
	struct sockaddr_in servaddr,cliaddr;
	socklen_t cliaddr_len;
	int listencfd,connfd;
	pthread_t tid;
	struct s_info ts[256];//定义一共可以创建多少盒线程
	int i=0;
	listencfd=Socket(AF_INET,SOCK_STREAM,0);//创建套接字用来建立连接

	int opt=1;
	setsockopt(listencfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));//端口复用

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(SERV_PORT);

	Bind(listencfd,(struct sockaddr*)&servaddr,sizeof(servaddr));//绑定
	Listen(listencfd,128);//设置同一时刻可以连接的客户端上线
	printf("Accepting client connect...\n");
	
	while(1){
		cliaddr_len=sizeof(cliaddr);
		connfd=Accept(listencfd,(struct sockaddr*)&cliaddr,&cliaddr_len);//阻塞监听客户端连接请求
		ts[i].cliaddr=cliaddr;
		ts[i].connfd=connfd;

		pthread_create(&tid,NULL,do_work,(void*)&ts[i]);
		pthread_detach(tid);//字线程分离，防止僵线程产生
		i++;
	}
	return 0;
}
