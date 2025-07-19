/*
 * 文件名: StrategyException.h
 * 说明: 策略层异常类定义
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 定义策略层使用的各种异常类型，提供统一的错误处理机制。
 * 所有异常都继承自标准库的runtime_error，便于统一处理。
 */

#ifndef STRATEGY_EXCEPTION_H
#define STRATEGY_EXCEPTION_H

#include <stdexcept>
#include <string>

namespace strategy {

/**
 * @brief 策略层基础异常类
 * 
 * 所有策略层异常的基类，提供统一的异常处理接口
 */
class StrategyException : public std::runtime_error {
public:
    /**
     * @brief 构造函数
     * @param message 异常消息
     */
    explicit StrategyException(const std::string& message) 
        : std::runtime_error(message), error_code_(0) {}

    /**
     * @brief 构造函数（带错误码）
     * @param message 异常消息
     * @param error_code 错误码
     */
    StrategyException(const std::string& message, int error_code)
        : std::runtime_error(message), error_code_(error_code) {}

    /**
     * @brief 获取错误码
     * @return 错误码
     */
    int GetErrorCode() const noexcept { return error_code_; }

private:
    int error_code_;  ///< 错误码
};

/**
 * @brief 数据库异常类
 * 
 * 数据库操作相关的异常
 */
class DatabaseException : public StrategyException {
public:
    explicit DatabaseException(const std::string& message) 
        : StrategyException("Database Error: " + message) {}
    
    DatabaseException(const std::string& message, int error_code)
        : StrategyException("Database Error: " + message, error_code) {}
};

/**
 * @brief 算法交互异常类
 * 
 * 与算法层交互时发生的异常
 */
class AlgorithmException : public StrategyException {
public:
    explicit AlgorithmException(const std::string& message) 
        : StrategyException("Algorithm Error: " + message) {}
    
    AlgorithmException(const std::string& message, int error_code)
        : StrategyException("Algorithm Error: " + message, error_code) {}
};

/**
 * @brief 配置异常类
 * 
 * 配置加载和解析相关的异常
 */
class ConfigException : public StrategyException {
public:
    explicit ConfigException(const std::string& message) 
        : StrategyException("Config Error: " + message) {}
    
    ConfigException(const std::string& message, int error_code)
        : StrategyException("Config Error: " + message, error_code) {}
};

/**
 * @brief 玩家服务异常类
 * 
 * 玩家服务相关的异常
 */
class PlayerServiceException : public StrategyException {
public:
    explicit PlayerServiceException(const std::string& message) 
        : StrategyException("Player Service Error: " + message) {}
    
    PlayerServiceException(const std::string& message, int error_code)
        : StrategyException("Player Service Error: " + message, error_code) {}
};

/**
 * @brief 事件调度异常类
 * 
 * 事件调度相关的异常
 */
class EventSchedulerException : public StrategyException {
public:
    explicit EventSchedulerException(const std::string& message) 
        : StrategyException("Event Scheduler Error: " + message) {}
    
    EventSchedulerException(const std::string& message, int error_code)
        : StrategyException("Event Scheduler Error: " + message, error_code) {}
};

/**
 * @brief 网络异常类
 * 
 * 网络通信相关的异常
 */
class NetworkException : public StrategyException {
public:
    explicit NetworkException(const std::string& message) 
        : StrategyException("Network Error: " + message) {}
    
    NetworkException(const std::string& message, int error_code)
        : StrategyException("Network Error: " + message, error_code) {}
};

} // namespace strategy

#endif // STRATEGY_EXCEPTION_H
