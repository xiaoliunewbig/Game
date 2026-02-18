/*
 * 文件名: SQLiteConnection.cpp
 * 说明: SQLite数据库连接实现
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */

#ifdef HAS_SQLITE

#include "database/SQLiteConnection.h"
#include <iostream>
#include <cctype>
#include <stdexcept>

namespace strategy {

SQLiteConnection::SQLiteConnection(const DatabaseConfig& config)
    : config_(config)
    , db_(nullptr)
    , in_transaction_(false)
{
}

SQLiteConnection::~SQLiteConnection() {
    Disconnect();
}

bool SQLiteConnection::Connect() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (db_) {
        return true;
    }

    std::string db_path = config_.GetConnectionString();
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;

    int rc = sqlite3_open_v2(db_path.c_str(), &db_, flags, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQLite连接失败: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }

    ConfigurePragmas();
    std::cout << "SQLite连接成功: " << config_.database << std::endl;
    return true;
}

void SQLiteConnection::Disconnect() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (in_transaction_) {
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        in_transaction_ = false;
    }

    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool SQLiteConnection::IsConnected() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return db_ != nullptr;
}

void SQLiteConnection::ConfigurePragmas() {
    if (!db_) {
        return;
    }

    // WAL模式: 允许并发读写
    sqlite3_exec(db_, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
    // 启用外键约束
    sqlite3_exec(db_, "PRAGMA foreign_keys=ON;", nullptr, nullptr, nullptr);
    // 设置busy_timeout (毫秒)
    int timeout_ms = config_.connection_timeout * 1000;
    std::string pragma = "PRAGMA busy_timeout=" + std::to_string(timeout_ms) + ";";
    sqlite3_exec(db_, pragma.c_str(), nullptr, nullptr, nullptr);
}

std::string SQLiteConnection::ConvertParamPlaceholders(const std::string& query) const {
    std::string result;
    result.reserve(query.size());

    for (size_t i = 0; i < query.size(); ++i) {
        if (query[i] == '$' && i + 1 < query.size() && std::isdigit(static_cast<unsigned char>(query[i + 1]))) {
            result += '?';
            ++i; // skip first digit
            while (i + 1 < query.size() && std::isdigit(static_cast<unsigned char>(query[i + 1]))) {
                ++i; // skip remaining digits
            }
        } else {
            result += query[i];
        }
    }

    return result;
}

bool SQLiteConnection::BindParameters(sqlite3_stmt* stmt, const std::vector<std::any>& params) {
    for (size_t i = 0; i < params.size(); ++i) {
        int idx = static_cast<int>(i) + 1; // SQLite参数索引从1开始

        if (!params[i].has_value()) {
            sqlite3_bind_null(stmt, idx);
        } else if (params[i].type() == typeid(int)) {
            sqlite3_bind_int(stmt, idx, std::any_cast<int>(params[i]));
        } else if (params[i].type() == typeid(long long)) {
            sqlite3_bind_int64(stmt, idx, std::any_cast<long long>(params[i]));
        } else if (params[i].type() == typeid(std::string)) {
            const auto& str = std::any_cast<const std::string&>(params[i]);
            sqlite3_bind_text(stmt, idx, str.c_str(), static_cast<int>(str.size()), SQLITE_TRANSIENT);
        } else if (params[i].type() == typeid(double)) {
            sqlite3_bind_double(stmt, idx, std::any_cast<double>(params[i]));
        } else if (params[i].type() == typeid(bool)) {
            sqlite3_bind_int(stmt, idx, std::any_cast<bool>(params[i]) ? 1 : 0);
        } else {
            sqlite3_bind_null(stmt, idx);
        }
    }

    return true;
}

QueryResult SQLiteConnection::ExecuteQuery(const std::string& query, const std::vector<std::any>& params) {
    std::lock_guard<std::mutex> lock(mutex_);
    QueryResult result;

    if (!db_) {
        throw std::runtime_error("SQLite: 数据库未连接");
    }

    std::string converted_query = ConvertParamPlaceholders(query);

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, converted_query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string error = sqlite3_errmsg(db_);
        throw std::runtime_error("SQLite查询预处理失败: " + error);
    }

    if (!BindParameters(stmt, params)) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("SQLite: 参数绑定失败");
    }

    int col_count = sqlite3_column_count(stmt);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        std::map<std::string, std::any> row;
        for (int i = 0; i < col_count; ++i) {
            std::string col_name = sqlite3_column_name(stmt, i);
            int col_type = sqlite3_column_type(stmt, i);

            switch (col_type) {
                case SQLITE_INTEGER:
                    row[col_name] = static_cast<long long>(sqlite3_column_int64(stmt, i));
                    break;
                case SQLITE_FLOAT:
                    row[col_name] = sqlite3_column_double(stmt, i);
                    break;
                case SQLITE_TEXT: {
                    const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                    row[col_name] = std::string(text ? text : "");
                    break;
                }
                case SQLITE_NULL:
                default:
                    row[col_name] = std::any{};
                    break;
            }
        }
        result.push_back(std::move(row));
    }

    if (rc != SQLITE_DONE) {
        std::string error = sqlite3_errmsg(db_);
        sqlite3_finalize(stmt);
        throw std::runtime_error("SQLite查询执行失败: " + error);
    }

    sqlite3_finalize(stmt);
    return result;
}

int SQLiteConnection::ExecuteUpdate(const std::string& query, const std::vector<std::any>& params) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!db_) {
        throw std::runtime_error("SQLite: 数据库未连接");
    }

    std::string converted_query = ConvertParamPlaceholders(query);

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, converted_query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string error = sqlite3_errmsg(db_);
        throw std::runtime_error("SQLite更新预处理失败: " + error);
    }

    if (!BindParameters(stmt, params)) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("SQLite: 参数绑定失败");
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        std::string error = sqlite3_errmsg(db_);
        sqlite3_finalize(stmt);
        throw std::runtime_error("SQLite更新执行失败: " + error);
    }

    int changes = sqlite3_changes(db_);
    sqlite3_finalize(stmt);
    return changes;
}

bool SQLiteConnection::BeginTransaction() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!db_) {
        return false;
    }

    if (in_transaction_) {
        return true;
    }

    int rc = sqlite3_exec(db_, "BEGIN;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQLite开始事务失败: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    in_transaction_ = true;
    return true;
}

bool SQLiteConnection::CommitTransaction() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!in_transaction_) {
        return false;
    }

    int rc = sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQLite提交事务失败: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    in_transaction_ = false;
    return true;
}

bool SQLiteConnection::RollbackTransaction() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!in_transaction_) {
        return false;
    }

    int rc = sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQLite回滚事务失败: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    in_transaction_ = false;
    return true;
}

long long SQLiteConnection::GetLastInsertId() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!db_) {
        return -1;
    }

    return sqlite3_last_insert_rowid(db_);
}

} // namespace strategy

#endif // HAS_SQLITE
