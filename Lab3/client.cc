#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include "wrap.h"
#include "resp.h"
#include <ctype.h>
#include <string>
#include <iostream>


#define SERV_PORT 8001
#define SERV_IP "127.0.0.1"
using namespace std;
int main(int argc,char *argv[]){
	int cfd;
	struct sockaddr_in serv_addr;

	char buf[BUFSIZ];
	int n;
	cfd=Socket(AF_INET,SOCK_STREAM,0);

	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(SERV_PORT);
	inet_pton(AF_INET,SERV_IP,&serv_addr.sin_addr.s_addr);

	Connect(cfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
	
	string ss;
	int flag=0;
	while(1){
	//	cin>>ss;
	//	if(ss=="set")flag=1;
//		else if(ss=="get")flag=2;
//		else if(ss=="del")flag=3;
		fgets(buf,sizeof(buf),stdin);	
//		if(flag==1)
	//	Write(cfd,Rset(buf),strlen(Rset(buf)));
//		else if(flag==2)
		Write(cfd,Rget(buf),strlen(Rget(buf)));
//		else if(flag==3)
//		Write(cfd,Rdel(buf),strlen(Rdel(buf)));
	
	//	n=Read(cfd,buf,sizeof(buf));
	//	Write(STDOUT_FILENO,buf,n);
	}
	Close(cfd);
	return 0;
}
