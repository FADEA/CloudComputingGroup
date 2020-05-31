#ifndef _PARTICIPANT_H
#define _PARTICIPANT_H

void *send_heart(void *arg);

int participant(char *cip,int cport,char *pip,int pport);

enum State{INIT,READY,ABORT,COMMIT};


#endif
