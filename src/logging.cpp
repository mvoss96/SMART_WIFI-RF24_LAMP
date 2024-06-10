#include <Arduino.h>
#include "logging.h"

const static int LOG_BUFFER_SIZE = 256; // Buffer size for the log message

// Function to get log level string
static const char *getLogLevelString(LOG_LEVEL level)
{
    switch (level)
    {
    case LOG_LEVEL::DEBUG:
        return "DEBUG";
    case LOG_LEVEL::INFO:
        return "INFO";
    case LOG_LEVEL::WARNING:
        return "WARNING";
    case LOG_LEVEL::ERROR:
        return "ERROR";
    default:
        return "";
    }
}

// Custom log functions
void log(LOG_LEVEL level, const char *format, ...)
{
    char buffer[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, LOG_BUFFER_SIZE, format, args);
    va_end(args);
    printf("[%s] %s", getLogLevelString(level), buffer);
}
