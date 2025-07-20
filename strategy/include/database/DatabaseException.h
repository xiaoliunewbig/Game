/*
 * 文件名: DatabaseException.h
 * 说明: 数据库操作异常类定义，提供统一的错误处理机制
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 */
#ifndef STRATEGY_DATABASE_DATABASEEXCEPTION_H
#define STRATEGY_DATABASE_DATABASEEXCEPTION_H

#include <stdexcept>
#include <string>
#include <memory>

namespace strategy {

/**
 * @brief 数据库操作基础异常类
 */
class DatabaseException : public std::runtime_error {
public:
    explicit DatabaseException(const std::string& message);
    DatabaseException(const std::string& message, int error_code);
    DatabaseException(const std::string& message, int error_code, 
                     const std::exception& cause);
    
    virtual ~DatabaseException() noexcept = default;
    
    int GetErrorCode() const noexcept { return error_code_; }
    virtual const char* GetExceptionType() const noexcept { return "DatabaseException"; }
    const std::exception* GetCause() const noexcept { return cause_.get(); }
    
private:
    int error_code_;
    std::unique_ptr<std::exception> cause_;
};

/**
 * @brief 数据库连接异常
 */
class ConnectionException : public DatabaseException {
public:
    explicit ConnectionException(const std::string& message);
    ConnectionException(const std::string& message, int error_code);
    
    ~ConnectionException() noexcept override = default;
    
    const char* GetExceptionType() const noexcept override { 
        return "ConnectionException"; 
    }
};

/**
 * @brief SQL查询执行异常
 */
class QueryException : public DatabaseException {
public:
    QueryException(const std::string& message, const std::string& query);
    QueryException(const std::string& message, const std::string& query, int error_code);
    
    ~QueryException() noexcept override = default;
    
    const std::string& GetQuery() const noexcept { return query_; }
    const char* GetExceptionType() const noexcept override { 
        return "QueryException"; 
    }
    
private:
    std::string query_;
};

/**
 * @brief 数据库事务异常
 */
class TransactionException : public DatabaseException {
public:
    enum class OperationType {
        BEGIN,
        COMMIT,
        ROLLBACK,
        SAVEPOINT
    };
    
    TransactionException(const std::string& message, OperationType operation);
    TransactionException(const std::string& message, OperationType operation, int error_code);
    
    ~TransactionException() noexcept override = default;
    
    OperationType GetOperationType() const noexcept { return operation_; }
    const char* GetOperationName() const noexcept;
    const char* GetExceptionType() const noexcept override { 
        return "TransactionException"; 
    }
    
private:
    OperationType operation_;
};

/**
 * @brief 数据库配置异常
 */
class ConfigurationException : public DatabaseException {
public:
    explicit ConfigurationException(const std::string& message);
    
    ~ConfigurationException() noexcept override = default;
    
    const char* GetExceptionType() const noexcept override { 
        return "ConfigurationException"; 
    }
};

} // namespace strategy
#endif // STRATEGY_DATABASE_DATABASEEXCEPTION_H
