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
#include "threadpool.h"
#include <iostream>
#include <fcntl.h>
#define MAXLINE 100
#define OPEN_MAX 5000
using namespace std;

int reach[100];
int which=1;

ssize_t efd;
map<int,IPC>mmap;

struct info{
	int fd;
	int n;
	char bbuf[MAXLINE];
};

pthread_mutex_t map_lock;


void *to_participant(void *arg){
	info *inf=(info*)arg;
	map<int,IPC>::iterator ite;
	//cout<<inf->bbuf;
	int num=0;
	int i=0;
	char temp[50];int t=0;
	char how_much[10];int ht=0;
	memset(temp,0,sizeof(temp));
	memset(how_much,0,sizeof(how_much));
	for(i=0;inf->bbuf[i]!='\r';i++){
		if(inf->bbuf[i]>='0'&&inf->bbuf[i]<='9'){
			how_much[ht++]=inf->bbuf[i];
		}
	}
	num=atoi(how_much);
	//cout<<"num="<<num<<endl;
	
	temp[t++]='*';
	char fd_char[5];
	memset(fd_char,0,sizeof(fd_char));
	sprintf(fd_char,"%d",inf->fd);
	int fd_len=(int)strlen(fd_char);
	for(int j=0;j<fd_len;j++){
		temp[t++]=fd_char[j];
	}
	temp[t++]='|';
	int flag=2;
	while(flag){
		if(inf->bbuf[i++]=='\n')flag--;
	}	
	int method=0;
	if(inf->bbuf[i]=='S'){
		temp[t++]='0';
		method=1;
	}
	else if(inf->bbuf[i]=='G'){
		temp[t++]='1';
		method=2;
	}
	else if(inf->bbuf[i]=='D'){
		temp[t++]='2';
		method=3;
	}
	//cout<<"method="<<method<<endl;
	i=i+5;
	num--;
//	cout<<"aaa\n";
	while(num){
		if(inf->bbuf[i]=='$'){
			num--;
			char h[10];int hh=0;
			memset(h,0,sizeof(h));
			temp[t++]='|';
			for(i=i+1;;i++){
				if(inf->bbuf[i]>='0'&&inf->bbuf[i]<='9')h[hh++]=inf->bbuf[i];
				else break;
			}
		//	cout<<"bbbb\n";
			hh=atoi(h);
	//		cout<<hh<<endl;
			i=i+2;
	//		cout<<inf->bbuf[i]<<endl;
			int ti=i+hh;
			for(i=i;i<ti;i++){
				
				temp[t++]=inf->bbuf[i];
			}
		}
		else i=i+1;
		temp[t]='|';
	}
	//cout<<"fff "<<inf->bbuf[i]<<endl;
	/*
	while(num){
		if(inf->bbuf[i++]=='\r'){
			temp[t++]='|';
			num--;
		}
		else if((inf->bbuf[i]>='0'&&inf->bbuf[i]<='9')||(inf->bbuf[i]>='a'&&inf->bbuf[i]<='z')||(inf->bbuf[i]>='A'&&inf->bbuf[i]<='Z')){
			temp[t++]=inf->bbuf[i];
			i++;
		}
	}
	*/
	//cout<<"temp="<<temp<<endl;
	int has_participant=1;
	if(method==1||method==3){
		pthread_mutex_lock(&map_lock);
		reach[inf->fd]=mmap.size();
		if(reach[inf->fd]==0)has_participant=0;
		if(has_participant==1){
			//cout<<"reach="<<reach[inf->fd]<<endl;
			for(ite=mmap.begin();ite!=mmap.end();ite++){
				Write(ite->first,temp,sizeof(temp));
			}
			pthread_mutex_unlock(&map_lock);

		}
		else pthread_mutex_unlock(&map_lock);
	}
	else if(method==2){//get的是挂了的参与者没考虑
		pthread_mutex_lock(&map_lock);
		reach[inf->fd]=mmap.size();
		if(reach[inf->fd]==0)has_participant=0;
		if(has_participant==1){
			//cout<<"which="<<which<<" inf->fd="<<inf->fd<<endl;
			int wh=which % reach[inf->fd];
			which++;
		//	cout<<"wh="<<wh<<endl;
			for(ite=mmap.begin();ite!=mmap.end();ite++){
			//	cout<<"aaaa ite->first="<<ite->first<<endl;
				if(wh==0)break;
				wh--;
			}
		//	cout<<"ite->first="<<ite->first<<endl;
			Write(ite->first,temp,sizeof(temp));
			pthread_mutex_unlock(&map_lock);
		}
		else pthread_mutex_unlock(&map_lock);
		//cout<<"aaaaaa\n";
	}
	//cout<<"haha"<<endl;
	char error[9]="-ERROR\r\n";
	if(has_participant==0){
		Write(inf->fd,error,sizeof(error));
	}
	return NULL;
}

void *heart_handler(void* arg){
	printf("The heartbeat checking thread started.\n");
	while(1){
		map<int,IPC>::iterator it;
		pthread_mutex_lock(&map_lock);
		for(it=mmap.begin();it!=mmap.end();){
			if(it->second.count==5){
				printf("The participant %s:%d is offline.\n",it->second.ip,it->second.port);
				int fd=it->first;
				
				int res=epoll_ctl(efd,EPOLL_CTL_DEL,fd,NULL);
				if(res==-1){
					//perr_exit("epoll del error.");
					//printf("delete a null fd.\n");
					printf("cut participant bec network problem.\n");
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
		pthread_mutex_unlock(&map_lock);
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

	
    threadpool_t *thp = threadpool_create(3,100,100);
	printf("pool init.\n");
	if(pthread_mutex_init(&(map_lock),NULL)!=0){
		perr_exit("lock init failed.");
	}

	memset(reach,0,sizeof(reach));

	int listenfd,connfd,sockfd;
	int n;
	ssize_t nready,res;
	char buf[MAXLINE],str[INET_ADDRSTRLEN];
	memset(buf,0,sizeof(buf));
	//cout<<"No 0. "<<buf<<endl;
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
	
//	int flag;
//	tep.events=EPOLLIN|EPOLLET;
//	flag = fcntl(listenfd, F_GETFL);          /* 修改connfd为非阻塞读 */
//    flag |= O_NONBLOCK;
//    fcntl(listenfd, F_SETFL, flag);

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
	printf("into while(1)\n");
	while(1){
		nready=epoll_wait(efd,ep,OPEN_MAX,-1);
		//cout<<"3333\n";
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
				cout<<"client or participant "<<str<<":"<<clie_port<<endl;
				for(int i=0;i<p;i++){
					if((strcmp(str,pip[i])==0)&&(pport[i]==clie_port)){
						printf("%s:%d is add.\n",str,clie_port);
						IPC ipc;
						strcpy(ipc.ip,str);
						ipc.port=clie_port;
						ipc.count=0;
						pthread_mutex_lock(&map_lock);
						mmap.insert(pair<int,IPC>(connfd,ipc));
						pthread_mutex_unlock(&map_lock);
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
				memset(buf,0,sizeof(buf));
				n=Read(sockfd,buf,sizeof(buf));
				//cout<<"No1. "<<buf<<endl;
				if(n==0){
				//	pthread_mutex_lock(&map_lock);
				//	if(mmap.find(sockfd)!=mmap.end()){
					//	cout<<"22222\n";
						res=epoll_ctl(efd,EPOLL_CTL_DEL,sockfd,NULL);
						if(res==-1){
						perr_exit("epoll_ctl error");
						}
		//				pthread_mutex_unlock(&map_lock);
		//				continue;
		//			}
				//	cout<<"1111\n";
		//			pthread_mutex_unlock(&map_lock);
		//			res=epoll_ctl(efd,EPOLL_CTL_DEL,sockfd,NULL);
		//			if(res==-1){
		//				perr_exit("epoll_ctl error");
		//			}
					pthread_mutex_lock(&map_lock);
					if(mmap.find(sockfd)!=mmap.end()){
						pthread_mutex_unlock(&map_lock);
						continue;
					}
					pthread_mutex_unlock(&map_lock);
					Close(sockfd);
					printf("%d is shutdown\n",sockfd);
				}
				else if(n<0){
					perror("read n<0 error");
					res=epoll_ctl(efd,EPOLL_CTL_DEL,sockfd,NULL);
					if(res==-1){
						perr_exit("epoll_ctl n<0 error");
					}
					Close(sockfd);
				}
				else{
					if(buf[0]=='!'){
						map<int,IPC>::iterator it;
						pthread_mutex_lock(&map_lock);
						it=mmap.find(sockfd);
						it->second.count=0;
						pthread_mutex_unlock(&map_lock);
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
						info inf;
						inf.fd=sockfd;
						inf.n=n;
						strcpy(inf.bbuf,buf);
						threadpool_add(thp,to_participant,(void*)&inf);
						/*
						map<int,IPC>::iterator ite;
						cout<<buf;
						pthread_mutex_lock(&map_lock);
						for(ite=mmap.begin();ite!=mmap.end();ite++){
							Write(ite->first,buf,n);
						//	sleep(3);
						}
						pthread_mutex_unlock(&map_lock);
						*/
						//char End[2]="!";
						//for(ite=mmap.begin();ite!=mmap.end();ite++){
						//	Write(ite->first,End,sizeof(End));
						//}
						
					}
				}
			}
		}
	}
	Close(listenfd);
	Close(efd);
	return 0;
}


