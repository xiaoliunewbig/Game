/*
 * 文件名: LogServiceFile.h
 * 说明: 策略层日志服务文件实现头文件。
 * 作者: 彭承康
 * 创建时间: 2025-07-13
 */
#ifndef STRATEGY_LOGSERVICEFILE_H
#define STRATEGY_LOGSERVICEFILE_H

#include "Log/ILogService.h"
#include <string>
#include <fstream>
#include <memory>

namespace strategy {

/**
 * @brief 文件日志服务实现
 * 单一职责：只负责日志输出到文件。
 */
class LogServiceFile : public ILogService {
public:
    explicit LogServiceFile(const std::string& filename);
    virtual ~LogServiceFile();

    /**
     * @brief 输出日志到文件
     */
    void Log(LogLevel level,
             const std::string& file,
             int line,
             const std::string& type,
             const std::string& message) override;

private:
    std::string LogLevelToString(LogLevel level) const;
    std::unique_ptr<std::ofstream> log_file_;
};

} // namespace strategy

#endif // STRATEGY_LOGSERVICEFILE_H 