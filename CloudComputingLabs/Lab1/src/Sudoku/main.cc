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
			sem_post(&empty);
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

int main(int argc, char* argv[]) {
	init_neighbors();

	FILE* fp = fopen(argv[1], "r");


	bool (*solve)(int) = solve_sudoku_basic;
	if (argv[2] != NULL)
		if (argv[2][0] == 'a')
			solve = solve_sudoku_min_arity;
		else if (argv[2][0] == 'c')
			solve = solve_sudoku_min_arity_cache;
		else if (argv[2][0] == 'd')
			solve = solve_sudoku_dancing_links;
	int64_t start = now();
	while (fgets(puzzle, sizeof puzzle, fp) != NULL) {
		if (strlen(puzzle) >= N) {
			++total;
			input(puzzle);
			init_cache();
			//if (solve_sudoku_min_arity_cache(0)) {
			//if (solve_sudoku_min_arity(0))
			//if (solve_sudoku_basic(0)) {
			if (solve(0)) {
				++total_solved;
				if (!solved())
					assert(0);
			} else {
				printf("No: %s", puzzle);
			}
		}
	}
	int64_t end = now();
	double sec = (end-start)/1000000.0;
	printf("%f sec %f ms each %d\n", sec, 1000*sec/total, total_solved);

	return 0;
}

