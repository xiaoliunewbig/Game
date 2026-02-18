/*
 * 文件名: SQLiteConnection.h
 * 说明: SQLite数据库连接实现
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#ifndef STRATEGY_DATABASE_SQLITECONNECTION_H
#define STRATEGY_DATABASE_SQLITECONNECTION_H

#include "database/IDatabaseConnection.h"
#include "database/DatabaseConfig.h"
#include <sqlite3.h>
#include <mutex>
#include <string>

namespace strategy {

class SQLiteConnection : public IDatabaseConnection {
public:
    explicit SQLiteConnection(const DatabaseConfig& config);
    virtual ~SQLiteConnection();

    bool Connect() override;
    void Disconnect() override;
    bool IsConnected() const override;

    QueryResult ExecuteQuery(const std::string& query, const std::vector<std::any>& params = {}) override;
    int ExecuteUpdate(const std::string& query, const std::vector<std::any>& params = {}) override;

    bool BeginTransaction() override;
    bool CommitTransaction() override;
    bool RollbackTransaction() override;

    long long GetLastInsertId() override;

private:
    DatabaseConfig config_;
    sqlite3* db_;
    bool in_transaction_;
    mutable std::mutex mutex_;

    /**
     * @brief 将PostgreSQL风格的参数占位符($1, $2, ...)转换为SQLite风格(?)
     */
    std::string ConvertParamPlaceholders(const std::string& query) const;

    /**
     * @brief 绑定参数到预编译语句
     */
    bool BindParameters(sqlite3_stmt* stmt, const std::vector<std::any>& params);

    /**
     * @brief 配置SQLite编译指示（WAL模式、外键、busy_timeout等）
     */
    void ConfigurePragmas();
};

} // namespace strategy

#endif // STRATEGY_DATABASE_SQLITECONNECTION_H
