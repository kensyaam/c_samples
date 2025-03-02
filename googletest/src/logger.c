#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <unistd.h>

#include "logger.h"
#include "util.h"

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t log_cond = PTHREAD_COND_INITIALIZER;
LogMessage *log_queue_head = NULL;
LogMessage *log_queue_tail = NULL;
int log_queue_size = 0;
int log_thread_running = 1;
pthread_t log_thread;

void rotate_logs(const char *log_file) {
    char old_log[256];
    char new_log[256];

    // Remove the oldest log file
    snprintf(old_log, sizeof(old_log), "%s.%d", log_file, MAX_LOG_FILES - 1);
    remove(old_log);

    // Rotate the log files
    for (int i = MAX_LOG_FILES - 2; i >= 0; i--) {
        snprintf(old_log, sizeof(old_log), "%s.%d", log_file, i);
        snprintf(new_log, sizeof(new_log), "%s.%d", log_file, i + 1);
        rename(old_log, new_log);
    }

    // Rename the current log file
    snprintf(new_log, sizeof(new_log), "%s.0", log_file);
    rename(log_file, new_log);
}

void *log_thread_func(void *arg) {
    (void)arg; // Unused

    while (log_thread_running) {
        pthread_mutex_lock(&log_mutex);

        while (log_queue_size == 0 && log_thread_running) {
            pthread_cond_wait(&log_cond, &log_mutex);
        }

        if (!log_thread_running) {
            pthread_mutex_unlock(&log_mutex);
            break;
        }

        LogMessage *msg = log_queue_head;
        log_queue_head = log_queue_head->next;
        if (log_queue_head == NULL) {
            log_queue_tail = NULL;
        }
        log_queue_size--;

        // printf("[log_thread_func] log_queue_size: %d\n", log_queue_size);
        // printf("[log_thread_func] head: %p, ->next: %p\n", 
        //         log_queue_head, 
        //         (log_queue_head != NULL ? log_queue_head->next : NULL));
        // printf("[log_thread_func] tail: %p, ->next: %p\n", 
        //         log_queue_tail, 
        //         (log_queue_tail != NULL ? log_queue_tail->next : NULL));

        pthread_mutex_unlock(&log_mutex);

        const char *log_file;
        switch (msg->type) {
            case LOG_TYPE_INFO:
                log_file = LOG_FILE_INFO;
                break;
            case LOG_TYPE_DEBUG:
                log_file = LOG_FILE_DEBUG;
                break;
            case LOG_TYPE_ERROR:
                log_file = LOG_FILE_ERROR;
                break;
            default:
                free_wrapped(msg);
                continue;
        }

        FILE *file = fopen(log_file, "a");
        if (file == NULL) {
            free_wrapped(msg);
            continue;
        }

        // Check the size of the log file
        struct stat st;
        if (stat(log_file, &st) == 0 && st.st_size >= MAX_LOG_SIZE) {
            fclose(file);
            rotate_logs(log_file);
            file = fopen(log_file, "a");
            if (file == NULL) {
                free_wrapped(msg);
                continue;
            }
        }

        // Write the log message
        fprintf(file, "%s", msg->message);
        fclose(file);

        free_wrapped(msg);
    }

    return NULL;
}

void log_message(LogType type, const char *format, ...) {
    va_list args;
    va_start(args, format);

    LogMessage *msg = (LogMessage *)malloc_wrapped(sizeof(LogMessage));
    if (msg == NULL) {
        va_end(args);
        return;
    }

    msg->type = type;
    msg->next = NULL;
    vsnprintf(msg->message, MAX_LOG_MESSAGE_SIZE, format, args);
    va_end(args);

    msg->next = NULL;

    pthread_mutex_lock(&log_mutex);

    if (log_queue_head == NULL) {
        log_queue_head = msg;
        log_queue_tail = msg;
    } else {
        log_queue_tail->next = msg;
        log_queue_tail = msg;
    }
    log_queue_size++;
    // printf("[log_message] log_queue_size: %d\n", log_queue_size);
    // printf("[log_message] head: %p, ->next: %p\n", 
    //         log_queue_head, 
    //         (log_queue_head != NULL ? log_queue_head->next : NULL));
    // printf("[log_message] tail: %p, ->next: %p\n", 
    //         log_queue_tail, 
    //         (log_queue_tail != NULL ? log_queue_tail->next : NULL));


    pthread_cond_signal(&log_cond);
    pthread_mutex_unlock(&log_mutex);
}

void start_log_thread() {
    pthread_create(&log_thread, NULL, log_thread_func, NULL);
    printf("Log thread started.\n");
}

void stop_log_thread() {
    // log_thread と同一スレッドか確認
    if (pthread_equal(log_thread, pthread_self())) {
        return;
    }

    pthread_mutex_lock(&log_mutex);
    log_thread_running = 0;
    pthread_cond_signal(&log_cond);
    pthread_mutex_unlock(&log_mutex);

    pthread_join(log_thread, NULL);

    printf("Log thread stopped.\n");
}