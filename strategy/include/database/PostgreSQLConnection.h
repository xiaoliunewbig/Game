/*
 * 文件名: PostgreSQLConnection.h
 * 说明: PostgreSQL数据库连接实现
 * 作者: 彭承康
 * 创建时间: 2025-01-20
 */
#ifndef STRATEGY_DATABASE_POSTGRESQLCONNECTION_H
#define STRATEGY_DATABASE_POSTGRESQLCONNECTION_H

#include "database/IDatabaseConnection.h"
#include "database/DatabaseConfig.h"
#include <pqxx/pqxx>
#include <memory>

namespace strategy {

class PostgreSQLConnection : public IDatabaseConnection {
public:
    explicit PostgreSQLConnection(const DatabaseConfig& config);
    virtual ~PostgreSQLConnection();
    
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
    std::unique_ptr<pqxx::connection> connection_;
    std::unique_ptr<pqxx::work> transaction_;
    
    std::string ConvertParamsToQuery(const std::string& query, const std::vector<std::any>& params);
    std::any ConvertPqxxField(const pqxx::field& field);
};

} // namespace strategy

#endif // STRATEGY_DATABASE_POSTGRESQLCONNECTION_H