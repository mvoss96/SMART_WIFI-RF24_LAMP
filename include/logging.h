#pragma once
#define LOG_BUFFER_SIZE 256 // Buffer size for the log message

#include <cstdint>

// Enum for log levels
enum class LOG_LEVEL : uint8_t
{
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

#define LOG_SET_LEVEL LOG_LEVEL::INFO // Set the log level

#define LOG_DEBUG(format, ...) log(LOG_LEVEL::DEBUG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) log(LOG_LEVEL::INFO, format, ##__VA_ARGS__)
#define LOG_WARNING(format, ...) log(LOG_LEVEL::WARNING, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) log(LOG_LEVEL::ERROR, format, ##__VA_ARGS__)

// Custom log function
void log(LOG_LEVEL level, const char *format, ...);