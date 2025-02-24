#ifndef __LOGGER_INT_H__
#define __LOGGER_INT_H__

#define LOG_FILE_INFO "logfile_info.log"
#define LOG_FILE_DEBUG "logfile_debug.log"
#define LOG_FILE_ERROR "logfile_error.log"
#define MAX_LOG_SIZE 1024 * 1024 // 1MB
#define MAX_LOG_FILES 3
#define MAX_LOG_MESSAGE_SIZE 1024

typedef enum {
    LOG_TYPE_INFO,
    LOG_TYPE_DEBUG,
    LOG_TYPE_ERROR
} LogType;

typedef struct LogMessage {
    LogType type;
    char message[MAX_LOG_MESSAGE_SIZE];
    struct LogMessage *next;
} LogMessage;

#ifdef UNIT_TEST
extern pthread_mutex_t log_mutex;
extern pthread_cond_t log_cond;
extern LogMessage *log_queue_head;
extern LogMessage *log_queue_tail;
extern int log_queue_size;
extern int log_thread_running;
extern pthread_t log_thread;

void rotate_logs(const char *log_file);
void *log_thread_func(void *arg);
void log_message(LogType type, const char *format, ...);
#endif

#endif