#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <ctype.h>
#include <arpa/inet.h>
#define SERV_PORT 6666

int main(int argc,char *argv[]){
	int lfd,cfd;
	struct sockaddr_in serv_addr,clie_addr;
	socklen_t clie_addr_len;
	char buf[BUFSIZ];
	int n;

	lfd=socket(AF_INET,SOCK_STREAM,0);
	
	serv_addr.sin_family =AF_INET;
	serv_addr.sin_port=htons(SERV_PORT);
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);

	bind(lfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
	listen(lfd,128);

	clie_addr_len=sizeof(clie_addr);
	cfd=accept(lfd,(struct sockaddr *)&clie_addr,&clie_addr_len);

	while(1){
		n=read(cfd,buf,sizeof(buf));
		for(int i=0;i<n;i++){
			buf[i]=toupper(buf[i]);
		}
		write(cfd,buf,n);
	}
	close(cfd);
	close(lfd);
	return 0;
}
