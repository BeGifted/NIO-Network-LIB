#pragma once

#include <string>
#include "noncopyable.h"

#define LOG_INFO(logmsgFormat, ...) \
    { \
        Logger& logger = Logger::instance(); \
        logger.setLogLevel(INFO); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    }

#define LOG_ERROR(logmsgFormat, ...) \
    { \
        Logger& logger = Logger::instance(); \
        logger.setLogLevel(ERROR); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    }

#define LOG_FATAL(logmsgFormat, ...) \
    { \
        Logger& logger = Logger::instance(); \
        logger.setLogLevel(FATAL); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    }

#ifdef NETDEBUG
#define LOG_DEBUG(logmsgFormat, ...) \
    { \
        Logger& logger = Logger::instance(); \
        logger.setLogLevel(FATAL); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    }
#else 
#define LOG_DEBUG(logmsgFormat, ...) 
#endif

enum LogLevel {
    INFO,
    ERROR,
    FATAL,
    DEBUG
};

class Logger: noncopyable {
public:
    static Logger& instance();
    void setLogLevel(int level);
    void log(std::string msg);
private:
    int logLevel_;
    Logger(){}
};