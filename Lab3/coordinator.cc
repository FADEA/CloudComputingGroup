#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include <ctype.h>
#include <map>
#include "wrap.h"
#include <iostream>
#define MAXLINE 8192
#define OPEN_MAX 5000
using namespace std;


void *heart_handler(){

}

int coordinator(char *cip,int cport,char (*pip)[16],int pport[],int p){
	int listenfd,connfd,sockfd;
	int n,num;
	ssize_t nready,efd,res;
	char buf[MAXLINE],str[INET_ADDRSTRLEN];
	socklen_t clilen;

	struct sockaddr_in cliaddr,servaddr;
	struct epoll_event tep,ep[OPEN_MAX];

	listenfd=Socket(AF_INET,SOCK_STREAM,0);
	
	int opt=1;
	setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_familt=AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(cport);

	Bind(listenfd,(struct sockaddr *))&servaddr,sizeof(servaddr));
	Listen(listenfd,128);

	efp=epoll_create(OPEN_MAX);
	
	if(efp==-1){
		perr_exit("epoll_create error");
	}

	tep.event=EPOLLIN;
	tep.data.fd=listenfd;
	res=epoll_ctl(efd,EPOLL_CTL_ADD,listenfd,&tep);
	if(res==-1){
		perr_exit("epoll_ctl error");
	}
	while(1){
		nread
	}
	return 0;
}


