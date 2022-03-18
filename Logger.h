#pragma once

#include<assert.h>
#include<string>
#include"noncopyable.h"


// LOG_INFO("%s %d", arg1, arg2)
#define LOG_INFO(LogmsgFormat, ...) \
        do { \
            char buf[1024] = {0}; \
            Logger &logger = Logger::instance(); \
            logger.setLogLevel(INFO); \
            snprintf(buf, 1024, LogmsgFormat, ##__VA_ARGS__); \
            logger.log(buf); \
        } while(0)

#define LOG_ERROR(LogmsgFormat, ...) \
        do { \
            char buf[1024] = {0}; \
            Logger &logger = Logger::instance(); \
            logger.setLogLevel(ERROR); \
            snprintf(buf, 1024, LogmsgFormat, ##__VA_ARGS__); \
            logger.log(buf); \
        } while(0)

#define LOG_FATAL(LogmsgFormat, ...) \
        do { \
            char buf[1024] = {0}; \
            Logger &logger = Logger::instance(); \
            logger.setLogLevel(FATAL); \
            snprintf(buf, 1024, LogmsgFormat, ##__VA_ARGS__); \
            logger.log(buf); \
            exit(-1); \
        } while(0)  

#ifdef MUDEGUB
#define LOG_DEBUG(LogmsgFormat, ...) \
        do { \
            char buf[1024] = {0}; \
            Logger &logger = Logger::instance(); \
            logger.setLogLevel(DEBUG); \
            snprintf(buf, 1024, LogmsgFormat, ##__VA_ARGS__); \
            logger.log(buf); \
        } while(0)
#else
    #define LOG_DEBUG(LogmsgFormat, ...)
#endif

// 定义日志级别 INFO ERROR FATAL DEBUG
enum LogLevel {
    INFO,   // 普通信息
    ERROR,  // 错误信息
    FATAL,  // core信息
    DEBUG,  // 调试信息
};

// 输出一个日志类
class Logger : noncopyable {
 public:
  // 获取日志类唯一的实例对象（懒汉单例模式）
  static Logger& instance();
  // 设置日志级别 
  void setLogLevel(int level);
  // 写日志
  void log(std::string msg);

 private:
  int logLevel_;
  Logger() {}
};