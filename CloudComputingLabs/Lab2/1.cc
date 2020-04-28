#include <sys/types.h>
#include <sys/socket.h> 
#include <iostream>
using namespace std;
int main()
{
/*1.创建监听套接字并将其绑定到端口*/
	/*创建嵌套字socket*/ 
	int domain=AF_INET;
	int type=SOCK_STREAM;/*TCP流*/ 
	int protocol=0;
	int sock_fd=socket(domain,ype,protocol);
	/*绑定bind函数 bind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen);*/
	struct sockaddr_in serv_addr; 
	memset(&serv_addr, sizeof(struct serv_addr)); 
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(6000); 
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  
	if(bind(sock_fd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in)) < 0)
	{
		perror("Bind Error!\n");
		exit(-1); 
	}
	/*监听listen函数 listen(int s, int backlog)*/
	#define backlog 5   
	if(listen(sock_fd, backlog) < 0)
	{ 
		perror("Listen Error!\n"); 
		exit(-1); 
	}
	
	/*接受accept函数 accept(int s, struct sockaddr *addr, socklen_t *addrlen);*/
	struct sockaddr_in client_addr;
	 while(1)
    {
        int len = sizeof(caddr);
        int c = accept(sock_fd,(struct sockaddr*)&client_addr, sizeof(struct sockaddr_in));

        if(c < 0)
        {
            perror("accpet error!\n"); 
			exit(-1); 
        }
        printf("accept(c = %d),ip:%s,port:%d\n",c,inet_ntoa(caddr.sin_addr),ntohs(caddr.sin_port));
        char buff[128] = {0};
        recv(c,buff,127,0);
        printf("buff = %s\n",buff);
        send(c,"ok",2,0);
 
        close(c);  
    }
	close(sock_fd);
} 
