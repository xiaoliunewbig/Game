/*
 * 文件名: LogServiceConsole.h
 * 说明: 策略层日志服务控制台实现头文件。
 * 作者: 彭承康
 * 创建时间: 2025-07-13
 */
#ifndef STRATEGY_LOGSERVICECONSOLE_H
#define STRATEGY_LOGSERVICECONSOLE_H

#include "Log/ILogService.h"

namespace strategy {

/**
 * @brief 控制台日志服务实现
 * 单一职责：只负责日志输出到控制台。
 */
class LogServiceConsole : public ILogService {
public:
    virtual ~LogServiceConsole() = default;

    /**
     * @brief 输出日志到控制台
     */
    void Log(LogLevel level,
             const std::string& file,
             int line,
             const std::string& type,
             const std::string& message) override;

private:
    /**
     * @brief 日志级别转字符串
     */
    std::string LogLevelToString(LogLevel level) const;
};

} // namespace strategy

#endif // STRATEGY_LOGSERVICECONSOLE_H 