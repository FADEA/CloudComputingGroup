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
#include <stack>
#include "wrap.h"
#include "participant.h"
#include <iostream>
using namespace std;

/*
 * set----0
 * get----1
 * del----2
 * abort--3
 * commit-4
 */
struct LE{
	int kind;
	string content;
};

int fd;
State state=INIT;
int coor_is_dead=0;
int count=0;
int opt=1;
int efd1;
int flag;

map<string,string> database;
string key,value;
map<string,string>::iterator it;

stack<LE> log;

struct sockaddr_in serv_addr;
struct sockaddr_in localaddr;
struct epoll_event event;

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
			int res=epoll_ctl(efd1,EPOLL_CTL_DEL,fd,NULL);
			if(res==-1){
				perr_exit("epoll_ctl del error");
			}
			Close(fd);
			fd=Socket(AF_INET,SOCK_STREAM,0);
			setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
			Bind(fd,(struct sockaddr*)&localaddr,sizeof(localaddr));
			char str[16];
			int clie_port=0;
			while(1){
				ct=connect(fd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
				cout<<fd<<" "<<ct<<endl;
				//inet_ntop(AF_INET,&serv_addr.sin_addr,str,sizeof(str));
				//clie_port=serv_addr.sin_port;
				//cout<<"haha "<<str<<":"<<ntohs(clie_port)<<endl;
				if(ct>=0){
					count=0;
					coor_is_dead=0;
					event.events=EPOLLIN|EPOLLET;
					flag = fcntl(fd, F_GETFL);          /* 修改connfd为非阻塞读 */
   				 	flag |= O_NONBLOCK;
    				fcntl(fd, F_SETFL, flag);
	
					event.data.fd=fd;
					epoll_ctl(efd1,EPOLL_CTL_ADD,fd,&event);
					break;
				}
				sleep(1);
				//continue;
			}
		}
		sleep(3);
	}

}

int participant(char *cip,int cport,char *pip,int pport){
	printf("%s:%d is waiting...\n",pip,pport);
	
	fd=Socket(AF_INET,SOCK_STREAM,0);
	//int opt=1;
	setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	//struct sockaddr_in localaddr;
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

	cout<<serv_addr.sin_port<<endl;
	
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

	//struct epoll_event event;
	struct epoll_event resevent[10];
	int res;int len;
	char buf[100];	
	memset(buf,0,sizeof(buf));
	efd1=epoll_create(10);
	/*
	 * 为什么下面改成水平触发以及阻塞读，在while中epoll_wait反而不阻塞了
	 */
	event.events=EPOLLIN|EPOLLET;

	flag = fcntl(fd, F_GETFL);          /* 修改connfd为非阻塞读 */
    flag |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flag);

	event.data.fd=fd;
	epoll_ctl(efd1,EPOLL_CTL_ADD,fd,&event);
	while(1){
		//printf("epoll_wait begin\n");
        res = epoll_wait(efd1, resevent, 10, -1);        //最多10个, 阻塞监听
       // printf("epoll_wait end res %d\n", res);
		if(resevent[0].data.fd==fd){
			memset(buf,0,sizeof(buf));
			len=Read(fd,buf,sizeof(buf));
			cout<<"len= "<<len<<endl;
			if(buf[0]=='!')count=0;//心跳包
			else{
				cout<<buf<<endl;
				/*
				if(buf[0]=='*'&&state==INIT){//set/get/del/abort/del
					LE le;
					char temp[30];
					int t=0;
					int kind;
					string content;
					kind=buf[1]-'0';
					if(kind==3||kind==4)continue;
					for(int i=2;buf[i]!='?';i++){
						temp[t++]=buf[i];
					}
					temp[t]='\0';
					content=temp;
					le.kind=kind;
					le.content=content;
					log.push(le);
					state=READY;
				}
				else if(buf[0]=='*'&&state==READY){
					int kind;
					string content;
					kind=buf[1]-'0';
					if(kind==3){
							
					}
					else if(kind==4){
						string key;
						string value;
						LE le=log.top();
					}
					else{
						continue;
					}
				}
				else if(buf[0]=='#'){//log
					
				}*/

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
