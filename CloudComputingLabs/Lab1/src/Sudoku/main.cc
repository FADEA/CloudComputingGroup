#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <iostream>
#include "sudoku.h"
#include <semaphore.h>
#include <pthread.h>
using namespace std;

sem_t mutex;
sem_t empty;
sem_t full;
char puzzle[128];
FILE* fp;
int total_solved = 0;
int total = 0;

int64_t now() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

void *read(void* args) {
	while (fgets(puzzle, sizeof puzzle, fp) != NULL) {
		if (strlen(puzzle) >= N) {
			sem_wait(&empty);
			sem_wait(&mutex);
			sem_post(&mutex);
			sem_post(&full);
		}
	}
}

void *solve(void* args) {
	while (fgets(puzzle, sizeof puzzle, fp) != NULL) {
		sem_wait(&full);
		sem_wait(&mutex);
		if (strlen(puzzle) >= N) {
			++total;
			input(puzzle);
			init_cache();
			if (solve(0)) {
				++total_solved;
				for(int i=0; i<N; i++) {
					cout<<board[i];
				}
				cout<<endl;
				if (!solved())
					assert(0);
			} else {
				printf("No: %s", puzzle);
			}
		}
		sem_post(&mutex);
		sem_post(&empty);
	}
}
//信号量初始化
void init_sem() {
	sem_init(&mutex,0,1);
	sem_init(&full,0,0);
	sem_init(&empty,0,1);
}

int main(int argc, char* argv[]) {
	init_neighbors();
	FILE* fp = fopen(argv[1], "r");

	init_sem();
	pthread_t r;
	pthread_t s;

	int64_t start = now();
	pthread_create(&r,NULL,read,NULL);
	pthread_create(&s,NULL,solve,NULL);
	pthread_join(r,NULL);
	pthread_join(s,NULL);

	int64_t end = now();
	double sec = (end-start)/1000000.0;
	printf("%f sec %f ms each %d\n", sec, 1000*sec/total, total_solved);
	return 0;
}

