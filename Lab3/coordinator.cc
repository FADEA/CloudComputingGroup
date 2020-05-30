#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include <ctype.h>
#include <map>
#include "wrap.h"
#include "coordinator.h"
#include <iostream>
#define MAXLINE 8192
#define OPEN_MAX 5000
using namespace std;

ssize_t efd;
map<int,IPC>mmap;

void *heart_handler(void* arg){
	printf("The heartbeat checking thread started.\n");
	while(1){
		map<int,IPC>::iterator it;
		for(it=mmap.begin();it!=mmap.end();){
			if(it->second.count==5){
				printf("The client %s:%d is offline.\n",it->second.ip,it->second.port);
				int fd=it->first;
				int res=epoll_ctl(efd,EPOLL_CTL_DEL,fd,NULL);
				if(res==-1){
					perr_exit("epoll del error.");
				}
				close(fd);
				mmap.erase(it++);
			}
			else if(it->second.count<5&&it->second.count>=0){
				
				it->second.count+=1;
				++it;
			}
			else{
				++it;
			}
		}
		sleep(3);
	}
}

int coordinator(char *cip,int cport,char (*pip)[16],int pport[],int p){
	/*
	strcpy(ipc.ip,cip);
	ipc.port=cport;
	ipc.count=0;
	mmap.insert(pair<int,IPC>(0,ipc));
	map<int,IPC>::iterator it;
	it=mmap.find(0);
	cout<<it->second.ip<<it->second.port<<it->second.count<<endl;
	*/

	int listenfd,connfd,sockfd;
	int n;
	ssize_t nready,res;
	char buf[MAXLINE],str[INET_ADDRSTRLEN];
	socklen_t clilen;

	struct sockaddr_in cliaddr,servaddr;
	struct epoll_event tep,ep[OPEN_MAX];

	listenfd=Socket(AF_INET,SOCK_STREAM,0);
	
	int opt=1;
	setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(cport);

	Bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	Listen(listenfd,128);

	efd=epoll_create(OPEN_MAX);//创建epoll模型，efd指向红黑树根结点
	
	if(efd==-1){
		perr_exit("epoll_create error");
	}
	
	//将listenfd放入红黑树中
	tep.events=EPOLLIN;
	tep.data.fd=listenfd;
	res=epoll_ctl(efd,EPOLL_CTL_ADD,listenfd,&tep);
	if(res==-1){
		perr_exit("epoll_ctl error");
	}
	pthread_t tid;
	int ret=pthread_create(&tid,NULL,heart_handler,NULL);
	if(ret!=0){
		perr_exit("pthread_create error.");
	}
	pthread_detach(tid);
	while(1){
		nready=epoll_wait(efd,ep,OPEN_MAX,-1);
		if(nready==-1){
			perr_exit("epoll_wait error");
		}
		for(int i=0;i<nready;i++){
			if(!(ep[i].events &EPOLLIN))
					continue;
			if(ep[i].data.fd==listenfd){
				int clie_port;
				clilen=sizeof(cliaddr);
				connfd=Accept(listenfd,(struct sockaddr*)&cliaddr,&clilen);
				inet_ntop(AF_INET,&cliaddr.sin_addr,str,sizeof(str));
				clie_port=cliaddr.sin_port;
				cout<<"haha"<<str<<":"<<clie_port<<endl;
				for(int i=0;i<p;i++){
					if((strcmp(str,pip[i])==0)&&(pport[i]==clie_port)){
						printf("%s:%d is add.\n",str,clie_port);
						IPC ipc;
						strcpy(ipc.ip,str);
						ipc.port=clie_port;
						ipc.count=0;
						mmap.insert(pair<int,IPC>(connfd,ipc));
					}
				}
				tep.events=EPOLLIN;tep.data.fd=connfd;
				res=epoll_ctl(efd,EPOLL_CTL_ADD,connfd,&tep);
				if(res==-1){
					perr_exit("epoll_ctl error");
				}
			}
			else{
				sockfd=ep[i].data.fd;
				n=Read(sockfd,buf,MAXLINE);
				if(n==0){
					if(mmap.find(sockfd)!=mmap.end())continue;
					res=epoll_ctl(efd,EPOLL_CTL_DEL,sockfd,NULL);
					if(res==-1){
						perr_exit("epoll_ctl error");
					}
					Close(sockfd);
					printf("%d is shutdown\n",sockfd);
				}
				else if(n<0){
					perror("read n<0 error");
					res=epoll_ctl(efd,EPOLL_CTL_DEL,sockfd,NULL);
					Close(sockfd);
				}
				else{
					if(buf[0]=='!'){
						map<int,IPC>::iterator it;
						it=mmap.find(sockfd);
						it->second.count=0;
						char heart[2]="!";
						Write(sockfd,heart,sizeof(heart));
						//mmap[sockfd].second.count=0;
						cout<<"received heart_beat from client\n";
					}
					else{
							/*
						 for (i = 0; i < n; i++)
                        buf[i] = toupper(buf[i]);   //转大写,写回给客户端

               		     Write(STDOUT_FILENO, buf, n);
                  		 Writen(sockfd, buf, n);
						*/
						map<int,IPC>::iterator ite;
						for(ite=mmap.begin();ite!=mmap.end();ite++){
							Write(ite->first,buf,n);
							sleep(3);
						}
						char End[2]="!";
						for(ite=mmap.begin();ite!=mmap.end();ite++){
							Write(ite->first,End,sizeof(End));
						}
						
					}
				}
			}
		}
	}
	Close(listenfd);
	Close(efd);
	return 0;
}


