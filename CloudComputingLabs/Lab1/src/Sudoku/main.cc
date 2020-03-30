#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <iostream>
#include "sudoku.h"
#include <pthread.h>
#include <cstdio>
#include <semaphore.h>
#include <queue>


using namespace std;

bool (*solve)(int)=solve_sudoku_dancing_links;
char puzzle[128];
int total_solved = 0;
int total = 0;
FILE *fp;
sem_t full;
sem_t mutex;
sem_t empty;
int flag_one=0;

//bool (*solve)(int) = solve_sudoku_basic;
struct problem{
	int num;
	char str[128];
};
queue<problem> q;
struct result{
	int num;
	int re[81];
};
queue<result> r;
int64_t now()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000000 + tv.tv_usec;
}

void *read_thread(void *arg){
  while (fgets(puzzle, sizeof puzzle, fp) != NULL) {
     if (strlen(puzzle) >= N) {
      ++total;
	  problem pro;
	  pro.num=total;
	  //cout<<total<<" "<<puzzle;
	  strcpy(pro.str,puzzle);
	  sem_wait(&empty);
	  sem_wait(&mutex);
	  q.push(pro);
	//  cout<<total<<" "<<puzzle;
	  sem_post(&mutex);
	  sem_post(&full);
 	 }
  }
  sem_wait(&mutex);
  flag_one=1;
 // cout<<"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"<<endl;
  sem_post(&mutex);
}
void *solve_puzzle(void *arg){
		int flag=1;
		while(flag){
  	//		cout<<"abbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbaaaa"<<endl;
	//		sem_wait(&full);
			sem_wait(&mutex);
			if(flag_one&&q.empty()){
				sem_post(&mutex);
	//			cout<<"aaaa"<<endl;
				flag_one=0;
				flag=0;
				break;
			}
			sem_post(&mutex);
			sem_wait(&full);
			sem_wait(&mutex);
			problem pp=q.front();
			q.pop();
			sem_post(&mutex);
			sem_post(&empty);
      input(pp.str);
    //  init_cache();
      if (solve(0)) {
        ++total_solved;
		std::cout<<total_solved<<":"<<std::endl;
		for(int i=0;i<81;i++){
			std::cout<<*(board+i);
		}
		std::cout<<std::endl;
        if (!solved())
          assert(0);
      }
      else {
        printf("No: %s", puzzle);
      }

	}
}

int main(int argc, char* argv[])
{
	pthread_t read_puzzle;
	pthread_t solve_one;
	pthread_t solve_two;
	char s_temp[30];
	int file_num=0;
	char puzzle_file[10][30];
	while(fgets(s_temp,sizeof s_temp, stdin)!=NULL){
		if((s_temp[0]-'0')==-38)break;
		int i=0;
		for(i=0;i<strlen(s_temp)-1;i++){
			puzzle_file[file_num][i]=s_temp[i];
		}
		puzzle_file[file_num][i]='\0';
		file_num++;
	}
  int64_t start = now();
  int has_solved=0;
  while(has_solved++!=file_num){
		int flag_one=0;
		sem_init(&full,0,0);
		sem_init(&mutex,0,1);
		sem_init(&empty,0,10);
		fp=fopen(puzzle_file[has_solved-1],"r");
		pthread_create(&read_puzzle,NULL,read_thread,NULL);
		pthread_create(&solve_one,NULL,solve_puzzle,NULL);

		pthread_join(read_puzzle,NULL);
		pthread_join(solve_one,NULL);
  }
  int64_t end = now();
  double sec = (end-start)/1000000.0;
  printf("%f sec %f ms each %d\n", sec, 1000*sec/total, total_solved);
  return 0;
}

