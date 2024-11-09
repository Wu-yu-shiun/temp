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
    pthread_t thread_id;
    int thread_num;
    float time_wait;
    int policy;
    int priority;
} thread_info_t;

pthread_barrier_t barrier;



// worker thread
void *thread_func(void *arg){
    thread_info_t *info = (thread_info_t *)arg;

    /* 1. Wait until all threads are ready */
    pthread_barrier_wait(&barrier);

    /* 2. Do the task */ 
    for (int i = 0; i < 3; i++) {
        printf("Thread %d is starting\n", info->thread_num);
        clock_t start_time = clock();
        clock_t end_time = start_time + (clock_t)(info->time_wait * CLOCKS_PER_SEC);
        while (clock() < end_time){
            // Busy waiting
        }
    }

    /* 3. Exit the function  */
    return NULL;
}

// main thread
int main(int argc, char *argv[]) {
    /* 1. Parse program arguments */
    int num_threads = 0;
    float time_wait = 0;
    char *policies_str = NULL;
    char *priorities_str = NULL;

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
        }
    }

    int policies[num_threads];
    int priorities[num_threads];
    char *policy_token = strtok(policies_str, ",");
    for (int i = 0; i < num_threads; i++) {
        if (strcmp(policy_token, "NORMAL") == 0) {
            policies[i] = SCHED_OTHER;
        } else if (strcmp(policy_token, "FIFO") == 0) {
            policies[i] = SCHED_FIFO;
        }
        policy_token = strtok(NULL, ",");
    }
    char *priority_token = strtok(priorities_str, ",");
    for (int i = 0; i < num_threads; i++) {
        priorities[i] = atoi(priority_token);
        priority_token = strtok(NULL, ",");
    }

    // Debug
    // fprintf(stderr, "num_threads: %d\n", num_threads);
    // fprintf(stderr, "time_wait: %.2f\n", time_wait);
    // for (int i = 0; i < num_threads; i++) {
    //     fprintf(stderr, "policies[%d]: %s (%d)\n", i, (policies[i] == SCHED_OTHER) ? "NORMAL" : "FIFO", policies[i]);
    //     fprintf(stderr, "priorities[%d]: %d\n", i, priorities[i]);
    // }
    

    /* 2. Create <num_threads> worker threads */
    thread_info_t thread_info[num_threads];
		pthread_attr_t pthread_attr[num_threads];
		struct sched_param param[num_threads];
		pthread_barrier_init(&barrier, NULL, num_threads+1);
    
    /* 3. Set CPU affinity */
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(1, &cpuset);
    sched_setaffinity(getpid(), sizeof(cpuset), &cpuset);

    for (int i = 0; i < num_threads; i++) { 
        /* 4. Set the attributes to each thread */
        pthread_attr_init(&pthread_attr[i]);
				thread_info[i].thread_num = i;
				thread_info[i].policy = policies[i];
				thread_info[i].priority = priorities[i];
				thread_info[i].time_wait = time_wait;
				param[i].sched_priority = thread_info[i].priority;
	
				pthread_attr_setinheritsched(&pthread_attr[i], PTHREAD_EXPLICIT_SCHED);
				pthread_attr_setschedpolicy(&pthread_attr[i], thread_info[i].policy);
				pthread_attr_setschedparam(&pthread_attr[i], &param[i]);
				pthread_create(&thread_info[i].thread_id, &pthread_attr[i], thread_func, &thread_info[i]);
				pthread_setaffinity_np(thread_info[i].thread_id, sizeof(cpu_set_t), &cpuset);    
    }
    
    /* 5. Start all threads at once */
    pthread_barrier_wait(&barrier);
    
    /* 6. Wait for all threads to finish  */ 
    for (int i = 0; i < num_threads; i++) {
        pthread_join(thread_info[i].thread_id, NULL);
    }
    pthread_barrier_destroy(&barrier);

    return 0;
}
