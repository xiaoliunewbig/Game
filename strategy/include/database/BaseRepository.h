/*
 * 文件名: BaseRepository.h
 * 说明: 数据库仓储基类
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#ifndef STRATEGY_DATABASE_BASEREPOSITORY_H
#define STRATEGY_DATABASE_BASEREPOSITORY_H

#include "database/IDatabaseConnection.h"
#include <memory>
#include <string>

namespace strategy {

class BaseRepository {
public:
    explicit BaseRepository(std::unique_ptr<IDatabaseConnection> connection);
    virtual ~BaseRepository() = default;

protected:
    std::unique_ptr<IDatabaseConnection> connection_;
    
    /**
     * @brief 执行查询并返回结果
     */
    QueryResult ExecuteQuery(const std::string& query, const std::vector<std::any>& params = {});
    
    /**
     * @brief 执行更新操作
     */
    int ExecuteUpdate(const std::string& query, const std::vector<std::any>& params = {});
    
    /**
     * @brief 开始事务
     */
    bool BeginTransaction();
    
    /**
     * @brief 提交事务
     */
    bool CommitTransaction();
    
    /**
     * @brief 回滚事务
     */
    bool RollbackTransaction();
    
    /**
     * @brief 获取最后插入的ID
     */
    long long GetLastInsertId();
};

} // namespace strategy

#endif // STRATEGY_DATABASE_BASEREPOSITORY_H