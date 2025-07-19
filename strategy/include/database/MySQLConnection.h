/*
 * 文件名: MySQLConnection.h
 * 说明: MySQL数据库连接实现，基于MySQL C++ Connector
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 本类实现MySQL数据库的连接和操作功能，包括：
 * 1. MySQL服务器连接管理
 * 2. SQL查询和更新操作
 * 3. 事务处理支持
 * 4. 参数化查询防SQL注入
 *
 * 使用示例：
 *   DatabaseConfig config;
 *   config.host = "localhost";
 *   config.database = "game_db";
 *   MySQLConnection conn(config);
 *   conn.Connect();
 *   auto result = conn.ExecuteQuery("SELECT * FROM players WHERE id = ?", {player_id});
 */
#ifndef STRATEGY_DATABASE_MYSQLCONNECTION_H
#define STRATEGY_DATABASE_MYSQLCONNECTION_H

#include "IDatabaseConnection.h"
#include "DatabaseConfig.h"
#include <mysql/mysql.h>
#include <memory>
#include <string>

namespace strategy {

/**
 * @brief MySQL数据库连接实现类
 * 
 * 基于MySQL C API实现的数据库连接类，提供完整的MySQL数据库操作功能。
 * 支持连接管理、查询执行、事务处理和参数化查询。
 * 
 * 核心功能：
 * - MySQL服务器连接和断开
 * - SQL查询和更新操作
 * - 预处理语句支持
 * - 事务的开始、提交和回滚
 * - 连接状态检查和重连
 * 
 * 线程安全性：本类不是线程安全的，每个线程应使用独立的连接实例
 */
class MySQLConnection : public IDatabaseConnection {
public:
    /**
     * @brief 构造函数
     * @param config 数据库配置信息，包含主机、端口、用户名、密码等
     * @throws std::invalid_argument 如果配置参数无效
     * 
     * @note 构造函数不会立即连接数据库，需要显式调用Connect()
     */
    explicit MySQLConnection(const DatabaseConfig& config);
    
    /**
     * @brief 析构函数
     * 
     * 自动断开数据库连接，清理MySQL资源。
     * 如果有未提交的事务，会自动回滚。
     */
    ~MySQLConnection() override;
    
    /**
     * @brief 连接到MySQL数据库
     * 
     * 使用配置信息建立到MySQL服务器的连接。
     * 设置字符集、超时参数和其他连接选项。
     * 
     * @return true 连接成功建立
     * @return false 连接失败
     * @throws DatabaseException 连接过程中发生严重错误
     * 
     * @note 连接失败时会记录详细的错误信息
     * @note 支持连接重试机制
     */
    bool Connect() override;
    
    /**
     * @brief 断开数据库连接
     * 
     * 关闭与MySQL服务器的连接，释放相关资源。
     * 如果有活跃事务，会先回滚再断开连接。
     * 
     * @note 断开后需要重新调用Connect()才能使用
     */
    void Disconnect() override;
    
    /**
     * @brief 检查连接是否有效
     * @return true 连接正常且可用
     * @return false 连接已断开或异常
     * 
     * @note 会发送ping命令到服务器验证连接状态
     */
    bool IsConnected() const override;
    
    /**
     * @brief 执行SQL查询语句
     * 
     * 执行SELECT等查询语句，返回结果集。
     * 支持参数化查询，自动处理参数绑定和类型转换。
     * 
     * @param query SQL查询语句，可包含?占位符
     * @param params 查询参数列表，按顺序对应占位符
     * @return QueryResult 查询结果集
     * @throws QueryException 查询执行失败
     * @throws std::runtime_error 连接未建立
     * 
     * @note 参数会自动进行SQL转义，防止注入攻击
     * @note 大结果集会使用流式读取以节省内存
     */
    QueryResult ExecuteQuery(const std::string& query, 
                           const std::vector<std::any>& params = {}) override;
    
    /**
     * @brief 执行SQL更新语句
     * 
     * 执行INSERT、UPDATE、DELETE等修改语句。
     * 支持参数化查询，返回受影响的行数。
     * 
     * @param query SQL更新语句，可包含?占位符
     * @param params 查询参数列表，按顺序对应占位符
     * @return int 受影响的行数，-1表示执行失败
     * @throws QueryException 查询执行失败
     * @throws std::runtime_error 连接未建立
     * 
     * @note 在事务中执行时，需要显式提交才能生效
     */
    int ExecuteUpdate(const std::string& query, 
                     const std::vector<std::any>& params = {}) override;
    
    /**
     * @brief 开始数据库事务
     * 
     * 开启一个新的事务，后续的更新操作将在事务中执行。
     * MySQL默认使用READ COMMITTED隔离级别。
     * 
     * @return true 事务开始成功
     * @return false 事务开始失败
     * @throws DatabaseException 事务操作失败
     * 
     * @note 嵌套事务会使用保存点机制
     * @note 事务开始后必须调用Commit()或Rollback()
     */
    bool BeginTransaction() override;
    
    /**
     * @brief 提交当前事务
     * 
     * 将事务中的所有更改永久保存到数据库。
     * 提交成功后事务结束，连接回到自动提交模式。
     * 
     * @return true 事务提交成功
     * @return false 事务提交失败
     * @throws DatabaseException 提交过程中发生错误
     * 
     * @note 提交失败时事务仍然活跃，可以选择重试或回滚
     */
    bool CommitTransaction() override;
    
    /**
     * @brief 回滚当前事务
     * 
     * 撤销事务中的所有更改，恢复到事务开始前的状态。
     * 回滚后事务结束，连接回到自动提交模式。
     * 
     * @return true 事务回滚成功
     * @return false 事务回滚失败
     * @throws DatabaseException 回滚过程中发生错误
     * 
     * @note 即使回滚失败，事务也会被强制终止
     */
    bool RollbackTransaction() override;
    
private:
    DatabaseConfig config_;        ///< 数据库配置信息
    MYSQL* mysql_connection_;      ///< MySQL连接句柄
    bool in_transaction_;          ///< 事务状态标志
    
    /**
     * @brief 初始化MySQL库
     * 
     * 初始化MySQL客户端库，设置全局选项。
     * 在第一次创建连接时调用。
     * 
     * @return true 初始化成功
     * @return false 初始化失败
     */
    bool InitializeMySQL();
    
    /**
     * @brief 设置连接选项
     * 
     * 配置MySQL连接的各种选项，如字符集、超时时间等。
     * 在建立连接前调用。
     */
    void SetConnectionOptions();
    
    /**
     * @brief 绑定查询参数
     * 
     * 将参数绑定到预处理语句中，处理类型转换和NULL值。
     * 
     * @param stmt MySQL预处理语句句柄
     * @param params 参数列表
     * @return true 绑定成功
     * @return false 绑定失败
     */
    bool BindParameters(MYSQL_STMT* stmt, const std::vector<std::any>& params);
    
    /**
     * @brief 处理MySQL错误
     * 
     * 获取最后的MySQL错误信息，记录日志并抛出相应异常。
     * 
     * @param operation 发生错误的操作名称
     * @throws DatabaseException 包含详细错误信息的异常
     */
    void HandleMySQLError(const std::string& operation);
};

} // namespace strategy
#endif // STRATEGY_DATABASE_MYSQLCONNECTION_H
