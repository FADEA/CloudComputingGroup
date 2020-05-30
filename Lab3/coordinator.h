#ifndef _COORDINATOR_H_
#define _COORDINATOR_H_
#include <map>

enum Type {HEART,OTHER};

struct PACKET_HEAD{
	Type type;
	int lenth;
};

struct IPC{
	char ip[16];
	int port;
	int count;
};

void *heart_handler(void *arg);

int coordinator(char *cip,int cport,char (*pip)[16],int pport[],int p);

#endif
