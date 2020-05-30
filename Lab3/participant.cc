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

int fd;

map<string,string> database;
string key,value;
map<string,string>::iterator it;

void *send_heart(void *arg){
	cout<<"The heartbeat is sending\n";
	char heart[2]="!";
	while(1){
		send(fd,heart,sizeof(heart),0);
		sleep(3);
	}
	return NULL;
}

int participant(char *cip,int cport,char *pip,int pport){
	printf("%s:%d is waiting...\n",pip,pport);
	
	fd=Socket(AF_INET,SOCK_STREAM,0);
	int opt=1;
	setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	struct sockaddr_in localaddr;
	bzero(&localaddr,sizeof(localaddr));
	localaddr.sin_family=AF_INET;
	localaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	localaddr.sin_port=pport;
	
	Bind(fd,(struct sockaddr*)&localaddr,sizeof(localaddr));
	
	struct sockaddr_in serv_addr;
	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(cport);
	inet_pton(AF_INET,cip,&serv_addr.sin_addr.s_addr);

	Connect(fd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));

	pthread_t tid;
	int ret=pthread_create(&tid,NULL,send_heart,NULL);
	pthread_detach(tid);
	if(ret!=0){
		perr_exit("pthread create error");
	}
	char buf[BUFSIZ];
	int n;
	while(1){
		fgets(buf,sizeof(buf),stdin);	
		Write(fd,buf,strlen(buf));
		n=Read(fd,buf,sizeof(buf));
		Write(STDOUT_FILENO,buf,n);
	}
	return 0;
}

