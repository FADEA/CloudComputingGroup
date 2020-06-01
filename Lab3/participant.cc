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
#include <vector>
#include <semaphore.h>
#include "wrap.h"
#include "participant.h"
#include "threadpool.h"
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
	int clifd;
	int kind;
	string content;
};

struct infor{
	int coorfd;
	char bbuf[50];
};

int fd;
//State state=INIT;
int coor_is_dead=0;
int count=0;
int opt=1;
int efd1;
int flag;

map<string,string> database;
//string key,value;
map<string,string>::iterator it;

vector<LE> log;

struct sockaddr_in serv_addr;
struct sockaddr_in localaddr;
struct epoll_event event;

pthread_mutex_t log_lock;
pthread_mutex_t data_lock;

int which_kind[100];
sem_t sem1[100];
sem_t sem2[100];

void *handle(void *arg){
	infor *inf=(infor*)arg;
//	cout<<inf->bbuf<<endl;
	State state=INIT;
	LE le;
	int i;int kind;
	string content;
	char clifd_char[5];int clifd=0;
	memset(clifd_char,0,sizeof(clifd_char));
	for(i=1;inf->bbuf[i]!='|';i++){
		clifd_char[clifd++]=inf->bbuf[i];
	}
	clifd=atoi(clifd_char);
	le.clifd=clifd;
	i=i+1;
	char temp[50];int tt=0;
	memset(temp,0,sizeof(temp));
	//cout<<"inf->bbuf[i]="<<inf->bbuf[i]<<endl;
	if(inf->bbuf[i]=='0'){
		kind=0;
	}
	else if(inf->bbuf[i]=='1'){
		kind=1;
	}
	else if(inf->bbuf[i]='2'){
		kind=2;
	}
	i=i+2;
	for(i=i;inf->bbuf[i]!='?';i++){
		temp[tt++]=inf->bbuf[i];
	}
	content=temp;
	if(kind==1){
	//	cout<<"get "<<content<<endl;
		string value;
		pthread_mutex_lock(&data_lock);
		map<string,string>::iterator getit=database.find(content);
		if(getit!=database.end()){
			value=getit->second;
			char val[25];
			strcpy(val,value.c_str());
			cout<<val<<endl;
			char get_char[35];
			memset(get_char,0,sizeof(get_char));
			get_char[0]=')';
			strcat(get_char,clifd_char);
			strcat(get_char,"|");
			strcat(get_char,val);
			Write(inf->coorfd,get_char,sizeof(get_char));
		}
		else{
			char get_char[10];
			memset(get_char,0,sizeof(get_char));
			get_char[0]='(';
			strcat(get_char,clifd_char);
			Write(inf->coorfd,get_char,sizeof(get_char));
		}
		pthread_mutex_unlock(&data_lock);
		return NULL;
	}
	else{
	le.kind=kind;
//	cout<<content<<endl;
	le.content=content;
	pthread_mutex_lock(&log_lock);
	log.push_back(le);
	pthread_mutex_unlock(&log_lock);
	char ret_to_c[10];
	memset(ret_to_c,0,sizeof(ret_to_c));
	ret_to_c[0]='@';
	strcat(ret_to_c,clifd_char);
	
/*	if(kind==2){
		strcat(ret_to_c,"|");
		int cou=0;
		int con_len=content.length();
		string key;
		for(int i=0;i<con_len;i++){	
			if(content[i]!='|'){
				key=key+content[i];
			}
			else if(content[i]=='|'){
				pthread_mutex_lock(&data_lock);
				map<string,string>::iterator mi=database.find(key);
				if(mi!=database.end())cou++;
				pthread_mutex_unlock(&data_lock);
				key.clear();
			}
		}
		pthread_mutex_lock(&data_lock);
		map<string,string>::iterator mi=database.find(key);
		if(mi!=database.end())cou++;
		pthread_mutex_unlock(&data_lock);
		key.clear();
		char cou_char[5];
		sprintf(cou_char,"%d",cou);
		strcat(ret_to_c,cou_char);
		strcat(ret_to_c,"|");
	}
*/
	Write(inf->coorfd,ret_to_c,sizeof(ret_to_c));
	sem_post(&sem1[clifd]);
	sem_wait(&sem2[clifd]);
//	cout<<which_kind[clifd]<<endl;
	if(which_kind[clifd]==3){
		vector<LE>::iterator lit;
		pthread_mutex_lock(&log_lock);
		for(lit=log.end()-1;;lit--){
			if(lit->content=="commit"&&lit->clifd==clifd)break;
			if(lit->clifd==clifd){
				log.erase(lit);
				break;
			}
			if(lit==log.begin())break;
		}
		pthread_mutex_unlock(&log_lock);
	}
	else if(which_kind[clifd]==4){
		vector<LE>::iterator lit;
		pthread_mutex_lock(&log_lock);
		for(lit=log.end()-1;;lit--){
			cout<<"content="<<lit->content<<" clifd="<<lit->clifd<<endl;
			if(lit->content=="commit"&&lit->clifd==clifd)break;
			string key;
			string value;
			int kv=0;int kv_len=lit->content.length();
			if(lit->kind==0){
					for(int i=0;i<kv_len;i++){
						if(kv==0&&lit->content[i]!='|'){
							key=key+lit->content[i];
						}
						else if(kv==0&&lit->content[i]=='|'){
							kv=1;
						}
						else if(kv==1){
							value=value+lit->content[i];
						}
					}
					pthread_mutex_lock(&data_lock);
					cout<<"key="<<key<<" value="<<value<<endl;
					database.insert(pair<string,string>(key,value));
					pthread_mutex_unlock(&data_lock);
					key.clear();value.clear();
			}
			else if(lit->kind==2){
				int count=0;
				for(int i=0;i<kv_len;i++){
					if(lit->content[i]!='|'){
						key=key+lit->content[i];
					}
					else if(lit->content[i]=='|'){
						pthread_mutex_lock(&data_lock);
						map<string,string>::iterator mi=database.find(key);
						if(mi!=database.end()){
							count++;
							database.erase(key);
						}
						pthread_mutex_unlock(&data_lock);
						key.clear();
					}
				}
				pthread_mutex_lock(&data_lock);
				map<string,string>::iterator mi=database.find(key);
				if(mi!=database.end()){
					count++;
					database.erase(key);
				}
				pthread_mutex_unlock(&data_lock);
				key.clear();
				char del_how[15];
				memset(del_how,0,sizeof(del_how));
				del_how[0]=':';
				char fd_char[4];
				sprintf(fd_char,"%d",lit->clifd);
				strcat(del_how,fd_char);
				char count_char[5];
				sprintf(count_char,"%d",count);
				strcat(del_how,"|");
				strcat(del_how,count_char);
				Write(inf->coorfd,del_how,sizeof(del_how));

			}
			if(lit==log.begin())break;
		}
		LE le_t;
		le_t.kind=4;
		le_t.clifd=clifd;
		le_t.content="commit";
		log.push_back(le_t);
		pthread_mutex_unlock(&log_lock);
	}




	}




	map<string,string>::iterator map_it;
	cout<<"------------------------\n";
	pthread_mutex_lock(&data_lock);
	for(map_it=database.begin();map_it!=database.end();map_it++){
		cout<<map_it->second<<endl;
	}
	pthread_mutex_unlock(&data_lock);
	cout<<"------------------------\n";
	which_kind[clifd]=0;
}


void *send_heart(void *arg){
	cout<<"The heartbeat is sending\n";
	char heart[2]="!";
	while(1){
		send(fd,heart,sizeof(heart),0);
		sleep(2);
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
		sleep(2);
	}

}

int participant(char *cip,int cport,char *pip,int pport){
	printf("%s:%d is waiting...\n",pip,pport);
	

	threadpool_t *thp = threadpool_create(3,100,100);
	printf("pool init.\n");
	if(pthread_mutex_init(&(log_lock),NULL)!=0){
		perr_exit("lock init failed.");
	}
	if(pthread_mutex_init(&(data_lock),NULL)!=0){
		perr_exit("lock init failed.");
	}
	memset(which_kind,0,sizeof(which_kind));
	for(int i=0;i<100;i++){
		sem_init(&sem1[i],0,0);
		sem_init(&sem2[i],0,0);
	}

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

	//cout<<serv_addr.sin_port<<endl;
	
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
			else if(buf[0]=='#'){
				cout<<buf<<endl;
				int i;
				for(i=1;;i++){
					if(buf[i]=='|')break;
				}
				i=i+1;
				if(buf[i]=='0'){
					i=i+2;
					string key;
					string value;
					for(i=i;;i++){
						if(buf[i]=='|')break;
						key=key+buf[i];
					}
					for(i=i+1;;i++){
						if(buf[i]=='?')break;
						value=value+buf[i];
					}
					cout<<"key="<<key<<" value="<<value<<endl;
					database.insert(pair<string,string>(key,value));
					key.clear();value.clear();					
				}
				else if(buf[i]=='2'){
					i=i+2;
					string key;
					for(i=i;;i++){
						if(buf[i]=='?'){
							database.erase(key);
							key.clear();
							break;
						}
						else if(buf[i]=='|'){
							database.erase(key);
							key.clear();
						}
						else{
							key=key+buf[i];
						}
					}
				}
			}
			else if(buf[0]=='^'){
				pthread_mutex_lock(&log_lock);
				int has_commit[100];int ht=0;
				memset(has_commit,0,sizeof(has_commit));
				vector<LE>::iterator lit;
				if(log.size()!=0){
						cout<<"log.size()="<<log.size()<<endl;
						for(lit=log.end()-1;;lit--){
							if(lit->content=="commit"){
								has_commit[ht++]=lit->clifd;
							}
							else{
								int flag=0;
								for(int i=0;i<ht;i++){
									if(lit->clifd==has_commit[i]){
										flag=1;
										break;
									}
								}
								if(flag==0){
									cout<<"erase a log.\n";
									log.erase(lit);
								}
							}
							if(lit==log.begin())break;
						}
				}
				pthread_mutex_unlock(&log_lock);
			}
			else if(buf[0]=='$'||buf[0]=='%'){
				//cout<<buf<<endl;
				char two_fd[5];int tft=0;
				memset(two_fd,0,sizeof(two_fd));
				for(int i=1;;i++){
					if(buf[i]>='0'&&buf[i]<='9')two_fd[tft++]=buf[i];
					else break;
				}
				tft=atoi(two_fd);
				sem_wait(&sem1[tft]);
				if(buf[0]=='$')
				which_kind[tft]=4;
				else if(buf[0]=='%')
				which_kind[tft]=3;
				sem_post(&sem2[tft]);
			}
	//		else if(buf[0]=='%'){
	//			//cout<<buf<<endl;
	//		}
			else if(buf[0]=='*'){
				//cout<<"hhhhhh "<<buf<<endl;
				infor inf;
				inf.coorfd=fd;
				strcpy(inf.bbuf,buf);
				threadpool_add(thp,handle,(void*)&inf);
				/*
				cout<<buf<<endl;
				LE le;
				int i;int kind;
				char clifd_char[5];int clifd=0;
				memset(clifd_char,0,sizeof(clifd_char));
				for(i=1;buf[i]!='|';i++){
					clifd_char[clifd++]=buf[i];
				}
				clifd=atoi(clifd_char);
				le.clifd=clifd;
				i=i+1;
				if(buf[i]=='0'){
					kind=0;
					state=READY;

				}
				else if(buf[i]=='1'){
					kind=1;
					state=READY;
				}
				else if(buf[i]='2'){
					kind=2;
					state=READY;
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

