/*
 * 文件名: DatabaseConfigManager.cpp
 * 说明: 数据库配置管理器实现
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#include "database/DatabaseConfigManager.h"
#include "database/JsonConfigParser.h"
#include "database/DatabaseException.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <iostream>

namespace strategy {

std::map<std::string, std::shared_ptr<DatabaseConfig>> DatabaseConfigManager::configs_;
std::string DatabaseConfigManager::default_config_name_ = "default";

bool DatabaseConfigManager::LoadFromFile(const std::string& config_file) {
    std::ifstream file(config_file);
    if (!file.is_open()) {
        std::cerr << "无法打开配置文件: " << config_file << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return LoadFromJson(buffer.str());
}

bool DatabaseConfigManager::LoadFromJson(const std::string& json_content) {
    try {
        // 简单的JSON解析 (实际项目中建议使用nlohmann/json或其他JSON库)
        // 这里为了演示，使用简化的解析方式
        (void)json_content;  // 避免未使用参数警告
        
        // 示例JSON格式:
        // {
        //   "databases": {
        //     "primary": {
        //       "type": "postgresql",
        //       "host": "localhost",
        //       "port": 5432,
        //       "database": "game_db",
        //       "username": "game_user",
        //       "password": "game_password"
        //     }
        //   },
        //   "default": "primary"
        // }
        
        // 这里简化处理，实际应该用JSON库
        configs_.clear();
        
        // 创建一个默认的SQLite配置
        auto default_config = std::make_shared<DatabaseConfig>();
        default_config->type = DatabaseType::SQLITE;
        default_config->database = "game_data.db";
        default_config->max_connections = 10;
        
        configs_["primary"] = default_config;
        configs_["default"] = default_config;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "解析JSON配置失败: " << e.what() << std::endl;
        return false;
    }
}

std::shared_ptr<DatabaseConfig> DatabaseConfigManager::GetConfig(const std::string& config_name) {
    auto it = configs_.find(config_name);
    if (it != configs_.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<DatabaseConfig> DatabaseConfigManager::GetDefaultConfig() {
    return GetConfig(default_config_name_);
}

void DatabaseConfigManager::SetDefaultConfig(const std::string& config_name) {
    default_config_name_ = config_name;
}

std::vector<std::string> DatabaseConfigManager::GetAllConfigNames() {
    std::vector<std::string> names;
    for (const auto& pair : configs_) {
        names.push_back(pair.first);
    }
    return names;
}

DatabaseType DatabaseConfigManager::ParseDatabaseType(const std::string& type_str) {
    std::string lower_type = type_str;
    std::transform(lower_type.begin(), lower_type.end(), lower_type.begin(), ::tolower);
    
    if (lower_type == "postgresql" || lower_type == "postgres") {
        return DatabaseType::POSTGRESQL;
    } else if (lower_type == "mysql") {
        return DatabaseType::MYSQL;
    } else if (lower_type == "sqlite") {
        return DatabaseType::SQLITE;
    } else if (lower_type == "mongodb" || lower_type == "mongo") {
        return DatabaseType::MONGODB;
    }
    
    return DatabaseType::POSTGRESQL; // 默认
}

} // namespace strategy
