/*
 * 文件名: LogMacros.h
 * 说明: 日志系统便捷宏，自动填充文件名和行号。
 * 作者: 彭承康
 * 创建时间: 2025-07-13
 */
#ifndef STRATEGY_LOGMACROS_H
#define STRATEGY_LOGMACROS_H

#include "Log/ILogService.h"

// 日志宏，自动填充文件名和行号
#define STRATEGY_LOG(logService, level, type, message) \
    (logService).Log((level), __FILE__, __LINE__, (type), (message))

#endif // STRATEGY_LOGMACROS_H 