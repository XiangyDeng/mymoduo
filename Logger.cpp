#include"Logger.h"

#include<assert.h>
#include"Timestamp.h"

// 获取日志类唯一的实例对象（懒汉单例模式）
Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

// 设置日志级别 
void Logger::setLogLevel(int level) {
    logLevel_ = level;
}

// 写日志 [级别信息] time ：msg
void Logger::log(std::string msg) {
    // 打印 级别信息
    switch (logLevel_) {
      case INFO: {
        std::cout << "[INFO]";
      break;
      }
      case ERROR: {
        std::cout << "[ERROR]";
      break;
      }
      case FATAL: {
        std::cout << "[FATAL]";
      break;
      }
      case DEBUG: {
        std::cout << "[DEBUG]";
      break;
      }
      default: {
          assert(false);
      }
    }

    // 打印时间与msg
    std::cout << Timestamp::now().toString() << " : " << msg << std::endl;
}

// int main() {
//   LOG_INFO("%s", "hello world!");
//   LOG_ERROR("%s", "hello world!");
//   LOG_FATAL("%s", "hello world!");
// }