#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define MAX_THREADS 100 // 定義最大可用執行緒數量

// 定義 thread_info_t 結構
typedef struct {
    int thread_id;             // 唯一的執行緒標識符
    int policy;                // 排程策略
    int priority;              // 優先級
    float time_wait;           // 忙碌等待時間
    pthread_barrier_t *barrier; // 用於同步的屏障
} thread_info_t;

// Busy-waiting function
void busy_wait(float seconds) {
    clock_t start_time = clock();
    clock_t end_time = start_time + (clock_t)(seconds * CLOCKS_PER_SEC);
    while (clock() < end_time) {
        // Busy loop doing nothing
    }
}

// Worker thread function
void *worker_thread(void *arg) {
    thread_info_t *info = (thread_info_t *)arg;

    // 等待屏障解除（同步開始）
    pthread_barrier_wait(info->barrier);

    for (int i = 0; i < 3; i++) {
        printf("Thread %d is starting\n", info->thread_id);
        busy_wait(info->time_wait);
    }

    return NULL;
}

// 主程式
int main(int argc, char *argv[]) {
    // 1. Parse program arguments
    int num_threads = 0;
    float time_wait = 0.5;
    int policies[MAX_THREADS];
    int priorities[MAX_THREADS];

    // 簡單的參數解析（略，根據需求添加）

    // 2. Create <num_threads> worker threads
    pthread_t threads[MAX_THREADS];
    thread_info_t thread_infos[MAX_THREADS];
    pthread_barrier_t barrier;

    // 初始化屏障
    if (pthread_barrier_init(&barrier, NULL, num_threads) != 0) {
        fprintf(stderr, "Failed to initialize barrier: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    for (int i = 0; i < num_threads; i++) {
        thread_infos[i].thread_id = i;
        thread_infos[i].policy = policies[i];
        thread_infos[i].priority = priorities[i];
        thread_infos[i].time_wait = time_wait;
        thread_infos[i].barrier = &barrier;

        // 3. Set CPU affinity (略，根據需求實現)
        // 可使用 sched_setaffinity 函數來指定 CPU affinity

        // 4. Set the attributes to each thread
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        struct sched_param param;
        param.sched_priority = thread_infos[i].priority;
        pthread_attr_setschedpolicy(&attr, thread_infos[i].policy);
        pthread_attr_setschedparam(&attr, &param);

        // 5. Start all threads at once
        if (pthread_create(&threads[i], &attr, worker_thread, &thread_infos[i]) != 0) {
            fprintf(stderr, "Error creating thread %d: %s\n", i, strerror(errno));
            return EXIT_FAILURE;
        }

        pthread_attr_destroy(&attr); // 清理屬性
    }

    // 6. Wait for all threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // 清理屏障
    pthread_barrier_destroy(&barrier);

    return EXIT_SUCCESS;
}
