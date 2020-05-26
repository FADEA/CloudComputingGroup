#include <stdio.h>
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
using namespace std;


map<string,string> database;
string key,value;
map<string,string>::iterator iter;

int participant(char *cip,int cport,char *pip,int pport){
	printf("%s:%d is waiting...\n",pip,pport);
	
	int listenfd,connfd,sockfd;
	struct sockaddr_in cliaddr,servaddr;
	listenfd=Socket(AF_INET,SOCK_STREAM,0);
	
	int opt=1;
	setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(pport);
	Bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	Listen(listenfd,128);
	



	

	return 0;
}

