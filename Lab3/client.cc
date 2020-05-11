#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>

#define SERV_PORT 6666
#define SERV_IP "127.0.0.1"

int main(int argc,char *argv[]){
	int cfd;
	struct sockaddr_in serv_addr;

	char buf[BUFSIZ];
	int n;
	cfd=socket(AF_INET,SOCK_STREAM,0);

	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(SERV_PORT);
	inet_pton(AF_INET,SERV_IP,&serv_addr.sin_addr.s_addr);

	connect(cfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
	
	while(1){
		fgets(buf,sizeof(buf),stdin);	
		write(cfd,buf,strlen(buf));
		n=read(cfd,buf,sizeof(buf));
		write(STDOUT_FILENO,buf,n);
	}
	close(cfd);
	return 0;
}
