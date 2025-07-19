/*
 * 文件名: DatabaseConfig.cpp
 * 说明: 数据库配置实现
 * 作者: 彭承康
 * 创建时间: 2025-01-20
 */
#include "database/DatabaseConfig.h"
#include <sstream>

namespace strategy {

std::string DatabaseConfig::GetConnectionString() const {
    std::stringstream ss;
    
    switch (type) {
        case DatabaseType::POSTGRESQL:
            ss << "host=" << host 
               << " port=" << port 
               << " dbname=" << database 
               << " user=" << username 
               << " password=" << password;
            break;
            
        case DatabaseType::MYSQL:
            ss << "mysql://";
            if (!username.empty()) {
                ss << username;
                if (!password.empty()) {
                    ss << ":" << password;
                }
                ss << "@";
            }
            ss << host << ":" << port << "/" << database;
            break;
            
        case DatabaseType::SQLITE:
            ss << database; // SQLite只需要文件路径
            break;
            
        case DatabaseType::MONGODB:
            ss << "mongodb://";
            if (!username.empty()) {
                ss << username;
                if (!password.empty()) {
                    ss << ":" << password;
                }
                ss << "@";
            }
            ss << host << ":" << port << "/" << database;
            break;
    }
    
    // 添加额外参数
    for (const auto& param : extra_params) {
        ss << " " << param.first << "=" << param.second;
    }
    
    return ss.str();
}

} // namespace strategy