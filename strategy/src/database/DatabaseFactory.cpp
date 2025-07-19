/*
 * 文件名: DatabaseFactory.cpp
 * 说明: 数据库连接工厂实现
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 */
#include "database/DatabaseFactory.h"
#include "database/PostgreSQLConnection.h"
#include "database/DatabaseConfigManager.h"
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
    return std::make_unique<PostgreSQLConnection>(config);
}

std::unique_ptr<IDatabaseConnection> DatabaseFactory::CreateMySQLConnection(const DatabaseConfig& config) {
    // TODO: 实现MySQL连接
    (void)config;  // 避免未使用参数警告
    std::cerr << "MySQL连接暂未实现" << std::endl;
    throw std::runtime_error("MySQL连接暂未实现");
}

std::unique_ptr<IDatabaseConnection> DatabaseFactory::CreateSQLiteConnection(const DatabaseConfig& config) {
    // TODO: 实现SQLite连接
    (void)config;  // 避免未使用参数警告
    std::cerr << "SQLite连接暂未实现" << std::endl;
    throw std::runtime_error("SQLite连接暂未实现");
}

std::unique_ptr<IDatabaseConnection> DatabaseFactory::CreateMongoDBConnection(const DatabaseConfig& config) {
    // TODO: 实现MongoDB连接
    (void)config;  // 避免未使用参数警告
    std::cerr << "MongoDB连接暂未实现" << std::endl;
    throw std::runtime_error("MongoDB连接暂未实现");
}

} // namespace strategy
