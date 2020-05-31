#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include <ctype.h>
#include <iostream>
#include "wrap.h"
#include "participant.h"
#include "coordinator.h"
using namespace std;

//协调者的ip的端口号
char CIP[16];
int CPORT;

//参与者的ip和端口号以及一共有多少个参与者
//如果配置文件是参与者的配置文件的话，那么第零个就是这个参与者的信息
char PIP[10][16];
int PPORT[10];
int P=0;

FILE *fp;

//将配置文件进行解析
//1->协调者
//2->参与者
//-1->配置文件错误
int parse_config(char *config_file){
	char cbuf[BUFSIZ];
	int flag=0;
	fp=fopen(config_file,"r");
	while(fgets(cbuf,sizeof(cbuf),fp)!=NULL){
		if(cbuf[0]=='!')continue;
		else if(strncmp(cbuf,"mode coordinator",16)==0){
			flag=1;
			continue;
		}
		else if(strncmp(cbuf,"mode participant",16)==0){
			flag=2;
			continue;
		}
		else if(strncmp(cbuf,"coordinator_info",16)==0){
			int t=0,i;
			char temp[10];
			for(i=17;;i++){
				if(cbuf[i]==':')break;
				CIP[t++]=cbuf[i];
			}
			CIP[t]='\0';
			t=0;
			for(i=i+1;i<(int)strlen(cbuf);i++){
				temp[t++]=cbuf[i];
			}
			temp[t]='\0';
			CPORT=atoi(temp);
			continue;
		}
		else if(strncmp(cbuf,"participant_info",16)==0){
			int t=0,i;
			char temp[10];
			for(i=17;;i++){
				if(cbuf[i]==':')break;
				PIP[P][t++]=cbuf[i];
			}
			PIP[P][t]='\0';
			t=0;
			for(i=i+1;i<(int)strlen(cbuf);i++){
				temp[t++]=cbuf[i];
			}
			temp[t]='\0';
			PPORT[P]=atoi(temp);
			P++;
			continue;
		}
		else{
			return -1;
		}
	}
	return flag;
}
int main(int argc,char *argv[]){
	char *config_file;
	int C_or_P=0;
	int success;
	if(argc!=3){
		printf("usage: %s --config_path config_path\n",basename(argv[0]));
		return 1;
	}
	else{
		if(strcmp(argv[1],"--config_path")==0){
			config_file=argv[2];
		}
		else{
			printf("usage: %s --config_path config_path\n",basename(argv[0]));
			return 1;
		}
	}
	C_or_P=parse_config(config_file);
	fflush(fp);
	fflush(stdin);
	fflush(stdout);
	if(C_or_P==-1){
		printf("%s error\n",basename(argv[2]));
		return 1;
	}
	else if(C_or_P==2){
		success=participant(CIP,CPORT,PIP[0],PPORT[0]);
	}
	else if(C_or_P==1){
		success=coordinator(CIP,CPORT,PIP,PPORT,P);
	}
	return 0;
}

