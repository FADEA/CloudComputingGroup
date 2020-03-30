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

sem_t empty;
sem_t full;
sem_t mutex;
char puzzle[128];
int prime=0;
int total_solved = 0;
int total = 0;
char* ar=NULL;
int num=0;

int64_t now()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000000 + tv.tv_usec;
}

void *producer(void* args){
	FILE* fp = fopen(ar, "r");
	while (prime==0){
		sem_wait(&empty);
		sem_wait(&mutex);
		if(fgets(puzzle, sizeof puzzle, fp)==NULL){prime=1;}
		sem_post(&mutex);
		sem_post(&full);	
	}
}
void *consumer(void* args){
	bool (*solve)(int) = solve_sudoku_dancing_links;
	while (prime==0){
		sem_wait(&full);
		sem_wait(&mutex);
		if (strlen(puzzle) >= N) {
		  //cout<<puzzle<<endl;
		  ++total;
		  input(puzzle);
		  init_cache();
		  if (solve(0)) {
		    ++total_solved;
            cout<<"num="<<num++<<":";
            for(int i=0;i<N;i++){
        			cout<<board[i];
        		}
            cout<<endl;
		    if (!solved())
		      assert(0);
		  }
		  else {
		    printf("No: %s", puzzle);
		  }
		}
        
		
		sem_post(&mutex);
		sem_post(&empty);	
	}
    sem_post(&mutex);
	sem_post(&full);
}

int main(int argc, char* argv[])
{
  init_neighbors();

  sem_init(&empty,0,1);
  sem_init(&full,0,0);
  sem_init(&mutex,0,1);
  pthread_t p;
  pthread_t c1,c2,c3;
  ar=argv[1];
  
  int64_t start = now();
  pthread_create(&p,NULL,producer,NULL);
  pthread_create(&c1,NULL,consumer,NULL);
  pthread_create(&c2,NULL,consumer,NULL);
  pthread_create(&c3,NULL,consumer,NULL);
  pthread_join(p,NULL);
  pthread_join(c1,NULL);
  cout<<"1"<<endl; 
  pthread_join(c2,NULL);
  cout<<"2"<<endl;
  pthread_join(c3,NULL);
  cout<<"3"<<endl;
  int64_t end = now();
  double sec = (end-start)/1000000.0;
  printf("%f sec %f ms each %d\n", sec, 1000*sec/total, total_solved);

  return 0;
}

