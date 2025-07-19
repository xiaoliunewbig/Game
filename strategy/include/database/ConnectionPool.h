/*
 * 文件名: ConnectionPool.h
 * 说明: 数据库连接池管理器，提供高效的数据库连接复用机制
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 本类实现数据库连接池功能，包括：
 * 1. 连接的创建、复用和回收
 * 2. 线程安全的连接获取和归还
 * 3. 连接池大小的动态管理
 * 4. 连接健康检查和自动重连
 *
 * 使用示例：
 *   DatabaseConfig config = {...};
 *   ConnectionPool pool(config);
 *   auto conn = pool.GetConnection();
 *   // 使用连接执行查询
 *   pool.ReturnConnection(conn);
 */
#ifndef STRATEGY_DATABASE_CONNECTIONPOOL_H
#define STRATEGY_DATABASE_CONNECTIONPOOL_H

#include "IDatabaseConnection.h"
#include "DatabaseConfig.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>

namespace strategy {

/**
 * @brief 数据库连接池管理器
 * 
 * 提供高效的数据库连接复用机制，避免频繁创建和销毁连接的开销。
 * 支持多线程并发访问，自动管理连接的生命周期。
 * 
 * 核心功能：
 * - 预创建指定数量的数据库连接
 * - 线程安全的连接获取和归还
 * - 连接健康检查和自动重连
 * - 连接池大小的动态调整
 * - 连接超时和清理机制
 * 
 * 线程安全性：本类是线程安全的，支持多线程并发访问
 */
class ConnectionPool {
public:
    /**
     * @brief 构造函数
     * @param config 数据库配置信息，包含连接参数和池配置
     * @throws std::invalid_argument 如果配置无效
     * @throws DatabaseException 如果初始连接创建失败
     * 
     * @note 构造时会预创建最小数量的连接
     */
    explicit ConnectionPool(const DatabaseConfig& config);
    
    /**
     * @brief 析构函数
     * 
     * 关闭所有活跃连接，清理资源。
     * 等待所有借出的连接归还后再销毁。
     */
    ~ConnectionPool();
    
    /**
     * @brief 从连接池获取一个可用连接
     * 
     * 如果池中有可用连接，立即返回；否则创建新连接（不超过最大限制）。
     * 如果达到最大连接数且无可用连接，将阻塞等待直到有连接归还。
     * 
     * @return std::shared_ptr<IDatabaseConnection> 数据库连接对象
     * @throws DatabaseException 如果无法创建新连接
     * @throws std::runtime_error 如果连接池已关闭
     * 
     * @note 返回的连接保证是已连接且健康的
     * @note 使用完毕后必须调用 ReturnConnection 归还
     */
    std::shared_ptr<IDatabaseConnection> GetConnection();
    
    /**
     * @brief 将连接归还到连接池
     * 
     * 检查连接健康状态，如果连接正常则放回池中复用，
     * 否则销毁连接并在需要时创建新连接补充。
     * 
     * @param conn 要归还的数据库连接，不能为空
     * @throws std::invalid_argument 如果连接为空
     * 
     * @note 归还后不应再使用该连接对象
     * @note 如果连接已断开，会自动销毁而不是放回池中
     */
    void ReturnConnection(std::shared_ptr<IDatabaseConnection> conn);
    
    /**
     * @brief 获取连接池当前状态信息
     * @return std::pair<size_t, size_t> 返回 (当前连接数, 可用连接数)
     */
    std::pair<size_t, size_t> GetPoolStatus() const;
    
    /**
     * @brief 关闭连接池
     * 
     * 停止接受新的连接请求，等待所有借出连接归还，
     * 然后关闭所有连接并清理资源。
     * 
     * @note 关闭后的连接池不能再使用
     */
    void Shutdown();
    
private:
    DatabaseConfig config_;                                           ///< 数据库配置信息
    std::queue<std::shared_ptr<IDatabaseConnection>> available_connections_; ///< 可用连接队列
    mutable std::mutex mutex_;                                        ///< 保护连接池状态的互斥锁
    std::condition_variable condition_;                               ///< 用于等待可用连接的条件变量
    std::atomic<size_t> current_size_;                               ///< 当前连接池总大小
    std::atomic<size_t> borrowed_count_;                             ///< 当前借出的连接数量
    std::atomic<bool> shutdown_;                                     ///< 连接池关闭标志
    
    size_t min_size_;                                                ///< 连接池最小大小
    size_t max_size_;                                                ///< 连接池最大大小
    
    /**
     * @brief 创建新的数据库连接
     * @return std::shared_ptr<IDatabaseConnection> 新创建的连接
     * @throws DatabaseException 如果连接创建失败
     */
    std::shared_ptr<IDatabaseConnection> CreateConnection();
    
    /**
     * @brief 检查连接是否健康
     * @param conn 要检查的连接
     * @return true 连接健康可用
     * @return false 连接已断开或异常
     */
    bool IsConnectionHealthy(const std::shared_ptr<IDatabaseConnection>& conn);
    
    /**
     * @brief 初始化连接池
     * 
     * 预创建最小数量的连接放入池中。
     * 在构造函数中调用。
     */
    void InitializePool();
};

} // namespace strategy
#endif // STRATEGY_DATABASE_CONNECTIONPOOL_H
