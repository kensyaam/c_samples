#ifndef __INCLUDE_LOGGER_H__
#define __INCLUDE_LOGGER_H__

typedef enum {
    LOG_TYPE_INFO,
    LOG_TYPE_DEBUG,
    LOG_TYPE_ERROR
} LogType;

void start_log_thread();
void stop_log_thread();
void log_message(LogType type, const char *format, ...);

#define logi(format, ...) log_message(LOG_TYPE_INFO, format, ##__VA_ARGS__)
#define loge(format, ...) log_message(LOG_TYPE_ERROR, format, ##__VA_ARGS__)

#endif