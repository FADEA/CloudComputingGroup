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
int solve_flag=0;

pthread_cond_t emp;
pthread_cond_t ful;
pthread_mutex_t mut;
pthread_mutex_t mut2;



struct problem{
	int num;
	char str[128];
};
queue<problem> q;

struct result{
	int num;
	int re[81];
};

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
	  strcpy(pro.str,puzzle);
	 // sem_wait(&empty);
	 // sem_wait(&mutex);
	  pthread_mutex_lock(&mut);
	  q.push(pro);
	//  cout<<"input!!!!"<<endl;
	  pthread_cond_signal(&ful);
	  pthread_mutex_unlock(&mut);
	 // sem_post(&mutex);
	 //sem_post(&full);
 	 }
  }
 // sem_wait(&mutex);
  flag_one=1;
 // sem_post(&mutex);
}
void *solve_puzzle(void *arg){
		int flag=1;
/*****************
		while(flag){
			sem_wait(&mutex);
			if(flag_one&&q.empty()){
				sem_post(&mutex);
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
********************/
	 int read_end=1;
	 if(solve_flag)read_end=0;
	 while(read_end){
	//	cout<<"aaaaaaaaaaaaaaaaaaaaaaaa"<<endl;
		pthread_mutex_lock(&mut);
		//cout<<"abc "<<q.empty()<<endl;
		while(q.empty()){
	//		cout<<"cccccccccccccccccccccc"<<endl;
			if(flag_one||solve_flag){
			//	cout<<"dddddddd"<<endl;
				read_end=0;
	//			flag_one=0;
				solve_flag=1;
				break;
			}
	//		cout<<"bbbbbbbbbbbbb"<<endl;
			pthread_cond_wait(&ful,&mut);
		}
		if(!read_end){
			pthread_cond_signal(&ful);
			pthread_mutex_unlock(&mut);
			break;
		}
		problem pp=q.front();
		q.pop();
		pthread_mutex_unlock(&mut);
		pthread_mutex_lock(&mut2);
      input(pp.str);
    //  init_cache();
      if (solve(0)) {
        ++total_solved;
		std::cout<<total_solved<<":"<<std::endl;
		//cout<<pp.num<<endl;
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
	 pthread_mutex_unlock(&mut2);
	// pthread_mutex_unlock(&mut);
	}
}

int main(int argc, char* argv[])
{
	pthread_t read_puzzle;
	//pthread_t solve_one;
	//pthread_t solve_two;
	pthread_t solve_n[20];
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
		flag_one=0;
		solve_flag=0;
		q=queue<problem>();
	//	sem_init(&full,0,0);
	//	sem_init(&mutex,0,1);
	//	sem_init(&empty,0,10);
		fp=fopen(puzzle_file[has_solved-1],"r");
		pthread_create(&read_puzzle,NULL,read_thread,NULL);
	//	pthread_create(&solve_one,NULL,solve_puzzle,NULL);
	//	pthread_create(&solve_two,NULL,solve_puzzle,NULL);
		for(int i=0;i<10;i++){
			pthread_create(&solve_n[i],NULL,solve_puzzle,NULL);
		}
		pthread_join(read_puzzle,NULL);
//		pthread_join(solve_one,NULL);
		for(int i=0;i<10;i++){
			pthread_join(solve_n[i],NULL);
		}
//		pthread_join(solve_two,NULL);
  }
  int64_t end = now();
  double sec = (end-start)/1000000.0;
  printf("%f sec %f ms each %d\n", sec, 1000*sec/total, total_solved);
  return 0;
}

