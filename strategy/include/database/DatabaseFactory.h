/*
 * 文件名: DatabaseFactory.h
 * 说明: 数据库连接工厂
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#ifndef STRATEGY_DATABASE_DATABASEFACTORY_H
#define STRATEGY_DATABASE_DATABASEFACTORY_H

#include "database/IDatabaseConnection.h"
#include "database/DatabaseConfig.h"
#include <memory>

namespace strategy {

class DatabaseFactory {
public:
    /**
     * @brief 根据配置创建数据库连接
     */
    static std::unique_ptr<IDatabaseConnection> CreateConnection(const DatabaseConfig& config);
    
    /**
     * @brief 根据配置名称创建数据库连接
     */
    static std::unique_ptr<IDatabaseConnection> CreateConnection(const std::string& config_name);
    
    /**
     * @brief 创建默认数据库连接
     */
    static std::unique_ptr<IDatabaseConnection> CreateDefaultConnection();

private:
    static std::unique_ptr<IDatabaseConnection> CreatePostgreSQLConnection(const DatabaseConfig& config);
    static std::unique_ptr<IDatabaseConnection> CreateMySQLConnection(const DatabaseConfig& config);
    static std::unique_ptr<IDatabaseConnection> CreateSQLiteConnection(const DatabaseConfig& config);
    static std::unique_ptr<IDatabaseConnection> CreateMongoDBConnection(const DatabaseConfig& config);
};

} // namespace strategy

#endif // STRATEGY_DATABASE_DATABASEFACTORY_H