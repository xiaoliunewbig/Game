/*
 * File: PostgreSQLConnection.h
 * Description: PostgreSQL database connection implementation.
 */
#ifndef STRATEGY_DATABASE_POSTGRESQLCONNECTION_H
#define STRATEGY_DATABASE_POSTGRESQLCONNECTION_H

#ifdef HAS_PQXX

#include "database/DatabaseConfig.h"
#include "database/IDatabaseConnection.h"

#include <memory>
#include <pqxx/pqxx>

namespace strategy {

class PostgreSQLConnection : public IDatabaseConnection {
public:
    explicit PostgreSQLConnection(const DatabaseConfig& config);
    ~PostgreSQLConnection() override;

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

    std::any ConvertPqxxField(const pqxx::field& field);
};

} // namespace strategy

#endif // HAS_PQXX

#endif // STRATEGY_DATABASE_POSTGRESQLCONNECTION_H
