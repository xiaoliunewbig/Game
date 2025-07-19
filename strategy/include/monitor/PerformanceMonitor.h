/*
 * 文件名: PerformanceMonitor.h
 * 说明: 性能监控服务，用于监控系统性能指标
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 提供性能监控功能，包括执行时间统计、内存使用监控、
 * 系统资源监控等。采用单例模式，支持多线程环境。
 */

#ifndef STRATEGY_PERFORMANCE_MONITOR_H
#define STRATEGY_PERFORMANCE_MONITOR_H

#include <chrono>
#include <string>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <memory>

namespace strategy {

/**
 * @brief 性能统计数据结构
 */
struct PerformanceStats {
    double total_time = 0.0;        ///< 总执行时间（毫秒）
    double min_time = 0.0;          ///< 最小执行时间（毫秒）
    double max_time = 0.0;          ///< 最大执行时间（毫秒）
    double average_time = 0.0;      ///< 平均执行时间（毫秒）
    int call_count = 0;             ///< 调用次数
    std::chrono::steady_clock::time_point last_call_time;  ///< 最后调用时间
};

/**
 * @brief 系统资源信息
 */
struct SystemResourceInfo {
    double cpu_usage = 0.0;         ///< CPU使用率（百分比）
    double memory_usage = 0.0;      ///< 内存使用率（百分比）
    size_t memory_used_mb = 0;      ///< 已使用内存（MB）
    size_t memory_total_mb = 0;     ///< 总内存（MB）
    int active_threads = 0;         ///< 活跃线程数
};

/**
 * @brief 性能监控服务
 * 
 * 提供系统性能监控功能，包括执行时间统计、资源使用监控等
 */
class PerformanceMonitor {
public:
    /**
     * @brief 获取单例实例
     * @return PerformanceMonitor实例引用
     */
    static PerformanceMonitor& GetInstance();

    /**
     * @brief 开始计时
     * @param name 计时器名称
     */
    void StartTimer(const std::string& name);

    /**
     * @brief 结束计时
     * @param name 计时器名称
     * @return 本次执行时间（毫秒）
     */
    double EndTimer(const std::string& name);

    /**
     * @brief 获取平均执行时间
     * @param name 计时器名称
     * @return 平均执行时间（毫秒）
     */
    double GetAverageTime(const std::string& name) const;

    /**
     * @brief 获取性能统计信息
     * @param name 计时器名称
     * @return 性能统计数据
     */
    PerformanceStats GetStats(const std::string& name) const;

    /**
     * @brief 获取所有性能统计信息
     * @return 所有计时器的统计数据
     */
    std::unordered_map<std::string, PerformanceStats> GetAllStats() const;

    /**
     * @brief 记录单次执行时间
     * @param name 操作名称
     * @param duration_ms 执行时间（毫秒）
     */
    void RecordTime(const std::string& name, double duration_ms);

    /**
     * @brief 重置指定计时器
     * @param name 计时器名称
     */
    void ResetTimer(const std::string& name);

    /**
     * @brief 重置所有计时器
     */
    void ResetAllTimers();

    /**
     * @brief 输出性能统计日志
     */
    void LogPerformanceStats() const;

    /**
     * @brief 获取系统资源信息
     * @return 系统资源使用情况
     */
    SystemResourceInfo GetSystemResourceInfo() const;

    /**
     * @brief 启用/禁用性能监控
     * @param enabled 是否启用
     */
    void SetEnabled(bool enabled);

    /**
     * @brief 检查性能监控是否启用
     * @return 是否启用
     */
    bool IsEnabled() const;

    /**
     * @brief 设置性能警告阈值
     * @param name 操作名称
     * @param threshold_ms 阈值（毫秒）
     */
    void SetWarningThreshold(const std::string& name, double threshold_ms);

private:
    PerformanceMonitor() = default;
    ~PerformanceMonitor() = default;
    PerformanceMonitor(const PerformanceMonitor&) = delete;
    PerformanceMonitor& operator=(const PerformanceMonitor&) = delete;

    /**
     * @brief 计时器数据结构
     */
    struct TimerData {
        std::chrono::steady_clock::time_point start_time;
        PerformanceStats stats;
        bool is_running = false;
    };

    mutable std::mutex monitor_mutex_;                              ///< 线程安全锁
    std::unordered_map<std::string, TimerData> timers_;            ///< 计时器数据
    std::unordered_map<std::string, double> warning_thresholds_;   ///< 警告阈值
    bool enabled_ = true;                                          ///< 是否启用监控

    /**
     * @brief 更新统计数据
     * @param stats 统计数据引用
     * @param duration_ms 执行时间
     */
    void UpdateStats(PerformanceStats& stats, double duration_ms);

    /**
     * @brief 检查是否超过警告阈值
     * @param name 操作名称
     * @param duration_ms 执行时间
     */
    void CheckWarningThreshold(const std::string& name, double duration_ms) const;
};

/**
 * @brief RAII风格的性能计时器
 * 
 * 自动在构造时开始计时，析构时结束计时
 */
class ScopedTimer {
public:
    /**
     * @brief 构造函数，开始计时
     * @param name 计时器名称
     */
    explicit ScopedTimer(const std::string& name);

    /**
     * @brief 析构函数，结束计时
     */
    ~ScopedTimer();

private:
    std::string name_;  ///< 计时器名称
};

// 便利宏定义
#define PERF_TIMER(name) strategy::ScopedTimer _timer(name)
#define PERF_START(name) strategy::PerformanceMonitor::GetInstance().StartTimer(name)
#define PERF_END(name) strategy::PerformanceMonitor::GetInstance().EndTimer(name)

} // namespace strategy

#endif // STRATEGY_PERFORMANCE_MONITOR_H
