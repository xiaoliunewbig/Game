/*
 * 文件名: MySQLConnection.cpp
 * 说明: MySQL数据库连接实现（条件编译，需要HAS_MYSQL宏）
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */

#ifdef HAS_MYSQL

#include "database/MySQLConnection.h"
#include <iostream>
#include <cstring>

namespace strategy {

MySQLConnection::MySQLConnection(const DatabaseConfig& config)
    : config_(config)
    , mysql_connection_(nullptr)
    , in_transaction_(false)
{
}

MySQLConnection::~MySQLConnection() {
    Disconnect();
}

bool MySQLConnection::InitializeMySQL() {
    mysql_connection_ = mysql_init(nullptr);
    if (!mysql_connection_) {
        std::cerr << "MySQLConnection: mysql_init失败" << std::endl;
        return false;
    }
    return true;
}

void MySQLConnection::SetConnectionOptions() {
    if (!mysql_connection_) {
        return;
    }

    // Set charset to UTF-8
    mysql_options(mysql_connection_, MYSQL_SET_CHARSET_NAME, "utf8mb4");

    // Set connection timeout
    unsigned int timeout = static_cast<unsigned int>(config_.connection_timeout);
    mysql_options(mysql_connection_, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);

    // Enable auto-reconnect
    bool reconnect = true;
    mysql_options(mysql_connection_, MYSQL_OPT_RECONNECT, &reconnect);
}

bool MySQLConnection::Connect() {
    if (!InitializeMySQL()) {
        return false;
    }

    SetConnectionOptions();

    unsigned int port = static_cast<unsigned int>(config_.port);
    MYSQL* result = mysql_real_connect(
        mysql_connection_,
        config_.host.c_str(),
        config_.username.c_str(),
        config_.password.c_str(),
        config_.database.c_str(),
        port,
        nullptr,
        0
    );

    if (!result) {
        HandleMySQLError("Connect");
        return false;
    }

    std::cout << "MySQLConnection: 连接成功: " << config_.database << std::endl;
    return true;
}

void MySQLConnection::Disconnect() {
    if (in_transaction_) {
        RollbackTransaction();
    }

    if (mysql_connection_) {
        mysql_close(mysql_connection_);
        mysql_connection_ = nullptr;
    }
}

bool MySQLConnection::IsConnected() const {
    if (!mysql_connection_) {
        return false;
    }
    return mysql_ping(const_cast<MYSQL*>(mysql_connection_)) == 0;
}

bool MySQLConnection::BindParameters(MYSQL_STMT* stmt, const std::vector<std::any>& params) {
    if (params.empty()) {
        return true;
    }

    std::vector<MYSQL_BIND> binds(params.size());
    std::vector<long long> int_values(params.size());
    std::vector<std::string> str_values(params.size());
    std::vector<unsigned long> str_lengths(params.size());
    std::vector<double> double_values(params.size());

    std::memset(binds.data(), 0, sizeof(MYSQL_BIND) * params.size());

    for (size_t i = 0; i < params.size(); ++i) {
        if (params[i].type() == typeid(int)) {
            int_values[i] = std::any_cast<int>(params[i]);
            binds[i].buffer_type = MYSQL_TYPE_LONGLONG;
            binds[i].buffer = &int_values[i];
        } else if (params[i].type() == typeid(long long)) {
            int_values[i] = std::any_cast<long long>(params[i]);
            binds[i].buffer_type = MYSQL_TYPE_LONGLONG;
            binds[i].buffer = &int_values[i];
        } else if (params[i].type() == typeid(std::string)) {
            str_values[i] = std::any_cast<std::string>(params[i]);
            str_lengths[i] = static_cast<unsigned long>(str_values[i].size());
            binds[i].buffer_type = MYSQL_TYPE_STRING;
            binds[i].buffer = const_cast<char*>(str_values[i].c_str());
            binds[i].buffer_length = str_lengths[i];
            binds[i].length = &str_lengths[i];
        } else if (params[i].type() == typeid(double)) {
            double_values[i] = std::any_cast<double>(params[i]);
            binds[i].buffer_type = MYSQL_TYPE_DOUBLE;
            binds[i].buffer = &double_values[i];
        } else {
            binds[i].buffer_type = MYSQL_TYPE_NULL;
        }
    }

    if (mysql_stmt_bind_param(stmt, binds.data()) != 0) {
        std::cerr << "MySQLConnection: 参数绑定失败: "
                  << mysql_stmt_error(stmt) << std::endl;
        return false;
    }

    return true;
}

QueryResult MySQLConnection::ExecuteQuery(const std::string& query,
                                         const std::vector<std::any>& params) {
    QueryResult result;

    if (!IsConnected()) {
        throw std::runtime_error("MySQLConnection: 数据库未连接");
    }

    MYSQL_STMT* stmt = mysql_stmt_init(mysql_connection_);
    if (!stmt) {
        throw std::runtime_error("MySQLConnection: mysql_stmt_init失败");
    }

    if (mysql_stmt_prepare(stmt, query.c_str(), static_cast<unsigned long>(query.size())) != 0) {
        std::string error = mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        throw std::runtime_error("MySQLConnection: 预处理失败: " + error);
    }

    if (!BindParameters(stmt, params)) {
        mysql_stmt_close(stmt);
        throw std::runtime_error("MySQLConnection: 参数绑定失败");
    }

    if (mysql_stmt_execute(stmt) != 0) {
        std::string error = mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        throw std::runtime_error("MySQLConnection: 执行查询失败: " + error);
    }

    // Fetch result metadata
    MYSQL_RES* meta = mysql_stmt_result_metadata(stmt);
    if (meta) {
        unsigned int num_fields = mysql_num_fields(meta);
        MYSQL_FIELD* fields = mysql_fetch_fields(meta);

        mysql_stmt_store_result(stmt);

        // Prepare output buffers
        std::vector<MYSQL_BIND> output_binds(num_fields);
        std::vector<std::vector<char>> buffers(num_fields, std::vector<char>(1024));
        std::vector<unsigned long> lengths(num_fields);
        std::vector<my_bool> nulls(num_fields);

        std::memset(output_binds.data(), 0, sizeof(MYSQL_BIND) * num_fields);

        for (unsigned int i = 0; i < num_fields; ++i) {
            output_binds[i].buffer_type = MYSQL_TYPE_STRING;
            output_binds[i].buffer = buffers[i].data();
            output_binds[i].buffer_length = 1024;
            output_binds[i].length = &lengths[i];
            output_binds[i].is_null = &nulls[i];
        }

        mysql_stmt_bind_result(stmt, output_binds.data());

        while (mysql_stmt_fetch(stmt) == 0) {
            std::map<std::string, std::any> row;
            for (unsigned int i = 0; i < num_fields; ++i) {
                std::string col_name = fields[i].name;
                if (nulls[i]) {
                    row[col_name] = std::any{};
                } else {
                    std::string value(buffers[i].data(), lengths[i]);
                    row[col_name] = value;
                }
            }
            result.push_back(row);
        }

        mysql_free_result(meta);
    }

    mysql_stmt_close(stmt);
    return result;
}

int MySQLConnection::ExecuteUpdate(const std::string& query,
                                  const std::vector<std::any>& params) {
    if (!IsConnected()) {
        throw std::runtime_error("MySQLConnection: 数据库未连接");
    }

    MYSQL_STMT* stmt = mysql_stmt_init(mysql_connection_);
    if (!stmt) {
        throw std::runtime_error("MySQLConnection: mysql_stmt_init失败");
    }

    if (mysql_stmt_prepare(stmt, query.c_str(), static_cast<unsigned long>(query.size())) != 0) {
        std::string error = mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        throw std::runtime_error("MySQLConnection: 预处理失败: " + error);
    }

    if (!BindParameters(stmt, params)) {
        mysql_stmt_close(stmt);
        throw std::runtime_error("MySQLConnection: 参数绑定失败");
    }

    if (mysql_stmt_execute(stmt) != 0) {
        std::string error = mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        throw std::runtime_error("MySQLConnection: 执行更新失败: " + error);
    }

    int affected = static_cast<int>(mysql_stmt_affected_rows(stmt));
    mysql_stmt_close(stmt);
    return affected;
}

bool MySQLConnection::BeginTransaction() {
    if (!IsConnected()) {
        return false;
    }

    if (mysql_real_query(mysql_connection_, "START TRANSACTION", 17) != 0) {
        HandleMySQLError("BeginTransaction");
        return false;
    }

    in_transaction_ = true;
    return true;
}

bool MySQLConnection::CommitTransaction() {
    if (!in_transaction_) {
        return false;
    }

    if (mysql_real_query(mysql_connection_, "COMMIT", 6) != 0) {
        HandleMySQLError("CommitTransaction");
        return false;
    }

    in_transaction_ = false;
    return true;
}

bool MySQLConnection::RollbackTransaction() {
    if (!in_transaction_) {
        return false;
    }

    if (mysql_real_query(mysql_connection_, "ROLLBACK", 8) != 0) {
        HandleMySQLError("RollbackTransaction");
        return false;
    }

    in_transaction_ = false;
    return true;
}

void MySQLConnection::HandleMySQLError(const std::string& operation) {
    if (mysql_connection_) {
        std::cerr << "MySQLConnection: " << operation << " 失败: "
                  << mysql_error(mysql_connection_) << std::endl;
    }
}

} // namespace strategy

#endif // HAS_MYSQL
