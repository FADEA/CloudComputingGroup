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
	bool (*solve)(int) = solve_sudoku_basic;
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
		    if (!solved())
		      assert(0);
		  }
		  else {
		    printf("No: %s", puzzle);
		  }
		}
		for(int i=0;i<N;i++){
			//if(i%9==0){cout<<endl;}
			cout<<board[i];
		}
		cout<<endl;
		sem_post(&mutex);
		sem_post(&empty);	
	}
}

int main(int argc, char* argv[])
{
  init_neighbors();

  sem_init(&empty,0,1);
  sem_init(&full,0,0);
  sem_init(&mutex,0,1);
  pthread_t p;
  pthread_t c;
  ar=argv[1];
  
  int64_t start = now();
  pthread_create(&p,NULL,producer,NULL);
  pthread_create(&c,NULL,consumer,NULL);
  pthread_join(p,NULL);
  pthread_join(c,NULL);
  int64_t end = now();
  double sec = (end-start)/1000000.0;
  printf("%f sec %f ms each %d\n", sec, 1000*sec/total, total_solved);

  return 0;
}

