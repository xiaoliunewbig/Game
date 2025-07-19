/*
 * 文件名: LogServiceFile.cpp
 * 说明: 策略层日志服务文件实现。
 * 作者: 彭承康
 * 创建时间: 2025-07-13
 */
#include "Log/LogServiceFile.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>

namespace strategy {

LogServiceFile::LogServiceFile(const std::string& filename) {
    log_file_ = std::make_unique<std::ofstream>(filename, std::ios_base::app);
    if (!log_file_->is_open()) {
        throw std::runtime_error("Failed to open log file: " + filename);
    }
}

LogServiceFile::~LogServiceFile() {
    if (log_file_ && log_file_->is_open()) {
        log_file_->close();
    }
}

void LogServiceFile::Log(LogLevel level,
                         const std::string& file,
                         int line,
                         const std::string& type,
                         const std::string& message) {
    if (!log_file_ || !log_file_->is_open()) {
        return;
    }

    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm_now, &now_c);
#else
    localtime_r(&now_c, &tm_now);
#endif

    std::string level_str = LogLevelToString(level);

    *log_file_ << "["
               << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S") << "]"
               << "[" << level_str << "]"
               << "[" << file << ":" << line << "]"
               << "[" << type << "] "
               << message << std::endl;
}

std::string LogServiceFile::LogLevelToString(LogLevel level) const {
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