#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

/*封装信号量的类*/
class sem{
    public:
        /*创建并初始化信号量*/
        sem(int n){
            if(sem_init(&m_sem,0,n)<0){
                perror("sem_init error\n");
            }
        }

        /*销毁信号*/
        ~sem(){
            if(sem_destroy(&m_sem)<0){
                perror("sem_destroy error\n");
            }
        }

        /*等待信号量*/
        bool wait(){
            if(sem_wait(&m_sem)<0){
                perror("sem_wait error\n");
                return false;
            }else{
                return true;
            }
        }

        /*增加信号量*/
        bool post(){
            if(sem_post(&m_sem)<0){
                perror("sem_post error\n");
                return false;
            }else{
                return true;
            }
        }
    private:
        sem_t m_sem;
};

/*封装互斥锁的类*/
class locker{
    public:
        /*创建并初始化互斥锁*/
        locker(){
            if(pthread_mutex_init(&m_mutex,NULL)!=0){
                perror("mutex_init error\n");
            }
        }

        /*销毁互斥锁*/
        ~locker(){
            if(pthread_mutex_destroy(&m_mutex)!=0){
                perror("mutex_destroy error\n");
            }
        }

        /*获取互斥锁*/
        bool lock(){
            if(pthread_mutex_lock(&m_mutex)!=0){
                perror("get lock failed\n");
                return false;
            }else{
                return true;
            }
        }

        /*释放互斥锁*/
        bool unlock(){
            if(pthread_mutex_unlock(&m_mutex)!=0){
                perror("release lock failed\n");
                return false;
            }else{
                return true;
            }
        }

    private:
        pthread_mutex_t m_mutex;
};