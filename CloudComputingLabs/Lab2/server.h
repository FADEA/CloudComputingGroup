#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <iostream>
#include <fstream> 
#include <fcntl.h>
#include <sys/epoll.h>
using namespace std;

#define OPEN_MAX 1024
#define MAX_LINE 8192
#define SERV_PORT 6001
#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 10

int create_sockfd();
int create_epoll(int);
void handle_request(int);
int setnonblocking(int fd);
void addfd(int,int,bool);
void reset_oneshot(int,int);
void et(epoll_event*,int,int,int);

struct fds{
    int epollfd;
    int sockfd;
};

