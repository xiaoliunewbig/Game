/*
 * 文件名: PlayerRepositoryPostgres.cpp
 * 说明: 玩家数据仓库的PostgreSQL实现。
 * 作者: 彭承康
 * 创建时间: 2025-07-13
 */
#include "player_service/PlayerRepositoryPostgres.h"
#include <stdexcept>
#include <iostream>
#include <sstream>      // 用于字符串流处理
#include <iomanip>      // 用于时间解析

namespace strategy {

PlayerRepositoryPostgres::PlayerRepositoryPostgres(const std::string& connection_string) {
    try {
        connection_ = std::make_unique<pqxx::connection>(connection_string);
        if (!connection_->is_open()) {
            throw std::runtime_error("无法连接到数据库。");
        }
        std::cout << "成功连接到数据库: " << connection_->dbname() << std::endl;
    } catch (const std::exception& e) {
        throw std::runtime_error("数据库连接失败: " + std::string(e.what()));
    }
}

std::optional<Player> PlayerRepositoryPostgres::CreatePlayer(const std::string& username, const std::string& password_hash, const std::string& email) {
    try {
        pqxx::work txn(*connection_);
        std::string sql = "INSERT INTO players (username, password_hash, email) VALUES ($1, $2, $3) RETURNING id, username, password_hash, email, created_at;";
        
        // 使用预备语句防止SQL注入
        connection_->prepare("create_player", sql);
        
        pqxx::result r = txn.exec_prepared("create_player", username, password_hash, email);
        txn.commit();

        if (r.empty()) {
            return std::nullopt;
        }
        return RowToPlayer(r[0]);
    } catch (const std::exception& e) {
        std::cerr << "创建玩家失败: " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::optional<Player> PlayerRepositoryPostgres::FindPlayerByUsername(const std::string& username) {
    try {
        pqxx::nontransaction ntxn(*connection_);
        std::string sql = "SELECT id, username, password_hash, email, created_at FROM players WHERE username = $1;";
        
        connection_->prepare("find_player_by_username", sql);
        
        pqxx::result r = ntxn.exec_prepared("find_player_by_username", username);

        if (r.empty()) {
            return std::nullopt;
        }
        return RowToPlayer(r[0]);
    } catch (const std::exception& e) {
        std::cerr << "按用户名查找玩家失败: " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::optional<Player> PlayerRepositoryPostgres::FindPlayerById(long long id) {
    try {
        pqxx::nontransaction ntxn(*connection_);
        std::string sql = "SELECT id, username, password_hash, email, created_at FROM players WHERE id = $1;";
        
        connection_->prepare("find_player_by_id", sql);
        
        pqxx::result r = ntxn.exec_prepared("find_player_by_id", id);

        if (r.empty()) {
            return std::nullopt;
        }
        return RowToPlayer(r[0]);
    } catch (const std::exception& e) {
        std::cerr << "按ID查找玩家失败: " << e.what() << std::endl;
        return std::nullopt;
    }
}

bool PlayerRepositoryPostgres::UpdatePlayer(const Player& player) {
    try {
        pqxx::work txn(*connection_);
        // 注意：通常不应允许更新用户名，这里我们只更新密码和邮箱
        std::string sql = "UPDATE players SET password_hash = $1, email = $2 WHERE id = $3;";
        
        connection_->prepare("update_player", sql);
        
        pqxx::result r = txn.exec_prepared("update_player", player.password_hash, player.email, player.id);
        txn.commit();

        // r.affected_rows() 会返回受影响的行数。如果为1，说明更新成功。
        return r.affected_rows() == 1;
    } catch (const std::exception& e) {
        std::cerr << "更新玩家失败: " << e.what() << std::endl;
        return false;
    }
}

bool PlayerRepositoryPostgres::DeletePlayer(long long id) {
    try {
        pqxx::work txn(*connection_);
        std::string sql = "DELETE FROM players WHERE id = $1;";
        
        connection_->prepare("delete_player", sql);
        
        pqxx::result r = txn.exec_prepared("delete_player", id);
        txn.commit();

        return r.affected_rows() == 1;
    } catch (const std::exception& e) {
        std::cerr << "删除玩家失败: " << e.what() << std::endl;
        return false;
    }
}

Player PlayerRepositoryPostgres::RowToPlayer(const pqxx::row& row) {
    Player player;
    player.id = row["id"].as<long long>();
    player.username = row["username"].as<std::string>();
    player.password_hash = row["password_hash"].as<std::string>();
    player.email = row["email"].as<std::string>();
    
    // 解析时间戳
    std::string created_at_str = row["created_at"].as<std::string>();
    std::tm tm = {};
    std::istringstream ss(created_at_str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    player.created_at = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    
    return player;
}

} // namespace strategy 
