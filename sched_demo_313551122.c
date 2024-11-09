#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>

typedef struct {
    int thread_id;
    int policy;
    int priority;
    float time_wait;
    pthread_barrier_t *barrier;
} thread_info_t;

void busy_wait(float seconds) {
    clock_t start_time = clock();
    clock_t end_time = start_time + (clock_t)(seconds * CLOCKS_PER_SEC);
    while (clock() < end_time) {
        // Busy waiting
    }
}

// worker thread
void *thread_func(void *arg){
    thread_info_t *info = (thread_info_t *)arg;
    /* 1. Wait until all threads are ready */
    pthread_barrier_wait(info->barrier);
    /* 2. Do the task */ 
    for (int i = 0; i < 3; i++) {
        printf("Thread %d is starting\n", info->thread_id);
        busy_wait(info->time_wait);
    }
    /* 3. Exit the function  */
    return NULL;
}

// main thread
int main(int argc, char *argv[]) {
    /* 1. Parse program arguments */
    int num_threads = 0;
    float time_wait = 0;
    char **policies = NULL;
    int *priorities = NULL;
    
    int opt;
    while ((opt = getopt(argc, argv, "n:t:s:p:")) != -1) {
        switch (opt) {
            case 'n':
                num_threads = atoi(optarg);
                break;
            case 't':
                time_wait = atof(optarg);
                break;
            case 's':
                policies = malloc(num_threads * sizeof(char *));
                char *policy_token = strtok(optarg, ",");
                for (int i = 0; i < num_threads; i++) {
                    policies[i] = strdup(policy_token);
                    policy_token = strtok(NULL, ",");
                }
                break;
            case 'p':
                priorities = malloc(num_threads * sizeof(int));
                char *priority_token = strtok(optarg, ",");
                for (int i = 0; i < num_threads; i++) {
                    priorities[i] = atoi(priority_token);
                    priority_token = strtok(NULL, ",");
                }
                break;
        }
    }

    // Debug prints for verification
    fprintf(stderr, "num_threads: %d\n", num_threads);
    fprintf(stderr, "time_wait: %.2f\n", time_wait);
    for (int i = 0; i < num_threads; i++) {
        fprintf(stderr, "policies[%d]: %s\n", i, policies[i]);
        fprintf(stderr, "priorities[%d]: %d\n", i, priorities[i]);
    }
    // Debug prints for verification
    

    /* 2. Create <num_threads> worker threads */
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, num_threads);
    pthread_t threads[num_threads];
    thread_info_t thread_info[num_threads];
    
    /* 3. Set CPU affinity */
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(3, &cpuset);
    sched_setaffinity(getpid(), sizeof(cpuset), &cpuset);
    
    for (int i = 0; i < num_threads; i++) {
        thread_info[i].thread_id = i;
        thread_info[i].policy = policies[i];
        thread_info[i].priority = priorities[i];
        thread_info[i].time_wait = time_wait;
        thread_info[i].barrier = &barrier;
        
        /* 4. Set the attributes to each thread */
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy(&attr, thread_info[i].policy);

        struct sched_param param;
        param.sched_priority = thread_info[i].priority;
        pthread_attr_setschedparam(&attr, &param);

        if (pthread_create(&threads[i], &attr, thread_func, &thread_info[i]) != 0) {
            fprintf(stderr, "Error creating thread %d: %s\n", i, strerror(errno));
            return EXIT_FAILURE;
        }

        pthread_attr_destroy(&attr);
    }
    
    /* 5. Start all threads at once */
    pthread_barrier_wait(&barrier);

    /* 6. Wait for all threads to finish  */ 
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    pthread_barrier_destroy(&barrier);
    return 0;
}
