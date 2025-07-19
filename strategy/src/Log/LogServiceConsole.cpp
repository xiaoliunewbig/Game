/*
 * 文件名: LogServiceConsole.cpp
 * 说明: 策略层日志服务控制台实现。
 * 作者: 彭承康
 * 创建时间: 2025-07-13
 */
#include "Log/LogServiceConsole.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>

namespace strategy {

void LogServiceConsole::Log(LogLevel level,
                            const std::string& file,
                            int line,
                            const std::string& type,
                            const std::string& message) {
    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm_now, &now_c);
#else
    localtime_r(&now_c, &tm_now);
#endif
    // 日志级别转字符串
    std::string level_str = LogLevelToString(level);
    // 输出格式: [日期 时间][级别][文件:行][类型] 内容
    std::cout << "["
              << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S") << "]"
              << "[" << level_str << "]"
              << "[" << file << ":" << line << "]"
              << "[" << type << "] "
              << message << std::endl;
}

std::string LogServiceConsole::LogLevelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

} // namespace strategy 