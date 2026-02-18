/*
 * 文件名: ConnectionPool.cpp
 * 说明: 数据库连接池实现
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#include "database/ConnectionPool.h"
#include "database/DatabaseFactory.h"
#include <iostream>
#include <stdexcept>

namespace strategy {

ConnectionPool::ConnectionPool(const DatabaseConfig& config)
    : config_(config)
    , current_size_(0)
    , borrowed_count_(0)
    , shutdown_(false)
    , min_size_(static_cast<size_t>(config.min_connections))
    , max_size_(static_cast<size_t>(config.max_connections))
{
    if (min_size_ == 0) {
        min_size_ = 1;
    }
    if (max_size_ < min_size_) {
        max_size_ = min_size_;
    }
    InitializePool();
}

ConnectionPool::~ConnectionPool() {
    Shutdown();
}

void ConnectionPool::InitializePool() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (size_t i = 0; i < min_size_; ++i) {
        try {
            auto conn = CreateConnection();
            if (conn) {
                available_connections_.push(conn);
                ++current_size_;
            }
        } catch (const std::exception& e) {
            std::cerr << "ConnectionPool: 初始化连接失败: " << e.what() << std::endl;
        }
    }
    std::cout << "ConnectionPool: 初始化完成，创建了 " << current_size_.load()
              << " 个连接" << std::endl;
}

std::shared_ptr<IDatabaseConnection> ConnectionPool::CreateConnection() {
    auto conn = DatabaseFactory::CreateConnection(config_);
    if (conn && conn->Connect()) {
        return conn;
    }
    throw std::runtime_error("ConnectionPool: 创建连接失败");
}

bool ConnectionPool::IsConnectionHealthy(const std::shared_ptr<IDatabaseConnection>& conn) {
    return conn && conn->IsConnected();
}

std::shared_ptr<IDatabaseConnection> ConnectionPool::GetConnection() {
    std::unique_lock<std::mutex> lock(mutex_);

    if (shutdown_) {
        throw std::runtime_error("ConnectionPool: 连接池已关闭");
    }

    // Try to get an available connection
    while (!available_connections_.empty()) {
        auto conn = available_connections_.front();
        available_connections_.pop();

        if (IsConnectionHealthy(conn)) {
            ++borrowed_count_;
            return conn;
        }
        // Unhealthy connection, discard it
        --current_size_;
    }

    // No available connections; create a new one if possible
    if (current_size_ < max_size_) {
        try {
            auto conn = CreateConnection();
            ++current_size_;
            ++borrowed_count_;
            return conn;
        } catch (const std::exception& e) {
            std::cerr << "ConnectionPool: 创建新连接失败: " << e.what() << std::endl;
        }
    }

    // Wait for a connection to be returned
    condition_.wait(lock, [this]() {
        return !available_connections_.empty() || shutdown_;
    });

    if (shutdown_) {
        throw std::runtime_error("ConnectionPool: 连接池已关闭");
    }

    auto conn = available_connections_.front();
    available_connections_.pop();
    ++borrowed_count_;
    return conn;
}

void ConnectionPool::ReturnConnection(std::shared_ptr<IDatabaseConnection> conn) {
    if (!conn) {
        throw std::invalid_argument("ConnectionPool: 归还的连接为空");
    }

    std::lock_guard<std::mutex> lock(mutex_);
    --borrowed_count_;

    if (shutdown_) {
        --current_size_;
        return;
    }

    if (IsConnectionHealthy(conn)) {
        available_connections_.push(conn);
    } else {
        --current_size_;
    }

    condition_.notify_one();
}

std::pair<size_t, size_t> ConnectionPool::GetPoolStatus() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return {current_size_.load(), available_connections_.size()};
}

void ConnectionPool::Shutdown() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (shutdown_) {
            return;
        }
        shutdown_ = true;
    }

    condition_.notify_all();

    // Drain the queue
    std::lock_guard<std::mutex> lock(mutex_);
    while (!available_connections_.empty()) {
        auto conn = available_connections_.front();
        available_connections_.pop();
        if (conn) {
            conn->Disconnect();
        }
    }
    current_size_ = 0;

    std::cout << "ConnectionPool: 已关闭" << std::endl;
}

} // namespace strategy
