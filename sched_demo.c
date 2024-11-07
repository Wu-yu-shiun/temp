#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// Structure to store thread information
typedef struct {
    int thread_id;
    int policy;
    int priority;
    float time_wait;
    pthread_barrier_t *barrier;
} thread_info_t;

// Busy-wait function for the specified number of seconds
void busy_wait(float seconds) {
    clock_t start_time = clock();
    clock_t end_time = start_time + (clock_t)(seconds * CLOCKS_PER_SEC);
    while (clock() < end_time) {
        // Busy loop doing nothing
    }
}

// Thread function
void *thread_func(void *arg) {
    thread_info_t *info = (thread_info_t *)arg;

    // Wait for all threads to be ready
    pthread_barrier_wait(info->barrier);

    // Perform the task 3 times
    for (int i = 0; i < 3; i++) {
        printf("Thread %d is starting with policy %d and priority %d\n",
               info->thread_id, info->policy, info->priority);
        busy_wait(info->time_wait);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    int num_threads = 0;
    float time_wait = 0;
    char *policies_str = NULL;
    char *priorities_str = NULL;

    // Parse command line arguments
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
                policies_str = optarg;
                break;
            case 'p':
                priorities_str = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s -n <num_threads> -t <time_wait> -s <policies> -p <priorities>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (num_threads <= 0 || time_wait <= 0 || !policies_str || !priorities_str) {
        fprintf(stderr, "All arguments must be provided and valid.\n");
        exit(EXIT_FAILURE);
    }

    // Parse scheduling policies
    int policies[num_threads];
    int priorities[num_threads];
    char *token;

    token = strtok(policies_str, ",");
    for (int i = 0; i < num_threads && token != NULL; i++) {
        if (strcmp(token, "NORMAL") == 0) {
            policies[i] = SCHED_OTHER;
        } else if (strcmp(token, "FIFO") == 0) {
            policies[i] = SCHED_FIFO;
        } else {
            fprintf(stderr, "Invalid policy: %s\n", token);
            exit(EXIT_FAILURE);
        }
        token = strtok(NULL, ",");
    }

    token = strtok(priorities_str, ",");
    for (int i = 0; i < num_threads && token != NULL; i++) {
        priorities[i] = atoi(token);
        token = strtok(NULL, ",");
    }

    // Create a barrier for synchronizing thread start
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, num_threads);

    // Create threads
    pthread_t threads[num_threads];
    thread_info_t thread_info[num_threads];

    for (int i = 0; i < num_threads; i++) {
        thread_info[i].thread_id = i;
        thread_info[i].policy = policies[i];
        thread_info[i].priority = priorities[i];
        thread_info[i].time_wait = time_wait;
        thread_info[i].barrier = &barrier;

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy(&attr, thread_info[i].policy);

        struct sched_param param;
        param.sched_priority = thread_info[i].priority;
        pthread_attr_setschedparam(&attr, &param);

        if (pthread_create(&threads[i], &attr, thread_func, (void *)&thread_info[i]) != 0) {
            perror("Failed to create thread");
            exit(EXIT_FAILURE);
        }

        pthread_attr_destroy(&attr);
    }

    // Wait for all threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);

    return 0;
}
