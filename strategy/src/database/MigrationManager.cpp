/*
 * 文件名: MigrationManager.cpp
 * 说明: 数据库迁移管理器实现
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#include "database/MigrationManager.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>

namespace strategy {

MigrationManager::MigrationManager(std::shared_ptr<IDatabaseConnection> connection,
                                   DatabaseType db_type)
    : connection_(std::move(connection))
    , db_type_(db_type)
{
    if (!connection_ || !connection_->IsConnected()) {
        throw std::invalid_argument("MigrationManager: 连接为空或未连接");
    }
    LoadMigrations();
}

void MigrationManager::LoadMigrations() {
    switch (db_type_) {
        case DatabaseType::SQLITE:
            LoadSQLiteMigrations();
            break;
        case DatabaseType::POSTGRESQL:
            LoadPostgreSQLMigrations();
            break;
        default:
            LoadPostgreSQLMigrations();
            break;
    }

    std::sort(migrations_.begin(), migrations_.end(),
              [](const Migration& a, const Migration& b) {
                  return a.version < b.version;
              });
}

void MigrationManager::LoadSQLiteMigrations() {
    migrations_.push_back({
        1,
        "创建玩家表",
        "CREATE TABLE IF NOT EXISTS players ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  username TEXT UNIQUE NOT NULL,"
        "  password_hash TEXT NOT NULL,"
        "  email TEXT UNIQUE NOT NULL,"
        "  level INTEGER DEFAULT 1,"
        "  experience INTEGER DEFAULT 0,"
        "  created_at TEXT DEFAULT CURRENT_TIMESTAMP,"
        "  updated_at TEXT DEFAULT CURRENT_TIMESTAMP"
        ");",
        "DROP TABLE IF EXISTS players;"
    });

    migrations_.push_back({
        2,
        "创建存档表",
        "CREATE TABLE IF NOT EXISTS game_saves ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  player_id INTEGER REFERENCES players(id) ON DELETE CASCADE,"
        "  save_name TEXT NOT NULL,"
        "  save_data TEXT NOT NULL,"
        "  created_at TEXT DEFAULT CURRENT_TIMESTAMP"
        ");",
        "DROP TABLE IF EXISTS game_saves;"
    });
}

void MigrationManager::LoadPostgreSQLMigrations() {
    migrations_.push_back({
        1,
        "创建玩家表",
        "CREATE TABLE IF NOT EXISTS players ("
        "  id BIGSERIAL PRIMARY KEY,"
        "  username VARCHAR(50) UNIQUE NOT NULL,"
        "  password_hash VARCHAR(128) NOT NULL,"
        "  email VARCHAR(100) UNIQUE NOT NULL,"
        "  level INT DEFAULT 1,"
        "  experience BIGINT DEFAULT 0,"
        "  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "  updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ");",
        "DROP TABLE IF EXISTS players;"
    });

    migrations_.push_back({
        2,
        "创建存档表",
        "CREATE TABLE IF NOT EXISTS game_saves ("
        "  id BIGSERIAL PRIMARY KEY,"
        "  player_id BIGINT REFERENCES players(id) ON DELETE CASCADE,"
        "  save_name VARCHAR(100) NOT NULL,"
        "  save_data TEXT NOT NULL,"
        "  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ");",
        "DROP TABLE IF EXISTS game_saves;"
    });
}

bool MigrationManager::CreateMigrationTable() {
    try {
        std::string sql;
        if (db_type_ == DatabaseType::SQLITE) {
            sql = "CREATE TABLE IF NOT EXISTS schema_migrations ("
                  "  version INTEGER PRIMARY KEY,"
                  "  description TEXT NOT NULL,"
                  "  executed_at TEXT DEFAULT CURRENT_TIMESTAMP"
                  ");";
        } else {
            sql = "CREATE TABLE IF NOT EXISTS schema_migrations ("
                  "  version INT PRIMARY KEY,"
                  "  description VARCHAR(255) NOT NULL,"
                  "  executed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
                  ");";
        }
        connection_->ExecuteUpdate(sql);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "MigrationManager: 创建迁移表失败: " << e.what() << std::endl;
        return false;
    }
}

int MigrationManager::GetCurrentVersion() {
    try {
        auto result = connection_->ExecuteQuery(
            "SELECT COALESCE(MAX(version), 0) AS current_version FROM schema_migrations;"
        );
        if (!result.empty()) {
            auto it = result[0].find("current_version");
            if (it != result[0].end()) {
                return static_cast<int>(std::any_cast<long long>(it->second));
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "MigrationManager: 获取版本失败: " << e.what() << std::endl;
    }
    return 0;
}

bool MigrationManager::RunMigrations() {
    if (!CreateMigrationTable()) {
        return false;
    }

    int current_version = GetCurrentVersion();
    std::cout << "MigrationManager: 当前数据库版本: " << current_version << std::endl;

    for (const auto& migration : migrations_) {
        if (migration.version <= current_version) {
            continue;
        }

        std::cout << "MigrationManager: 执行迁移 v" << migration.version
                  << ": " << migration.description << std::endl;

        try {
            connection_->BeginTransaction();

            connection_->ExecuteUpdate(migration.up_sql);

            connection_->ExecuteUpdate(
                "INSERT INTO schema_migrations (version, description) VALUES ($1, $2);",
                {std::any(migration.version), std::any(migration.description)}
            );

            connection_->CommitTransaction();
            std::cout << "MigrationManager: 迁移 v" << migration.version << " 成功" << std::endl;

        } catch (const std::exception& e) {
            std::cerr << "MigrationManager: 迁移 v" << migration.version
                      << " 失败: " << e.what() << std::endl;
            connection_->RollbackTransaction();
            return false;
        }
    }

    std::cout << "MigrationManager: 所有迁移完成" << std::endl;
    return true;
}

bool MigrationManager::RollbackMigration(int target_version) {
    int current_version = GetCurrentVersion();

    if (target_version >= current_version) {
        std::cout << "MigrationManager: 目标版本 >= 当前版本，无需回滚" << std::endl;
        return true;
    }

    // Iterate migrations in reverse order
    for (auto it = migrations_.rbegin(); it != migrations_.rend(); ++it) {
        if (it->version <= target_version) {
            break;
        }
        if (it->version > current_version) {
            continue;
        }

        std::cout << "MigrationManager: 回滚迁移 v" << it->version
                  << ": " << it->description << std::endl;

        try {
            connection_->BeginTransaction();

            connection_->ExecuteUpdate(it->down_sql);

            connection_->ExecuteUpdate(
                "DELETE FROM schema_migrations WHERE version = $1;",
                {std::any(it->version)}
            );

            connection_->CommitTransaction();
            std::cout << "MigrationManager: 回滚 v" << it->version << " 成功" << std::endl;

        } catch (const std::exception& e) {
            std::cerr << "MigrationManager: 回滚 v" << it->version
                      << " 失败: " << e.what() << std::endl;
            connection_->RollbackTransaction();
            return false;
        }
    }

    return true;
}

} // namespace strategy
