/*
 * 文件名: AsyncLogService.cpp
 * 说明: 策略层异步日志服务实现。
 * 作者: 彭承康
 * 创建时间: 2025-07-13
 */
#include "Log/AsyncLogService.h"

namespace strategy {

AsyncLogService::AsyncLogService(std::vector<std::shared_ptr<ILogService>> services)
    : service_pool_(std::move(services)) {
    worker_thread_ = std::thread(&AsyncLogService::ProcessQueue, this);
}

AsyncLogService::~AsyncLogService() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_flag_ = true;
    }
    condition_.notify_one();
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
}

void AsyncLogService::Log(LogLevel level,
                          const std::string& file,
                          int line,
                          const std::string& type,
                          const std::string& message) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        log_queue_.push({level, file, line, type, message});
    }
    condition_.notify_one();
}

void AsyncLogService::ProcessQueue() {
    while (true) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        condition_.wait(lock, [this] { return !log_queue_.empty() || stop_flag_; });

        if (stop_flag_ && log_queue_.empty()) {
            break;
        }

        // 为了减小锁的粒度，将队列中的消息移动到局部变量
        std::queue<LogMessage> processing_queue;
        log_queue_.swap(processing_queue);
        
        lock.unlock(); // 立即释放锁，允许主线程继续推入日志

        while (!processing_queue.empty()) {
            LogMessage msg = processing_queue.front();
            processing_queue.pop();
            for (const auto& service : service_pool_) {
                if (service) {
                    service->Log(msg.level, msg.file, msg.line, msg.type, msg.message);
                }
            }
        }
    }
}

} // namespace strategy 