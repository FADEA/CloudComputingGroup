#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <map>
#include "wrap.h"
#include <iostream>
using namespace std;

int fd;
int state=0;
int coor_is_dead=0;
int count=0;

map<string,string> database;
string key,value;
map<string,string>::iterator it;

struct sockaddr_in serv_addr;

void *send_heart(void *arg){
	cout<<"The heartbeat is sending\n";
	char heart[2]="!";
	while(1){
		send(fd,heart,sizeof(heart),0);
		sleep(3);
	}
	return NULL;
}

void *add_count(void *arg){
	int ct;
	while(1){
		if(!coor_is_dead){
			if(count==5){
				coor_is_dead=1;
			}
			else if(count>=0&&count<5){
				count=count+1;
			}
		}
		else{
			char str[16];
			ct=connect(fd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
			cout<<fd<<" "<<ct<<endl;
		
			int clie_port;
			inet_ntop(AF_INET,&serv_addr.sin_addr,str,sizeof(str));
			clie_port=serv_addr.sin_port;
			cout<<"haha"<<str<<":"<<ntohs(clie_port)<<endl;
		
			if(ct>=0){
				count=0;
				coor_is_dead=0;
			}
			sleep(1);
			continue;
		}
		sleep(3);
	}

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
	
	//struct sockaddr_in serv_addr;
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

	pthread_t tid2;
	ret=pthread_create(&tid2,NULL,add_count,NULL);
	pthread_detach(tid2);
	if(ret!=0){
		perr_exit("pthread create error");
	}

	struct epoll_event event;
	struct epoll_event resevent[10];
	int efd;int flag;int res;int len;
	char buf[BUFSIZ];

	efd=epoll_create(10);
	event.events=EPOLLIN|EPOLLET;

	flag = fcntl(fd, F_GETFL);          /* 修改connfd为非阻塞读 */
    flag |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flag);

	event.data.fd=fd;
	epoll_ctl(efd,EPOLL_CTL_ADD,fd,&event);
	while(1){
		printf("epoll_wait begin\n");
        res = epoll_wait(efd, resevent, 10, -1);        //最多10个, 阻塞监听
        printf("epoll_wait end res %d\n", res);
		if(resevent[0].data.fd==fd){
			len=Read(fd,buf,sizeof(buf));
			if(buf[0]=='!')count=0;
			else{
				cout<<buf<<endl;
			}
		}	
//       if (resevent[0].data.fd == fd) {
 //           while (1){    //非阻塞读, 轮询
//				len = Read(fd, buf,sizeof(buf) );
				//if(buf[0]=='!')break;
  //              write(STDOUT_FILENO, buf, len);
//		   	}	
			
//		}
	}

	/*
	char buf[BUFSIZ];
	int n;
	while(1){
		fgets(buf,sizeof(buf),stdin);	
		Write(fd,buf,strlen(buf));
		n=Read(fd,buf,sizeof(buf));
		Write(STDOUT_FILENO,buf,n);
	}
	*/
	return 0;
}

