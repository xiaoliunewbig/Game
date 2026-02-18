/*
 * 文件名: DatabaseFactory.cpp
 * 说明: 数据库连接工厂实现
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#include "database/DatabaseFactory.h"
#include "database/DatabaseConfigManager.h"
#ifdef HAS_PQXX
#include "database/PostgreSQLConnection.h"
#endif
#ifdef HAS_SQLITE
#include "database/SQLiteConnection.h"
#endif
#ifdef HAS_MYSQL
#include "database/MySQLConnection.h"
#endif
#include <stdexcept>
#include <iostream>

namespace strategy {

std::unique_ptr<IDatabaseConnection> DatabaseFactory::CreateConnection(const DatabaseConfig& config) {
    switch (config.type) {
        case DatabaseType::POSTGRESQL:
            return CreatePostgreSQLConnection(config);
        case DatabaseType::MYSQL:
            return CreateMySQLConnection(config);
        case DatabaseType::SQLITE:
            return CreateSQLiteConnection(config);
        case DatabaseType::MONGODB:
            return CreateMongoDBConnection(config);
        default:
            throw std::runtime_error("不支持的数据库类型");
    }
}

std::unique_ptr<IDatabaseConnection> DatabaseFactory::CreateConnection(const std::string& config_name) {
    auto config = DatabaseConfigManager::GetConfig(config_name);
    if (!config) {
        throw std::runtime_error("找不到配置: " + config_name);
    }
    return CreateConnection(*config);
}

std::unique_ptr<IDatabaseConnection> DatabaseFactory::CreateDefaultConnection() {
    auto config = DatabaseConfigManager::GetDefaultConfig();
    if (!config) {
        throw std::runtime_error("找不到默认数据库配置");
    }
    return CreateConnection(*config);
}

std::unique_ptr<IDatabaseConnection> DatabaseFactory::CreatePostgreSQLConnection(const DatabaseConfig& config) {
#ifdef HAS_PQXX
    return std::make_unique<PostgreSQLConnection>(config);
#else
    (void)config;
    std::cerr << "PostgreSQL支持未启用（需要编译时定义HAS_PQXX）" << std::endl;
    throw std::runtime_error("PostgreSQL支持未启用");
#endif
}

std::unique_ptr<IDatabaseConnection> DatabaseFactory::CreateMySQLConnection(const DatabaseConfig& config) {
#ifdef HAS_MYSQL
    return std::make_unique<MySQLConnection>(config);
#else
    (void)config;
    std::cerr << "MySQL支持未启用（需要编译时定义HAS_MYSQL）" << std::endl;
    throw std::runtime_error("MySQL支持未启用");
#endif
}

std::unique_ptr<IDatabaseConnection> DatabaseFactory::CreateSQLiteConnection(const DatabaseConfig& config) {
#ifdef HAS_SQLITE
    return std::make_unique<SQLiteConnection>(config);
#else
    (void)config;
    std::cerr << "SQLite支持未启用（需要编译时定义HAS_SQLITE）" << std::endl;
    throw std::runtime_error("SQLite支持未启用");
#endif
}

std::unique_ptr<IDatabaseConnection> DatabaseFactory::CreateMongoDBConnection(const DatabaseConfig& config) {
    // TODO: 实现MongoDB连接
    (void)config;  // 避免未使用参数警告
    std::cerr << "MongoDB连接暂未实现" << std::endl;
    throw std::runtime_error("MongoDB连接暂未实现");
}

} // namespace strategy
