/*
 * 文件名: JsonConfigParser.h
 * 说明: JSON格式数据库配置解析器，支持多数据库配置管理
 * 作者: 彭承康
 * 创建时间: 2026-02-18
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
 *
 * 使用简单的行式JSON解析器处理平面配置结构。
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
    static std::string ExtractStringValue(const std::string& json, const std::string& key);
    static int ExtractIntValue(const std::string& json, const std::string& key, int default_value);
};

} // namespace strategy
#endif // STRATEGY_DATABASE_JSONCONFIGPARSER_H
