#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>
#include <errno.h>

// Function for busy-wait to simulate work
void busy_wait(double seconds) {
    double start = (double)clock() / CLOCKS_PER_SEC;
    while (((double)clock() / CLOCKS_PER_SEC) - start < seconds);
}

// Thread function
void* thread_func(void* arg) {
    int id = *(int*)arg;
    pthread_barrier_wait(&barrier);

    for (int i = 0; i < 3; i++) {
        printf("Thread %d is starting\n", id);
        busy_wait(time_wait);  // Busy waiting for the specified time
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    int num_threads = 0;
    double time_wait = 0.0;
    char *policies;
    int *priorities;
    
    // Parsing command-line arguments
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
                char *policy_str = strtok(optarg, ",");
                for (int i = 0; policy_str != NULL && i < num_threads; i++) {
                    if (strcmp(policy_str, "NORMAL") == 0)
                        policies[i] = SCHED_OTHER;
                    else if (strcmp(policy_str, "FIFO") == 0)
                        policies[i] = SCHED_FIFO;
                    policy_str = strtok(NULL, ",");
                }
                break;
            case 'p':
                char *priority_str = strtok(optarg, ",");
                for (int i = 0; priority_str != NULL && i < num_threads; i++) {
                    priorities[i] = atoi(priority_str);
                    priority_str = strtok(NULL, ",");
                }
                break;
        }
    }
    
    // Create pthread attributes and set scheduling policies
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    int *thread_ids = malloc(num_threads * sizeof(int));
    pthread_attr_t attr;
    
    pthread_barrier_init(&barrier, NULL, num_threads);
    
    for (int i = 0; i < num_threads; i++) {
        pthread_attr_init(&attr);
        
        struct sched_param param;
        if (strcmp(policies[i], "FIFO") == 0) {
            pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
            param.sched_priority = priorities[i];
            pthread_attr_setschedparam(&attr, &param);
        } else {
            pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
        }
        
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        
        thread_ids[i] = i;
        pthread_create(&threads[i], &attr, thread_func, &thread_ids[i]);
        pthread_attr_destroy(&attr);
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    pthread_barrier_destroy(&barrier);
    free(threads);
    free(thread_ids);
    
    return 0;
}
