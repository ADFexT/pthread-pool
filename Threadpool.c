#include <stdlib.h>
#include "Threadpool.h"

//线程入口函数
static void* worker_routine(void *arg){
    Threadpool *pool = (Threadpool *)arg;
    while(1){
        //上锁 
        pthread_mutex_lock(&pool->mutex);
        //检查任务队列是否为空且池打开
        while(pool->stop == 0 && pool->task_queue_head == NULL){
            pthread_cond_wait(&pool->cond,&pool->mutex);
        }
        if (pool->stop == 1 && pool->task_queue_head == NULL){
            pthread_mutex_unlock(&pool->mutex);
            break;
        }
        //从队列头取一个任务并执行
        TaskNode *task = pool->task_queue_head;
        pool->task_queue_head = task->next;
        if(pool->task_queue_head == NULL){
            pool->task_queue_tail == NULL;
        }
        pthread_mutex_unlock(&pool->mutex);

        task->func(task->arg);
        free(task);
    }
}

//创建线程池
Threadpool *thread_pool_create(int thread_count){
    if(thread_count <= 0) return NULL;
    Threadpool *pool = (Threadpool *)malloc(sizeof(Threadpool));
    if(!pool) return NULL; 
    
    //池初始化
    pool->thread_count = thread_count;
    pool->task_queue_head = NULL;
    pool->task_queue_tail = NULL;
    pool->stop = 0;

    //初始化条件变量与互斥锁
    if (pthread_mutex_init(&pool->mutex,NULL)!=0){
        free(pool);
        return NULL;
    }
    
    if (pthread_cond_init(&pool->cond,NULL)!=0){
        free(pool);
        return NULL;
    }
    
    //创建线程数组
    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t)*thread_count);
    if(!pool->threads){
        pthread_cond_destroy(&pool->cond);
        pthread_mutex_destroy(&pool->mutex);
        free(pool);
        return NULL;
    }
    
    //创建线程
    for (size_t i = 0; i < thread_count; i++)
    {
        if(pthread_create(pool->threads[i],NULL,worker_routine,pool)!=0){
            //处理已经创建的线程
            for (size_t j = 0; j < i; j++)
            {
                pthread_join(pool->threads[j],NULL);
            }
            pthread_cond_destroy(&pool->cond);
            pthread_mutex_destroy(&pool->mutex);
            free(pool->threads);
            free(pool);
            return NULL;
        }
    }

    return pool;
}

//任务提交到队列
void thread_pool_enqueue(Threadpool *pool,void (*func)(void *),void *arg){
    if (!pool || !func) return;
    
    TaskNode *task = (TaskNode *)malloc(sizeof(TaskNode));
    if(!task) return;
    task->func = func;
    task->arg = arg;
    task->next = NULL;

    pthread_mutex_lock(&pool->mutex);
    if(pool->stop){
        pthread_mutex_unlock(&pool->mutex);
        free(task);
        return;
    }

    if(pool->task_queue_tail == NULL){
        pool->task_queue_head = task;
        pool->task_queue_tail = task;
    }else{
        pool->task_queue_tail->next = task;
        pool->task_queue_tail = task;
    }

    pthread_cond_signal(&pool->cond);
    pthread_mutex_destroy(&pool->mutex);
}

//销毁线程池
void thread_pool_destroy(Threadpool *pool){
    if(!pool) return;
    pthread_mutex_lock(&pool->mutex);
    pool->stop = 1;
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->mutex);

    for (size_t i = 0; i < pool->thread_count; i++)
    {
        pthread_join(pool->threads[i],NULL);
    }
    
    TaskNode *current = pool->task_queue_head;
    while (current)
    {
        TaskNode *next = current->next;
        free(current);
        current = next;
    }
    
    pthread_cond_destroy(&pool->cond);
    pthread_mutex_destroy(&pool->mutex);

    free(pool->threads);
    free(pool);
}