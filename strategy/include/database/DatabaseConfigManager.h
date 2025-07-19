/*
 * 文件名: DatabaseConfigManager.h
 * 说明: 数据库配置管理器，负责解析JSON配置
 * 作者: 彭承康
 * 创建时间: 2025-01-20
 */
#ifndef STRATEGY_DATABASE_DATABASECONFIGMANAGER_H
#define STRATEGY_DATABASE_DATABASECONFIGMANAGER_H

#include "database/DatabaseConfig.h"
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace strategy {

class DatabaseConfigManager {
public:
    /**
     * @brief 从JSON文件加载配置
     */
    static bool LoadFromFile(const std::string& config_file);
    
    /**
     * @brief 从JSON字符串加载配置
     */
    static bool LoadFromJson(const std::string& json_content);
    
    /**
     * @brief 获取指定名称的数据库配置
     */
    static std::shared_ptr<DatabaseConfig> GetConfig(const std::string& config_name);
    
    /**
     * @brief 获取默认数据库配置
     */
    static std::shared_ptr<DatabaseConfig> GetDefaultConfig();
    
    /**
     * @brief 设置默认配置名称
     */
    static void SetDefaultConfig(const std::string& config_name);
    
    /**
     * @brief 获取所有配置名称
     */
    static std::vector<std::string> GetAllConfigNames();

private:
    static std::map<std::string, std::shared_ptr<DatabaseConfig>> configs_;
    static std::string default_config_name_;
    
    static DatabaseType ParseDatabaseType(const std::string& type_str);
    static std::shared_ptr<DatabaseConfig> ParseDatabaseConfig(const std::string& json_object);
};

} // namespace strategy

#endif // STRATEGY_DATABASE_DATABASECONFIGMANAGER_H
