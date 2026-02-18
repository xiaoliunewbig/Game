/*
 * 文件名: BaseRepository.cpp
 * 说明: 数据库仓储基类实现
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#include "database/BaseRepository.h"
#include <stdexcept>

namespace strategy {

BaseRepository::BaseRepository(std::unique_ptr<IDatabaseConnection> connection)
    : connection_(std::move(connection)) {
    if (connection_ && !connection_->IsConnected()) {
        connection_->Connect();
    }
}

QueryResult BaseRepository::ExecuteQuery(const std::string& query, const std::vector<std::any>& params) {
    if (!connection_) {
        throw std::runtime_error("数据库连接为空");
    }
    return connection_->ExecuteQuery(query, params);
}

int BaseRepository::ExecuteUpdate(const std::string& query, const std::vector<std::any>& params) {
    if (!connection_) {
        throw std::runtime_error("数据库连接为空");
    }
    return connection_->ExecuteUpdate(query, params);
}

bool BaseRepository::BeginTransaction() {
    if (!connection_) {
        return false;
    }
    return connection_->BeginTransaction();
}

bool BaseRepository::CommitTransaction() {
    if (!connection_) {
        return false;
    }
    return connection_->CommitTransaction();
}

bool BaseRepository::RollbackTransaction() {
    if (!connection_) {
        return false;
    }
    return connection_->RollbackTransaction();
}

long long BaseRepository::GetLastInsertId() {
    if (!connection_) {
        return -1;
    }
    return connection_->GetLastInsertId();
}

} // namespace strategy
