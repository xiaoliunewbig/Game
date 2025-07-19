/*
 * 文件名: AsyncLogService.h
 * 说明: 策略层异步日志服务头文件。
 * 作者: 彭承康
 * 创建时间: 2025-07-13
 *
 * 本服务使用装饰器模式，为其他日志服务（如控制台、文件）提供异步功能。
 * 它内部维护一个线程安全的日志消息队列和一个后台工作线程，
 * 以解耦日志生成与I/O操作，避免阻塞主线程。
 */
#ifndef STRATEGY_ASYNCLOGSERVICE_H
#define STRATEGY_ASYNCLOGSERVICE_H

#include "ILogService.h"
#include <vector>
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace strategy {

/**
 * @brief 异步日志服务
 * 装饰一个或多个ILogService实例，使其具备异步处理能力。
 */
class AsyncLogService : public ILogService {
public:
    /**
     * @brief 构造函数
     * @param services 日志服务池，日志将被分发到池中的所有服务。
     */
    explicit AsyncLogService(std::vector<std::shared_ptr<ILogService>> services);
    
    /**
     * @brief 析构函数
     * 安全地停止后台线程，并确保所有缓冲区的日志都被处理。
     */
    virtual ~AsyncLogService();

    /**
     * @brief 将日志消息推入队列（非阻塞）
     * 该方法是线程安全的。
     */
    void Log(LogLevel level,
             const std::string& file,
             int line,
             const std::string& type,
             const std::string& message) override;

private:
    /**
     * @brief 后台线程的工作函数
     * 从队列中取出日志消息并分发给日志服务池中的所有服务。
     */
    void ProcessQueue();

    // 内部日志消息结构体
    struct LogMessage {
        LogLevel level;
        std::string file;
        int line;
        std::string type;
        std::string message;
    };

    std::vector<std::shared_ptr<ILogService>> service_pool_; // 日志服务池
    std::queue<LogMessage> log_queue_;                       // 日志消息队列
    std::mutex queue_mutex_;                                 // 队列互斥锁
    std::condition_variable condition_;                      // 条件变量，用于线程同步
    std::thread worker_thread_;                              // 后台工作线程
    bool stop_flag_ = false;                                 // 线程停止标志
};

} // namespace strategy

#endif // STRATEGY_ASYNCLOGSERVICE_H 