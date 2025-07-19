/*
 * 文件名: PerformanceMonitor.cpp
 * 说明: 性能监控服务实现
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 */

#include "monitor/PerformanceMonitor.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <sstream>

#ifdef __linux__
#include <sys/resource.h>
#include <unistd.h>
#include <fstream>
#include <thread>
#endif

namespace strategy {

PerformanceMonitor& PerformanceMonitor::GetInstance() {
    static PerformanceMonitor instance;
    return instance;
}

void PerformanceMonitor::StartTimer(const std::string& name) {
    if (!enabled_) return;

    std::lock_guard<std::mutex> lock(monitor_mutex_);
    
    auto& timer = timers_[name];
    timer.start_time = std::chrono::steady_clock::now();
    timer.is_running = true;
}

double PerformanceMonitor::EndTimer(const std::string& name) {
    if (!enabled_) return 0.0;

    auto end_time = std::chrono::steady_clock::now();
    
    std::lock_guard<std::mutex> lock(monitor_mutex_);
    
    auto it = timers_.find(name);
    if (it == timers_.end() || !it->second.is_running) {
        return 0.0;
    }

    auto& timer = it->second;
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - timer.start_time).count() / 1000.0;  // 转换为毫秒

    timer.is_running = false;
    UpdateStats(timer.stats, duration);
    CheckWarningThreshold(name, duration);

    return duration;
}

double PerformanceMonitor::GetAverageTime(const std::string& name) const {
    std::lock_guard<std::mutex> lock(monitor_mutex_);
    
    auto it = timers_.find(name);
    if (it != timers_.end()) {
        return it->second.stats.average_time;
    }
    return 0.0;
}

PerformanceStats PerformanceMonitor::GetStats(const std::string& name) const {
    std::lock_guard<std::mutex> lock(monitor_mutex_);
    
    auto it = timers_.find(name);
    if (it != timers_.end()) {
        return it->second.stats;
    }
    return PerformanceStats{};
}

std::unordered_map<std::string, PerformanceStats> PerformanceMonitor::GetAllStats() const {
    std::lock_guard<std::mutex> lock(monitor_mutex_);
    
    std::unordered_map<std::string, PerformanceStats> all_stats;
    for (const auto& pair : timers_) {
        all_stats[pair.first] = pair.second.stats;
    }
    return all_stats;
}

void PerformanceMonitor::RecordTime(const std::string& name, double duration_ms) {
    if (!enabled_) return;

    std::lock_guard<std::mutex> lock(monitor_mutex_);
    
    auto& timer = timers_[name];
    UpdateStats(timer.stats, duration_ms);
    CheckWarningThreshold(name, duration_ms);
}

void PerformanceMonitor::ResetTimer(const std::string& name) {
    std::lock_guard<std::mutex> lock(monitor_mutex_);
    
    auto it = timers_.find(name);
    if (it != timers_.end()) {
        it->second.stats = PerformanceStats{};
        it->second.is_running = false;
    }
}

void PerformanceMonitor::ResetAllTimers() {
    std::lock_guard<std::mutex> lock(monitor_mutex_);
    
    for (auto& pair : timers_) {
        pair.second.stats = PerformanceStats{};
        pair.second.is_running = false;
    }
}

void PerformanceMonitor::LogPerformanceStats() const {
    std::lock_guard<std::mutex> lock(monitor_mutex_);
    
    if (timers_.empty()) {
        std::cout << "没有性能统计数据" << std::endl;
        return;
    }

    std::cout << "\n=== 性能统计报告 ===" << std::endl;
    std::cout << std::left << std::setw(20) << "操作名称"
              << std::setw(10) << "调用次数"
              << std::setw(12) << "总时间(ms)"
              << std::setw(12) << "平均(ms)"
              << std::setw(12) << "最小(ms)"
              << std::setw(12) << "最大(ms)" << std::endl;
    std::cout << std::string(78, '-') << std::endl;

    for (const auto& pair : timers_) {
        const auto& stats = pair.second.stats;
        std::cout << std::left << std::setw(20) << pair.first
                  << std::setw(10) << stats.call_count
                  << std::setw(12) << std::fixed << std::setprecision(2) << stats.total_time
                  << std::setw(12) << std::fixed << std::setprecision(2) << stats.average_time
                  << std::setw(12) << std::fixed << std::setprecision(2) << stats.min_time
                  << std::setw(12) << std::fixed << std::setprecision(2) << stats.max_time
                  << std::endl;
    }
    std::cout << "===================" << std::endl;
}

SystemResourceInfo PerformanceMonitor::GetSystemResourceInfo() const {
    SystemResourceInfo info;

#ifdef __linux__
    // 获取CPU使用率
    std::ifstream stat_file("/proc/stat");
    if (stat_file.is_open()) {
        std::string line;
        std::getline(stat_file, line);
        // 简化的CPU使用率计算
        info.cpu_usage = 0.0;  // 实际实现需要更复杂的计算
    }

    // 获取内存信息
    std::ifstream meminfo_file("/proc/meminfo");
    if (meminfo_file.is_open()) {
        std::string line;
        while (std::getline(meminfo_file, line)) {
            if (line.find("MemTotal:") == 0) {
                std::istringstream iss(line);
                std::string label;
                size_t value;
                iss >> label >> value;
                info.memory_total_mb = value / 1024;
            } else if (line.find("MemAvailable:") == 0) {
                std::istringstream iss(line);
                std::string label;
                size_t available;
                iss >> label >> available;
                info.memory_used_mb = info.memory_total_mb - (available / 1024);
                if (info.memory_total_mb > 0) {
                    info.memory_usage = (double)info.memory_used_mb / info.memory_total_mb * 100.0;
                }
            }
        }
    }

    // 获取线程数
    info.active_threads = std::thread::hardware_concurrency();
#endif

    return info;
}

void PerformanceMonitor::SetEnabled(bool enabled) {
    std::lock_guard<std::mutex> lock(monitor_mutex_);
    enabled_ = enabled;
}

bool PerformanceMonitor::IsEnabled() const {
    std::lock_guard<std::mutex> lock(monitor_mutex_);
    return enabled_;
}

void PerformanceMonitor::SetWarningThreshold(const std::string& name, double threshold_ms) {
    std::lock_guard<std::mutex> lock(monitor_mutex_);
    warning_thresholds_[name] = threshold_ms;
}

void PerformanceMonitor::UpdateStats(PerformanceStats& stats, double duration_ms) {
    stats.call_count++;
    stats.total_time += duration_ms;
    stats.average_time = stats.total_time / stats.call_count;
    stats.last_call_time = std::chrono::steady_clock::now();

    if (stats.call_count == 1) {
        stats.min_time = duration_ms;
        stats.max_time = duration_ms;
    } else {
        stats.min_time = std::min(stats.min_time, duration_ms);
        stats.max_time = std::max(stats.max_time, duration_ms);
    }
}

void PerformanceMonitor::CheckWarningThreshold(const std::string& name, double duration_ms) const {
    auto it = warning_thresholds_.find(name);
    if (it != warning_thresholds_.end() && duration_ms > it->second) {
        std::cerr << "性能警告: " << name << " 执行时间 " << duration_ms 
                  << "ms 超过阈值 " << it->second << "ms" << std::endl;
    }
}

// ScopedTimer 实现
ScopedTimer::ScopedTimer(const std::string& name) : name_(name) {
    PerformanceMonitor::GetInstance().StartTimer(name_);
}

ScopedTimer::~ScopedTimer() {
    PerformanceMonitor::GetInstance().EndTimer(name_);
}

} // namespace strategy