#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <error.h>
#include "threadpool.h"

#define DEFAULT_TIME 10 /*十秒管理线程检测一次线程池是否需要增加或者销毁线程*/
#define MIN_WAIT_TASK_NUM 10  /*如果任务队列的的大小要大于这个数则说明需要有新的线程*/
#define DEFAULT_THREAD_VARY 10 /*每次创建和销毁的线程数*/
#define true 1
#define false 0

typedef struct{
	void *(*function)(void * );//回调函数
	void *arg;//回调函数的参数
}threadpool_task_t;//各个子线程的任务结构体

//描述线程池的相关信息
struct threadpool_t {
	pthread_mutex_t lock;//用于锁住整个结构体
	pthread_mutex_t thread_counter;//锁住busy_thr_num
	pthread_cond_t queue_not_full;//条件变量，当任务队列不为满是可以往任务队列中继续添加任务
	pthread_cond_t queue_not_empty;//条件变量，当任务队列不为空时可以通知线程池中的线程处理任务

	pthread_t *threads;//线程数组
	pthread_t adjust_tid;//管理线程的tid
	threadpool_task_t *task_queue;//任务队列

	int min_thr_num;//线程池最小值
	int max_thr_num;//线程池最大值
	int live_thr_num;//当前存在的线程
	int busy_thr_num;//忙碌的线程
	int wait_exit_thr_num;//等待销毁的线程

	int queue_front;//任务队列头
	int queue_rear;//任务队列尾
	int queue_size;//任务队列实际大小
	int queue_max_size;//任务队列最大值

	int shutdown;//是否关闭线程池
};

void *threadpool_thread(void *threadpool);//线程池工作线程处理函数

void *adjust_thread(void *threadpool);//管理线程处理函数

int is_thread_alive(pthread_t tid);//检查一个线程是否活跃

int threadpool_free(threadpool_t *pool);//使用线程池的内存空间

threadpool_t *threadpool_create(int min_thr_num,int max_thr_num,int queue_max_size){
	int i;
	threadpool_t *pool=NULL;
	do{
		if((pool=(threadpool_t*)malloc(sizeof(threadpool_t)))==NULL){
			printf("malloc threadpool failed!\n");
			break;
		}
	pool->min_thr_num=min_thr_num;
	pool->max_thr_num=max_thr_num;
	pool->live_thr_num=min_thr_num;
	pool->busy_thr_num=0;
	pool->wait_exit_thr_num=0;
	pool->queue_size=0;
	pool->queue_max_size=queue_max_size;
	pool->front=0;
	pool->rear=0;
	pool->shutdown=false;

	pool->threads=(pthread_t *)malloc(sizeof(pthread_t)*max_thr_num);
	if(pool->threads==NULL){
		printf("malloc threads failed\n");
		break;
	}
	memset(pool->threads,0,sizeof(pthread_t)*max_thr_num);

	pool->task_queue=(threadpool_task_t *)malloc(sizeof(threadpool_task_t)*queue_max_size);
	if(pool->task_queue==NULL){
		printf("malloc task_queue failed\n");
		break;
	}

	if(pthread_mutex_init(&(pool->lock),NULL)!=0
					||pthread_mutex_init(&(pool->thread_counter),NULL)!=0
					||pthread_cond_init(&(pool->queue_not_full),NULL)!=0
					||pthread_cond_init(&(pool->queue_not_empty),NULL)!=0){
		printf("init lock or cond failed\n");
		break;
	}

	for(i=0;i<min_thr_num;i++){
		pthread_create(&(pool->threads[i]),NULL,threadpool_thread,(void*)pool);
		printf("start thread 0x%x...\n",(unsigned int)pool->threads[i]);
	}
	pthread_create(&(pool->adjust_tid),NULL,adjust_thread,(void*)pool);
	return pool;
	}while(0)

	threadpool_free(pool);
	return NULL;
}

int threadpool_add(threadpool_t *pool,void *(*function)(void *arg),void *arg){
	pthread_mutex_lock(&(pool->lock));
	while((pool->queue_size==pool->queue_max_size)&&(!pool->shutdown)){
		pthread_cond_wait(&pool->queue_not_full,&(pool->lock));
	}	
	if(pool->shutdown){
		pthread_mutex_unlock(&(pool->lock));
	}
	if(pool->task_queue[pool->queue_rear].arg!=NULL){
		free(pool->task_queue[pool->queue.rear].arg);
		pool->task_queue[pool->queue.read].arg=NULL;
	}
	
	pool->task_queue[pool->queue_rear].function=function;
	pool->task_queue[pool->queue_rear].arg=arg;
	pool->queue_rear=(pool->queue_rear+1)%pool->queue_max_size;
	pool->queue_size++;

	pthread_cond_signal(&(pool->queue_not_empty));
	pthread_mutex_unlock(&(pool->lock));

	return 0;
}

void *threadpool_thread(void *threadpool){
	threadpool_t *pool=(threadpool_t *)threadpool;
	threadpool_task_t task;

	while(true){
		pthread_mutex_lock(&(pool->lock));
		while((pool->queue_size==0)&&(!pool->shutdown)){
			printf("thread 0x%x is waiting\n",(unsigned int)pthread_self());
			pthread_cond_wait(&(pool->queue_not_empty),&(pool->lock));
		
			if(pool->wait_exit_thr_num>0){
				pool->wait_exit_thr_num--;
				
				if(pool->live_thr_num>pool->min_thr_num){
					printf("thread 0x%x is exiting\n",(unsigned int)pthread_self());
					pool->live_thr_num--;
					pthread_mutex_unlock(&(lock->lock));
					pthread_exit(NULL);
				}
			}
		}
	}
	if(pool->shutdown){
		pthread_mutex_unlock(&(pool->lock));
		printf("thread 0x%x is exiting\n",(unsigned int)pthread_self());
		pthread_exit(NULL);
	}

	task.function=pool->task_queue[pool->queue_front].function;
	task.arg=pool->task_queue[pool->queue_front].arg;

	pool->queue_front=(pool->queue_font+1)%pool->queue_max_size;
	pool->queue_size--;

	pthread_cond_broadcast(&(pool->queue_not_full));

	pthread_mutex_unlock();
}

