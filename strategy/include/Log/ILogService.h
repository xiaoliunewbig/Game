/*
 * 文件名: ILogService.h
 * 说明: 策略层日志服务接口，定义日志输出的标准方式。
 * 作者: 彭承康
 * 创建时间: 2025-07-13
 *
 * 本接口用于策略层日志系统，满足单一职责和接口隔离原则。
 * 支持输出日期、文件名、行号、错误类型、日志内容等。
 */
#ifndef STRATEGY_ILOGSERVICE_H
#define STRATEGY_ILOGSERVICE_H

#include <string>

namespace strategy {

/**
 * @brief 日志级别枚举
 */
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

/**
 * @brief 日志服务接口（抽象类）
 * 单一职责：只负责日志输出。
 * 接口隔离：仅暴露日志相关方法。
 */
class ILogService {
public:
    virtual ~ILogService() = default;

    /**
     * @brief 输出日志
     * @param level 日志级别
     * @param file 文件名
     * @param line 行号
     * @param type 错误类型或分类
     * @param message 日志内容
     */
    virtual void Log(LogLevel level,
                     const std::string& file,
                     int line,
                     const std::string& type,
                     const std::string& message) = 0;
};

} // namespace strategy

#endif // STRATEGY_ILOGSERVICE_H 