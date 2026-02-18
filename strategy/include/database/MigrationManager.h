/*
 * 文件名: MigrationManager.h
 * 说明: 数据库迁移管理器，负责数据库版本控制和结构变更管理
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 *
 * 本类提供数据库迁移功能，包括：
 * 1. 自动执行数据库结构升级
 * 2. 支持版本回滚操作
 * 3. 维护迁移历史记录
 * 4. 确保数据库结构与代码版本同步
 *
 * 使用示例：
 *   auto connection = std::make_shared<PostgreSQLConnection>(config);
 *   MigrationManager manager(connection);
 *   if (!manager.RunMigrations()) {
 *       // 处理迁移失败
 *   }
 */
#ifndef STRATEGY_DATABASE_MIGRATIONMANAGER_H
#define STRATEGY_DATABASE_MIGRATIONMANAGER_H

#include "IDatabaseConnection.h"
#include "DatabaseConfig.h"
#include <vector>
#include <string>
#include <memory>

namespace strategy {

/**
 * @brief 数据库迁移脚本结构
 * 
 * 每个迁移包含版本号、描述和正向/反向SQL脚本
 */
struct Migration {
    int version;                ///< 迁移版本号，必须递增且唯一
    std::string description;    ///< 迁移描述，用于日志和调试
    std::string up_sql;         ///< 正向迁移SQL（升级数据库结构）
    std::string down_sql;       ///< 反向迁移SQL（回滚数据库结构）
};

/**
 * @brief 数据库迁移管理器
 * 
 * 负责管理数据库结构的版本控制，支持自动升级和回滚操作。
 * 迁移脚本按版本号顺序执行，确保数据库结构与应用程序版本同步。
 * 
 * 核心功能：
 * - 自动检测当前数据库版本
 * - 执行待处理的迁移脚本
 * - 支持回滚到指定版本
 * - 维护迁移执行历史
 * 
 * 线程安全性：本类不是线程安全的，需要外部同步
 */
class MigrationManager {
public:
    /**
     * @brief 构造函数
     * @param connection 数据库连接对象，必须已连接且有效
     * @param db_type 数据库类型，用于生成对应的DDL
     * @throws std::invalid_argument 如果连接为空或未连接
     */
    explicit MigrationManager(std::shared_ptr<IDatabaseConnection> connection,
                              DatabaseType db_type = DatabaseType::SQLITE);
    
    /**
     * @brief 执行所有待处理的数据库迁移
     * 
     * 检查当前数据库版本，执行所有版本号大于当前版本的迁移脚本。
     * 迁移按版本号升序执行，如果任何一个迁移失败，整个过程回滚。
     * 
     * @return true 所有迁移成功执行
     * @return false 迁移过程中发生错误
     * 
     * @note 执行前会自动创建迁移历史表（如果不存在）
     * @note 所有迁移在事务中执行，保证原子性
     */
    bool RunMigrations();
    
    /**
     * @brief 回滚数据库到指定版本
     * 
     * 执行反向迁移脚本，将数据库结构回滚到目标版本。
     * 回滚按版本号降序执行，直到达到目标版本。
     * 
     * @param target_version 目标版本号，必须小于当前版本
     * @return true 回滚成功完成
     * @return false 回滚过程中发生错误
     * 
     * @warning 回滚操作可能导致数据丢失，请谨慎使用
     * @note 如果目标版本等于或大于当前版本，操作将被忽略
     */
    bool RollbackMigration(int target_version);
    
    /**
     * @brief 获取当前数据库版本号
     * 
     * 从迁移历史表中查询最新的已执行迁移版本。
     * 如果迁移表不存在或为空，返回版本0。
     * 
     * @return int 当前数据库版本号，0表示未执行任何迁移
     * @throws DatabaseException 查询迁移表失败时抛出
     */
    int GetCurrentVersion();
    
private:
    std::shared_ptr<IDatabaseConnection> connection_;  ///< 数据库连接对象
    std::vector<Migration> migrations_;                ///< 所有可用的迁移脚本列表
    DatabaseType db_type_;                             ///< 数据库类型

    /**
     * @brief 从配置文件或代码中加载迁移脚本
     *
     * 初始化 migrations_ 向量，包含所有定义的迁移脚本。
     * 迁移脚本可以从文件系统、嵌入资源或硬编码中加载。
     *
     * @note 迁移脚本必须按版本号排序
     * @note 版本号必须连续且唯一
     */
    void LoadMigrations();

    /**
     * @brief 加载SQLite兼容的迁移脚本
     */
    void LoadSQLiteMigrations();

    /**
     * @brief 加载PostgreSQL兼容的迁移脚本
     */
    void LoadPostgreSQLMigrations();
    
    /**
     * @brief 创建迁移历史表
     * 
     * 如果迁移历史表不存在，创建用于记录已执行迁移的表。
     * 表结构包括：版本号、描述、执行时间等字段。
     * 
     * @return true 表创建成功或已存在
     * @return false 表创建失败
     * 
     * @note 表名通常为 "schema_migrations" 或 "migration_history"
     */
    bool CreateMigrationTable();
};

} // namespace strategy
#endif // STRATEGY_DATABASE_MIGRATIONMANAGER_H
