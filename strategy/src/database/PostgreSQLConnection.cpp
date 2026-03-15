/*
 * 文件名: PostgreSQLConnection.cpp
 * 说明: PostgreSQL数据库连接实现
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */

#ifdef HAS_PQXX

#include "database/PostgreSQLConnection.h"
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

std::string ParamToSqlLiteral(pqxx::transaction_base& txn, const std::any& param) {
    if (!param.has_value()) {
        return "NULL";
    }

    if (param.type() == typeid(int)) {
        return txn.quote(std::any_cast<int>(param));
    }
    if (param.type() == typeid(long long)) {
        return txn.quote(std::any_cast<long long>(param));
    }
    if (param.type() == typeid(double)) {
        return txn.quote(std::any_cast<double>(param));
    }
    if (param.type() == typeid(bool)) {
        return std::any_cast<bool>(param) ? "TRUE" : "FALSE";
    }
    if (param.type() == typeid(std::string)) {
        return txn.quote(std::any_cast<std::string>(param));
    }
    if (param.type() == typeid(const char*)) {
        return txn.quote(std::string(std::any_cast<const char*>(param)));
    }

    throw std::invalid_argument("Unsupported PostgreSQL parameter type");
}

std::string BuildSqlWithParams(
    pqxx::transaction_base& txn,
    const std::string& query,
    const std::vector<std::any>& params) {
    if (params.empty()) {
        return query;
    }

    std::string sql = query;

    if (sql.find('$') != std::string::npos) {
        // Replace $n placeholders from high index to low index.
        for (std::size_t i = params.size(); i > 0; --i) {
            const std::string placeholder = "$" + std::to_string(i);
            const std::string literal = ParamToSqlLiteral(txn, params[i - 1]);
            std::size_t pos = 0;
            while ((pos = sql.find(placeholder, pos)) != std::string::npos) {
                sql.replace(pos, placeholder.size(), literal);
                pos += literal.size();
            }
        }
        return sql;
    }

    if (sql.find('?') != std::string::npos) {
        // Replace ? placeholders in order.
        std::size_t pos = 0;
        std::size_t index = 0;
        while ((pos = sql.find('?', pos)) != std::string::npos && index < params.size()) {
            const std::string literal = ParamToSqlLiteral(txn, params[index]);
            sql.replace(pos, 1, literal);
            pos += literal.size();
            ++index;
        }
        return sql;
    }

    return sql;
}

} // namespace

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
        const std::string sql = BuildSqlWithParams(txn, query, params);
        pqxx::result r = txn.exec(sql);
        txn.commit();

        for (const auto& row : r) {
            std::map<std::string, std::any> row_data;
            for (std::size_t i = 0; i < row.size(); ++i) {
                std::string column_name = r.column_name(i);
                row_data[column_name] = ConvertPqxxField(row[static_cast<pqxx::row::size_type>(i)]);
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
        const std::string sql = BuildSqlWithParams(txn, query, params);
        pqxx::result r = txn.exec(sql);
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
