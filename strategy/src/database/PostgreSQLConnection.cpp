/*
 * 文件名: PostgreSQLConnection.cpp
 * 说明: PostgreSQL数据库连接实现
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */

#ifdef HAS_PQXX

#include "database/PostgreSQLConnection.h"
#include <iostream>

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
        pqxx::result r;

        if (params.empty()) {
            r = txn.exec(query);
        } else {
            pqxx::params p;
            for (const auto& param : params) {
                if (param.type() == typeid(int)) {
                    p.append(std::any_cast<int>(param));
                } else if (param.type() == typeid(long long)) {
                    p.append(std::any_cast<long long>(param));
                } else if (param.type() == typeid(std::string)) {
                    p.append(std::any_cast<std::string>(param));
                } else if (param.type() == typeid(double)) {
                    p.append(std::any_cast<double>(param));
                } else if (param.type() == typeid(bool)) {
                    p.append(std::any_cast<bool>(param));
                } else {
                    p.append();
                }
            }
            r = txn.exec_params(query, p);
        }
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
        pqxx::result r;

        if (params.empty()) {
            r = txn.exec(query);
        } else {
            pqxx::params p;
            for (const auto& param : params) {
                if (param.type() == typeid(int)) {
                    p.append(std::any_cast<int>(param));
                } else if (param.type() == typeid(long long)) {
                    p.append(std::any_cast<long long>(param));
                } else if (param.type() == typeid(std::string)) {
                    p.append(std::any_cast<std::string>(param));
                } else if (param.type() == typeid(double)) {
                    p.append(std::any_cast<double>(param));
                } else if (param.type() == typeid(bool)) {
                    p.append(std::any_cast<bool>(param));
                } else {
                    p.append();
                }
            }
            r = txn.exec_params(query, p);
        }
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

std::any PostgreSQLConnection::ConvertPqxxField(const pqxx::field& field) {
    if (field.is_null()) {
        return std::any{};
    }

    std::string value = field.as<std::string>();

    try {
        if (value.find('.') != std::string::npos) {
            return std::stod(value);
        } else {
            return std::stoll(value);
        }
    } catch (...) {
        return value;
    }
}

} // namespace strategy

#endif // HAS_PQXX
