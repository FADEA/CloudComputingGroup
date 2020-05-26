#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>



char *Rdel(char *ptr){
	static char str[100];
	char s[10];
	memset(str,0,sizeof(str));
	int space_position[10],space_num=0;
	int ptr_len=(int)strlen(ptr);
	int t=0;
	int ws=2;
	space_position[space_num++]=0;
	for(int i=0;i<ptr_len;i++){
		if(ptr[i]==32){
			space_position[space_num++]=i;
		}
	}	
	space_position[space_num]=ptr_len-1;
	str[t++]='*';
	sprintf(s,"%d",space_num);
	strcat(str,s);
	strcat(str,"\r\n$3\r\nDEL");
	t=(int)strlen(str);
	for(int i=3;i<ptr_len;i++){
		t=(int)strlen(str);
		char ts[10];
		if(ptr[i]==32){
			strcat(str,"\r\n$");
			sprintf(ts,"%d",space_position[ws]-space_position[ws-1]-1);
			ws++;
			strcat(str,ts);
			strcat(str,"\r\n");
			continue;
		}
		else{
			str[t]=ptr[i];
			continue;
		}
	}
	return str;
}


char *Rget(char *ptr){
	static char str[100];
	char s[10];
	memset(str,0,sizeof(str));
	int space_position[10],space_num=0;
	int ptr_len=(int)strlen(ptr);
	int t=0;
	int ws=2;
	space_position[space_num++]=0;
	for(int i=0;i<ptr_len;i++){
		if(ptr[i]==32){
			space_position[space_num++]=i;
		}
	}	
	space_position[space_num]=ptr_len-1;
	str[t++]='*';
	sprintf(s,"%d",space_num);
	strcat(str,s);
	strcat(str,"\r\n$3\r\nGET");
	t=(int)strlen(str);
	for(int i=3;i<ptr_len;i++){
		t=(int)strlen(str);
		char ts[10];
		if(ptr[i]==32){
			strcat(str,"\r\n$");
			sprintf(ts,"%d",space_position[ws]-space_position[ws-1]-1);
			ws++;
			strcat(str,ts);
			strcat(str,"\r\n");
			continue;
		}
		else{
			str[t]=ptr[i];
			continue;
		}
	}
	return str;
}

char *Rset(char *ptr){
	static char str[100];
	char s[10];
	memset(str,0,sizeof(str));
	int space_position[10],space_num=0;
	int ptr_len=(int)strlen(ptr);
	int t=0;
	int ws=2;
	space_position[space_num++]=0;
	for(int i=0;i<ptr_len;i++){
		if(ptr[i]==32){
			space_position[space_num++]=i;
		}
	}	
	space_position[space_num]=ptr_len-2;
	str[t++]='*';
	sprintf(s,"%d",space_num);
	strcat(str,s);
	strcat(str,"\r\n$3\r\nSET");
	t=(int)strlen(str);
	for(int i=3;i<ptr_len;i++){
		t=(int)strlen(str);
		char ts[10];
		if(ptr[i]==32&&ptr[i+1]!='"'){
			strcat(str,"\r\n$");
			sprintf(ts,"%d",space_position[ws]-space_position[ws-1]-1);
			ws++;
			strcat(str,ts);
			strcat(str,"\r\n");
			continue;
		}
		else if(ptr[i]==32&&ptr[i+1]=='"'){
			strcat(str,"\r\n$");
			sprintf(ts,"%d",space_position[ws]-space_position[ws-1]-2);
			ws++;
			strcat(str,ts);
			strcat(str,"\r\n");
			continue;
		}
		else if(ptr[i]!='"'){
			str[t]=ptr[i];
			continue;
		}
	}
	return str;
}

int main(){
	char buf[BUFSIZ];
	fgets(buf,sizeof(buf),stdin);
	printf("%s\n",Rdel(buf));
	return 0;
}
