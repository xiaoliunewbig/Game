/*
 * 文件名: IDatabaseConnection.h
 * 说明: 通用数据库连接接口
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#ifndef STRATEGY_DATABASE_IDATABASECONNECTION_H
#define STRATEGY_DATABASE_IDATABASECONNECTION_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <any>

namespace strategy {

using QueryResult = std::vector<std::map<std::string, std::any>>;

class IDatabaseConnection {
public:
    virtual ~IDatabaseConnection() = default;
    
    /**
     * @brief 连接数据库
     */
    virtual bool Connect() = 0;
    
    /**
     * @brief 断开连接
     */
    virtual void Disconnect() = 0;
    
    /**
     * @brief 检查连接状态
     */
    virtual bool IsConnected() const = 0;
    
    /**
     * @brief 执行查询
     */
    virtual QueryResult ExecuteQuery(const std::string& query, const std::vector<std::any>& params = {}) = 0;
    
    /**
     * @brief 执行更新/插入/删除
     */
    virtual int ExecuteUpdate(const std::string& query, const std::vector<std::any>& params = {}) = 0;
    
    /**
     * @brief 开始事务
     */
    virtual bool BeginTransaction() = 0;
    
    /**
     * @brief 提交事务
     */
    virtual bool CommitTransaction() = 0;
    
    /**
     * @brief 回滚事务
     */
    virtual bool RollbackTransaction() = 0;
    
    /**
     * @brief 获取最后插入的ID
     */
    virtual long long GetLastInsertId() = 0;
};

} // namespace strategy

#endif // STRATEGY_DATABASE_IDATABASECONNECTION_H