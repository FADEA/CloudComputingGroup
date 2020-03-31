#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include "sudoku.h"
#include <iostream>
using namespace std;

typedef struct _rwlock_t{
    sem_t lock;
    sem_t writelock;
    int readers;
}rwlock_t;

void rwlock_init(rwlock_t *rw){
    rw->readers=0;
    sem_init(&rw->lock,0,1);
    sem_init(&rw->writelock,0,1);
}

void rwlock_acquire_readlock(rwlock_t *rw){
    sem_wait(&rw->lock);
    rw->readers++;
    if(rw->readers==1){
        sem_wait(&rw->writelock);
    }
    sem_post(&rw->lock);
}

void rwlock_release_readlock(rwlock_t *rw){
    sem_wait(&rw->lock);
    rw->readers--;
    if(rw->readers==0){
        sem_post(&rw->writelock);
    }
    sem_post(&rw->lock);
}

void rwlock_acquire_writelock(rwlock_t *rw){
    sem_wait(&rw->writelock);
}

void rwlock_release_writelock(rwlock_t *rw){
    sem_post(&rw->writelock);

}

typedef struct _importation{
    int num;
    char puzzle[128];
}importation_t;


sem_t empty;
sem_t full;
sem_t mutex;
int prime=0;
int total_solved = 0;
int total = 0;
char* ar=NULL;
int num=0;
int get=0;
int pull=0;
int result[1000][81];
importation_t Input[10];
rwlock_t* rw=new rwlock_t;

int64_t now()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000000 + tv.tv_usec;
}

void *producer(void* args){
	FILE* fp = fopen(ar, "r");
    
	while (prime==0){
		rwlock_acquire_writelock(rw);
        int i=0;
        pull=0;get=0;
        while(i<10){
            if(fgets(Input[i].puzzle, sizeof(Input[i].puzzle), fp)==NULL){
                prime=1;
                break;
            }else{
                cout<<"num="<<num<<":"<<Input[i].puzzle;
                Input[i].num=num;
                pull++;num++;            
            }
            cout<<"i="<<i<<endl;
            i++;
            
        }
		rwlock_release_writelock(rw);
	}
}
void *consumer(void* args){
    cout<<"hhhhhh1"<<endl;
	bool (*solve)(int*) = solve_sudoku_dancing_links;
    int a[N];
	while (prime==0){
		rwlock_acquire_readlock;
        int g=get;get++;
		if (strlen(Input[g].puzzle) >= N&&g<=pull) {
		  ++total;
		  input(Input[g].puzzle,a);
		  if (solve(a)) {
		    ++total_solved;
            for(int i=0;i<N;i++){
        			result[Input[g].num][i]=a[i];
                cout<<a[i];
        		}
             cout<<endl;
		    if (!solved())
		      assert(0);
		  }
		  else {
		    printf("No: %s", Input[g].puzzle);
		  }
		}
		rwlock_release_readlock;	
	}
}

int main(int argc, char* argv[])
{
  init_neighbors();
  
  sem_init(&empty,0,1);
  sem_init(&full,0,0);
  sem_init(&mutex,0,1);
  rwlock_init(rw);
  
  pthread_t p;
  pthread_t c[4];
  ar=argv[1];
  
  int64_t start = now();
  
  pthread_create(&p,NULL,producer,NULL);
  for(int i=0;i<4;i++){
      pthread_create(&c[i],NULL,consumer,NULL);
  }
  pthread_join(p,NULL);
  for(int i=0;i<4;i++){
      pthread_join(c[i],NULL);
  }
  int64_t end = now();
  double sec = (end-start)/1000000.0;
  for(int i=0;i<num;i++){
    cout<<"num="<<i<<":";
    for(int j=0;j<81;j++){
        cout<<result[i][j];    
    }
    cout<<endl;
  }
  printf("%f sec %f ms each %d\n", sec, 1000*sec/total, total_solved);

  return 0;
}

