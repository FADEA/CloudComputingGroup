#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

typedef struct threadpool_t threadpool_t;

threadpool_t *threadpool_create(int min_thr_num,int max_thr_num,int queue_max_size);
/*
 * 创建线程池的函数
 * 参数一：线程池中至少有多少线程数
 * 参数二：线程池中最多有多少的线程数
 * 参数三：任务队列的最大值
 */

int threadpool_add(threadpool_t *pool,void *(*function)(void *arg),void *arg);
/*
 * 向线程池中添加一个任务
 * 参数二：回调函数，线程接到任务后回做什么
 * 参数三：回调函数的参数
 */

int threadpool_destroy(threadpool_t *pool);
/*
 * 摧毁线程池
 */

int threadpool_all_threadnum(threadpool_t *pool);
/*
 * 获得线程池的的线程数
 */

int threadpool_busy_threadnum(threadpool_t *pool);
/*
 * 获得线程池中忙碌的线程数
 */

#endif

