/*
 * 文件名: JsonConfigParser.cpp
 * 说明: JSON配置解析器实现（简单行式JSON解析）
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#include "database/JsonConfigParser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>

namespace strategy {

std::string JsonConfigParser::ExtractStringValue(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) {
        return "";
    }

    // Find the colon after the key
    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos) {
        return "";
    }

    // Find the opening quote of the value
    size_t start = json.find('"', pos + 1);
    if (start == std::string::npos) {
        return "";
    }

    // Find the closing quote
    size_t end = json.find('"', start + 1);
    if (end == std::string::npos) {
        return "";
    }

    return json.substr(start + 1, end - start - 1);
}

int JsonConfigParser::ExtractIntValue(const std::string& json, const std::string& key, int default_value) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) {
        return default_value;
    }

    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos) {
        return default_value;
    }

    // Skip whitespace
    ++pos;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) {
        ++pos;
    }

    // Check if it's a quoted number
    if (pos < json.size() && json[pos] == '"') {
        size_t end = json.find('"', pos + 1);
        if (end != std::string::npos) {
            try {
                return std::stoi(json.substr(pos + 1, end - pos - 1));
            } catch (...) {
                return default_value;
            }
        }
    }

    // Parse as numeric literal
    try {
        return std::stoi(json.substr(pos));
    } catch (...) {
        return default_value;
    }
}

std::string JsonConfigParser::ProcessEnvironmentVariables(const std::string& value) {
    std::string result = value;
    size_t pos = 0;

    while ((pos = result.find("${", pos)) != std::string::npos) {
        size_t end = result.find('}', pos);
        if (end == std::string::npos) {
            break;
        }

        std::string var_name = result.substr(pos + 2, end - pos - 2);
        const char* env_val = std::getenv(var_name.c_str());
        std::string replacement = env_val ? env_val : "";
        result.replace(pos, end - pos + 1, replacement);
        pos += replacement.size();
    }

    return result;
}

DatabaseType JsonConfigParser::ParseDatabaseType(const std::string& type_str) {
    std::string lower = type_str;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (lower == "postgresql" || lower == "postgres" || lower == "pgsql") {
        return DatabaseType::POSTGRESQL;
    } else if (lower == "mysql") {
        return DatabaseType::MYSQL;
    } else if (lower == "sqlite") {
        return DatabaseType::SQLITE;
    } else if (lower == "mongodb" || lower == "mongo") {
        return DatabaseType::MONGODB;
    }

    throw JsonParseException("未知数据库类型: " + type_str);
}

std::shared_ptr<DatabaseConfig> JsonConfigParser::ParseSingleConfig(const std::string& json_object) {
    auto config = std::make_shared<DatabaseConfig>();

    std::string type_str = ExtractStringValue(json_object, "type");
    if (!type_str.empty()) {
        config->type = ParseDatabaseType(type_str);
    } else {
        config->type = DatabaseType::POSTGRESQL;
    }

    config->host = ProcessEnvironmentVariables(ExtractStringValue(json_object, "host"));
    if (config->host.empty()) {
        config->host = "localhost";
    }

    config->port = ExtractIntValue(json_object, "port", 5432);
    config->database = ProcessEnvironmentVariables(ExtractStringValue(json_object, "database"));
    config->username = ProcessEnvironmentVariables(ExtractStringValue(json_object, "username"));
    config->password = ProcessEnvironmentVariables(ExtractStringValue(json_object, "password"));
    config->max_connections = ExtractIntValue(json_object, "max_connections", 10);
    config->min_connections = ExtractIntValue(json_object, "min_connections", 1);
    config->connection_timeout = ExtractIntValue(json_object, "connection_timeout", 30);

    return config;
}

std::map<std::string, std::shared_ptr<DatabaseConfig>>
JsonConfigParser::ParseDatabaseConfigs(const std::string& json_content) {
    std::map<std::string, std::shared_ptr<DatabaseConfig>> configs;

    // Find the "databases" block
    size_t db_pos = json_content.find("\"databases\"");
    if (db_pos == std::string::npos) {
        // Try treating the whole content as a single config
        auto config = ParseSingleConfig(json_content);
        configs["default"] = config;
        return configs;
    }

    // Find each named database block by looking for quoted keys followed by { }
    size_t search_start = json_content.find('{', db_pos);
    if (search_start == std::string::npos) {
        throw JsonParseException("databases块格式错误");
    }

    // Simple approach: find all "name": { ... } pairs within databases block
    size_t pos = search_start + 1;
    while (pos < json_content.size()) {
        // Find next quoted key
        size_t key_start = json_content.find('"', pos);
        if (key_start == std::string::npos || key_start >= json_content.size() - 1) {
            break;
        }

        size_t key_end = json_content.find('"', key_start + 1);
        if (key_end == std::string::npos) {
            break;
        }

        std::string name = json_content.substr(key_start + 1, key_end - key_start - 1);

        // Find the opening brace of the value object
        size_t obj_start = json_content.find('{', key_end);
        if (obj_start == std::string::npos) {
            break;
        }

        // Find matching closing brace (simple depth tracking)
        int depth = 1;
        size_t obj_end = obj_start + 1;
        while (obj_end < json_content.size() && depth > 0) {
            if (json_content[obj_end] == '{') {
                ++depth;
            } else if (json_content[obj_end] == '}') {
                --depth;
            }
            ++obj_end;
        }

        if (depth != 0) {
            throw JsonParseException("JSON格式错误: 未闭合的大括号");
        }

        std::string obj_content = json_content.substr(obj_start, obj_end - obj_start);
        try {
            configs[name] = ParseSingleConfig(obj_content);
        } catch (const std::exception& e) {
            std::cerr << "JsonConfigParser: 解析配置 '" << name << "' 失败: " << e.what() << std::endl;
        }

        pos = obj_end;
    }

    return configs;
}

std::map<std::string, std::shared_ptr<DatabaseConfig>>
JsonConfigParser::ParseDatabaseConfigsFromFile(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw JsonParseException("无法打开文件: " + file_path);
    }

    std::ostringstream oss;
    oss << file.rdbuf();
    return ParseDatabaseConfigs(oss.str());
}

std::vector<std::string> JsonConfigParser::ValidateConfig(const DatabaseConfig& config) {
    std::vector<std::string> errors;

    // SQLite只需要database路径，不需要host和port
    if (config.type != DatabaseType::SQLITE) {
        if (config.host.empty()) {
            errors.push_back("host不能为空");
        }
        if (config.port <= 0 || config.port > 65535) {
            errors.push_back("port必须在1-65535之间");
        }
    }
    if (config.database.empty()) {
        errors.push_back("database不能为空");
    }
    if (config.max_connections < 1) {
        errors.push_back("max_connections必须大于0");
    }
    if (config.min_connections < 0) {
        errors.push_back("min_connections不能为负数");
    }
    if (config.min_connections > config.max_connections) {
        errors.push_back("min_connections不能大于max_connections");
    }
    if (config.connection_timeout < 1) {
        errors.push_back("connection_timeout必须大于0");
    }

    return errors;
}

} // namespace strategy
