#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "logger_int.h"

void start_log_thread();
void stop_log_thread();
void log_message(LogType type, const char *format, ...);

#define logi(format, ...) log_message(LOG_TYPE_INFO, format, ##__VA_ARGS__)
#define loge(format, ...) log_message(LOG_TYPE_ERROR, format, ##__VA_ARGS__)
#define logd(format, ...) log_message(LOG_TYPE_DEBUG, format, ##__VA_ARGS__)

#endif