/*
 * 文件名: ConfigService.cpp
 * 说明: 配置管理服务实现
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 */

#include "config/ConfigService.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <vector>

namespace strategy {

ConfigService& ConfigService::GetInstance() {
    static ConfigService instance;
    return instance;
}

bool ConfigService::LoadConfig(const std::string& config_file) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    config_file_path_ = config_file;
    
    std::ifstream file(config_file);
    if (!file.is_open()) {
        std::cerr << "无法打开配置文件: " << config_file << std::endl;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return ParseJsonContent(buffer.str());
}

bool ConfigService::LoadFromJson(const std::string& json_content) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    return ParseJsonContent(json_content);
}

bool ConfigService::ParseJsonContent(const std::string& json_content) {
    try {
        // 简化的JSON解析实现
        // 实际项目中建议使用 nlohmann/json 或其他专业JSON库
        
        config_data_.clear();
        
        // 示例配置解析（简化版本）
        std::istringstream iss(json_content);
        std::string line;
        
        while (std::getline(iss, line)) {
            // 移除空白字符
            line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
            
            // 跳过注释和空行
            if (line.empty() || line[0] == '#' || line.substr(0, 2) == "//") {
                continue;
            }
            
            // 查找键值对
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                std::string key = line.substr(0, colon_pos);
                std::string value = line.substr(colon_pos + 1);
                
                // 移除引号
                if (!key.empty() && key.front() == '"' && key.back() == '"') {
                    key = key.substr(1, key.length() - 2);
                }
                if (!value.empty() && value.front() == '"' && value.back() == '"') {
                    value = value.substr(1, value.length() - 2);
                }
                
                // 移除末尾的逗号
                if (!value.empty() && value.back() == ',') {
                    value.pop_back();
                }
                
                config_data_[key] = value;
            }
        }
        
        // 添加一些默认配置
        if (config_data_.empty()) {
            config_data_["server.host"] = "localhost";
            config_data_["server.port"] = "8080";
            config_data_["database.host"] = "localhost";
            config_data_["database.port"] = "5432";
            config_data_["database.name"] = "game_db";
            config_data_["log.level"] = "INFO";
            config_data_["log.file"] = "game.log";
            config_data_["game.max_players"] = "1000";
            config_data_["game.debug_mode"] = "false";
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "解析配置失败: " << e.what() << std::endl;
        return false;
    }
}

std::string ConfigService::GetString(const std::string& key, const std::string& default_value) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    auto it = config_data_.find(key);
    if (it != config_data_.end()) {
        return it->second;
    }
    
    // 尝试嵌套键查找
    std::string nested_value = GetNestedValue(key);
    if (!nested_value.empty()) {
        return nested_value;
    }
    
    return default_value;
}

int ConfigService::GetInt(const std::string& key, int default_value) {
    std::string value = GetString(key);
    if (value.empty()) {
        return default_value;
    }
    
    try {
        return std::stoi(value);
    } catch (const std::exception&) {
        return default_value;
    }
}

bool ConfigService::GetBool(const std::string& key, bool default_value) {
    std::string value = GetString(key);
    if (value.empty()) {
        return default_value;
    }
    
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    return value == "true" || value == "1" || value == "yes" || value == "on";
}

double ConfigService::GetDouble(const std::string& key, double default_value) {
    std::string value = GetString(key);
    if (value.empty()) {
        return default_value;
    }
    
    try {
        return std::stod(value);
    } catch (const std::exception&) {
        return default_value;
    }
}

void ConfigService::SetString(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    config_data_[key] = value;
}

bool ConfigService::HasKey(const std::string& key) const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    return config_data_.find(key) != config_data_.end();
}

bool ConfigService::Reload() {
    if (config_file_path_.empty()) {
        return false;
    }
    return LoadConfig(config_file_path_);
}

std::vector<std::string> ConfigService::GetAllKeys() const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    std::vector<std::string> keys;
    keys.reserve(config_data_.size());
    
    for (const auto& pair : config_data_) {
        keys.push_back(pair.first);
    }
    
    return keys;
}

std::string ConfigService::GetNestedValue(const std::string& nested_key) const {
    // 简化的嵌套键处理
    // 实际实现中应该支持更复杂的嵌套结构
    return "";
}

} // namespace strategy