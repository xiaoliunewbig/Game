/*
 * 文件名: PostgreSQLConnection.cpp
 * 说明: PostgreSQL数据库连接实现
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 */
#include "database/PostgreSQLConnection.h"
#include <iostream>
#include <sstream>

namespace strategy {

PostgreSQLConnection::PostgreSQLConnection(const DatabaseConfig& config)
    : config_(config), connection_(nullptr), transaction_(nullptr) {
}

PostgreSQLConnection::~PostgreSQLConnection() {
    Disconnect();
}

bool PostgreSQLConnection::Connect() {
    try {
        std::string conn_str = config_.GetConnectionString();
        connection_ = std::make_unique<pqxx::connection>(conn_str);
        
        if (connection_->is_open()) {
            std::cout << "PostgreSQL连接成功: " << config_.database << std::endl;
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "PostgreSQL连接失败: " << e.what() << std::endl;
        connection_.reset();
    }
    return false;
}

void PostgreSQLConnection::Disconnect() {
    if (transaction_) {
        try {
            transaction_->abort();
        } catch (...) {}
        transaction_.reset();
    }
    
    if (connection_) {
        try {
            connection_->close();
        } catch (...) {}
        connection_.reset();
    }
}

bool PostgreSQLConnection::IsConnected() const {
    return connection_ && connection_->is_open();
}

QueryResult PostgreSQLConnection::ExecuteQuery(const std::string& query, const std::vector<std::any>& params) {
    QueryResult result;
    
    if (!IsConnected()) {
        throw std::runtime_error("数据库未连接");
    }
    
    try {
        pqxx::work txn(*connection_);
        std::string processed_query = ConvertParamsToQuery(query, params);
        pqxx::result r = txn.exec(processed_query);
        txn.commit();
        
        for (const auto& row : r) {
            std::map<std::string, std::any> row_data;
            for (size_t i = 0; i < row.size(); ++i) {
                std::string column_name = r.column_name(i);
                row_data[column_name] = ConvertPqxxField(row[i]);
            }
            result.push_back(row_data);
        }
    } catch (const std::exception& e) {
        std::cerr << "查询执行失败: " << e.what() << std::endl;
        throw;
    }
    
    return result;
}

int PostgreSQLConnection::ExecuteUpdate(const std::string& query, const std::vector<std::any>& params) {
    if (!IsConnected()) {
        throw std::runtime_error("数据库未连接");
    }
    
    try {
        pqxx::work txn(*connection_);
        std::string processed_query = ConvertParamsToQuery(query, params);
        pqxx::result r = txn.exec(processed_query);
        txn.commit();
        
        return static_cast<int>(r.affected_rows());
    } catch (const std::exception& e) {
        std::cerr << "更新执行失败: " << e.what() << std::endl;
        throw;
    }
}

bool PostgreSQLConnection::BeginTransaction() {
    if (!IsConnected()) {
        return false;
    }
    
    try {
        if (transaction_) {
            transaction_->abort();
        }
        transaction_ = std::make_unique<pqxx::work>(*connection_);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "开始事务失败: " << e.what() << std::endl;
        return false;
    }
}

bool PostgreSQLConnection::CommitTransaction() {
    if (!transaction_) {
        return false;
    }
    
    try {
        transaction_->commit();
        transaction_.reset();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "提交事务失败: " << e.what() << std::endl;
        return false;
    }
}

bool PostgreSQLConnection::RollbackTransaction() {
    if (!transaction_) {
        return false;
    }
    
    try {
        transaction_->abort();
        transaction_.reset();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "回滚事务失败: " << e.what() << std::endl;
        return false;
    }
}

long long PostgreSQLConnection::GetLastInsertId() {
    try {
        pqxx::work txn(*connection_);
        pqxx::result r = txn.exec("SELECT lastval()");
        txn.commit();
        
        if (!r.empty()) {
            return r[0][0].as<long long>();
        }
    } catch (const std::exception& e) {
        std::cerr << "获取最后插入ID失败: " << e.what() << std::endl;
    }
    return -1;
}

std::string PostgreSQLConnection::ConvertParamsToQuery(const std::string& query, const std::vector<std::any>& params) {
    std::string result = query;
    
    for (size_t i = 0; i < params.size(); ++i) {
        std::string placeholder = "$" + std::to_string(i + 1);
        std::string value;
        
        try {
            // 尝试转换不同类型的参数
            if (params[i].type() == typeid(int)) {
                value = std::to_string(std::any_cast<int>(params[i]));
            } else if (params[i].type() == typeid(long long)) {
                value = std::to_string(std::any_cast<long long>(params[i]));
            } else if (params[i].type() == typeid(std::string)) {
                value = "'" + std::any_cast<std::string>(params[i]) + "'";
            } else {
                value = "NULL";
            }
        } catch (const std::bad_any_cast& e) {
            value = "NULL";
        }
        
        size_t pos = result.find(placeholder);
        if (pos != std::string::npos) {
            result.replace(pos, placeholder.length(), value);
        }
    }
    
    return result;
}

std::any PostgreSQLConnection::ConvertPqxxField(const pqxx::field& field) {
    if (field.is_null()) {
        return std::any{};
    }
    
    // 根据字段类型转换
    std::string value = field.as<std::string>();
    
    // 这里简化处理，实际应该根据PostgreSQL的类型进行转换
    try {
        // 尝试转换为数字
        if (value.find('.') != std::string::npos) {
            return std::stod(value);
        } else {
            return std::stoll(value);
        }
    } catch (...) {
        // 如果不是数字，返回字符串
        return value;
    }
}

} // namespace strategy