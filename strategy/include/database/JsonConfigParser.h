/*
 * 文件名: JsonConfigParser.h
 * 说明: JSON格式数据库配置解析器，支持多数据库配置管理
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 */
#ifndef STRATEGY_DATABASE_JSONCONFIGPARSER_H
#define STRATEGY_DATABASE_JSONCONFIGPARSER_H

#include "DatabaseConfig.h"
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <stdexcept>

namespace strategy {

/**
 * @brief JSON配置解析异常
 */
class JsonParseException : public std::runtime_error {
public:
    explicit JsonParseException(const std::string& message) 
        : std::runtime_error("JSON配置解析错误: " + message) {}
};

/**
 * @brief JSON格式数据库配置解析器
 */
class JsonConfigParser {
public:
    static std::map<std::string, std::shared_ptr<DatabaseConfig>> 
        ParseDatabaseConfigs(const std::string& json_content);
    
    static std::map<std::string, std::shared_ptr<DatabaseConfig>> 
        ParseDatabaseConfigsFromFile(const std::string& file_path);
    
    static std::shared_ptr<DatabaseConfig> 
        ParseSingleConfig(const std::string& json_object);
    
    static std::vector<std::string> ValidateConfig(const DatabaseConfig& config);
    
private:
    static DatabaseType ParseDatabaseType(const std::string& type_str);
    static std::string ProcessEnvironmentVariables(const std::string& value);
    
    template<typename T>
    static T ParseValue(const void* json_value, const T& default_value);
    
    static const void* GetRequiredField(const void* json_obj, const std::string& field_name);
    static const void* GetOptionalField(const void* json_obj, const std::string& field_name);
};

} // namespace strategy
#endif // STRATEGY_DATABASE_JSONCONFIGPARSER_H
