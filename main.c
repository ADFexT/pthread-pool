#include <stdio.h>
#include <unistd.h>   // for usleep
#include "Threadpool.h"

// 任务函数示例
void print_hello(void* arg) {
    int id = *(int*)arg;
    printf("Task %d is processed by thread %lu\n", id, pthread_self());
    usleep(100000); // 模拟任务耗时 0.1 秒
}

int main() {
    // 创建包含4个线程的线程池
    Threadpool* pool = thread_pool_create(4);
    if (!pool) {
        fprintf(stderr, "Failed to create thread pool\n");
        return 1;
    }

    // 提交8个任务，每个任务有自己的参数
    int ids[8];
    for (int i = 0; i < 8; ++i) {
        ids[i] = i;
        thread_pool_enqueue(pool, print_hello, &ids[i]);
    }

    // 给点时间让任务执行完（实际中可用同步手段）
    usleep(1500000); // 1.5秒

    // 销毁线程池
    thread_pool_destroy(pool);

    printf("All tasks completed. Pool destroyed.\n");
    return 0;
}


