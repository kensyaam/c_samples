#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdarg.h>
#include <sys/stat.h>

#include "logger.h"

#define LOG_FILE_INFO "logfile_info.log"
#define LOG_FILE_DEBUG "logfile_debug.log"
#define LOG_FILE_ERROR "logfile_error.log"
#define MAX_LOG_SIZE 1024 * 1024 // 1MB
#define MAX_LOG_FILES 3

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

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

void log_message(LogType type, const char *format, ...) {
    pthread_mutex_lock(&log_mutex);

    const char *log_file;
    switch (type) {
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
            return;
    }

    FILE *file = fopen(log_file, "a");
    if (file == NULL) {
        pthread_mutex_unlock(&log_mutex);
        return;
    }

    // Check the size of the log file
    struct stat st;
    if (stat(log_file, &st) == 0 && st.st_size >= MAX_LOG_SIZE) {
        fclose(file);
        rotate_logs(log_file);
        file = fopen(log_file, "a");
        if (file == NULL) {
            pthread_mutex_unlock(&log_mutex);
            return;
        }
    }

    // Write the log message
    va_list args;
    va_start(args, format);
    vfprintf(file, format, args);
    va_end(args);

    fclose(file);
    pthread_mutex_unlock(&log_mutex);
}

void start_log_thread() {}
void stop_log_thread() {}
