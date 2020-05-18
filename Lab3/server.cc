#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <ctype.h>
#include <arpa/inet.h>
#include "wrap.h"
#define SERV_PORT 6666


int main(int argc,char *argv[]){
	int lfd,cfd;
	struct sockaddr_in serv_addr,clie_addr;
	socklen_t clie_addr_len;
	char buf[BUFSIZ];
	int n;

	lfd=Socket(AF_INET,SOCK_STREAM,0); //创建套接字用来连接客户端
	
	serv_addr.sin_family =AF_INET;
	serv_addr.sin_port=htons(SERV_PORT);
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);

	Bind(lfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
	Listen(lfd,128);

	clie_addr_len=sizeof(clie_addr);
	cfd=Accept(lfd,(struct sockaddr *)&clie_addr,&clie_addr_len);

	while(1){
		n=Read(cfd,buf,sizeof(buf));
		for(int i=0;i<n;i++){
			buf[i]=toupper(buf[i]);
		}
		Write(cfd,buf,n);
	}
	Close(cfd);
	Close(lfd);
	return 0;
}
