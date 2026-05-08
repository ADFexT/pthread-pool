#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>

//任务链表结构体
typedef struct TaskNode{
    void (*func)(void *arg);
    void *arg;
    struct TaskNode *next;
}TaskNode;

//线程池结构体
typedef struct Threadpool{
    pthread_t *threads;
    int thread_count;
    TaskNode *task_queue_head;
    TaskNode *task_queue_tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int stop;
}Threadpool;

//API
Threadpool *thread_pool_create(int thread_count);
void thread_pool_enqueue(Threadpool *pool,void (*func)(void *arg),void *arg);
void thread_pool_destroy(Threadpool *pool);
#endif