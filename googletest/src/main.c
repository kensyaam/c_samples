#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include "logger.h"

#define NUM_THREADS 10
#define NUM_LOGS_PER_THREAD 100

void *thread_func(void *arg) {
    int thread_id = *((int *)arg);
    for (int i = 0; i < NUM_LOGS_PER_THREAD; i++) {
        logi("Thread %d: Info log message %d\n", thread_id, i);
        loge("Thread %d: Error log message %d\n", thread_id, i);
        // printf("Thread %d: Log message %d\n", thread_id, i);
        // usleep((rand() % 100 + 1) * 1000);
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    // Start the log thread
    start_log_thread();

    struct timeval start, end;

    // 開始時刻の取得
    gettimeofday(&start, NULL);

    // Create worker threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // 終了時刻の取得
    gettimeofday(&end, NULL);

    // 経過時間の計算 (ミリ秒単位)
    long elapsed_ms = (end.tv_sec - start.tv_sec) * 1000 
                    + (end.tv_usec - start.tv_usec) / 1000;

    printf("経過時間: %ld ms\n", elapsed_ms);

    sleep(1);

    // Stop the log thread
    stop_log_thread();

    return 0;
}