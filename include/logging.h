#pragma once

// Enum for log levels
enum class LOG_LEVEL
{
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

#define LOG_DEBUG(format, ...) log(LOG_LEVEL::DEBUG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) log(LOG_LEVEL::INFO, format, ##__VA_ARGS__)
#define LOG_WARNING(format, ...) log(LOG_LEVEL::WARNING, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) log(LOG_LEVEL::ERROR, format, ##__VA_ARGS__)

// Custom log function
void log(LOG_LEVEL level, const char *format, ...);