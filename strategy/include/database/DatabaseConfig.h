/*
 * 文件名: DatabaseConfig.h
 * 说明: 数据库配置结构定义
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#ifndef STRATEGY_DATABASE_DATABASECONFIG_H
#define STRATEGY_DATABASE_DATABASECONFIG_H

#include <string>
#include <map>
#include <memory>

namespace strategy {

enum class DatabaseType {
    POSTGRESQL,
    MYSQL,
    SQLITE,
    MONGODB
};

struct DatabaseConfig {
    DatabaseType type;
    std::string host;
    int port;
    std::string database;
    std::string username;
    std::string password;
    std::map<std::string, std::string> extra_params;  // 额外参数
    
    // 连接池配置
    int max_connections = 10;
    int min_connections = 1;
    int connection_timeout = 30;  // 秒
    
    // 获取连接字符串
    std::string GetConnectionString() const;
};

} // namespace strategy

#endif // STRATEGY_DATABASE_DATABASECONFIG_H