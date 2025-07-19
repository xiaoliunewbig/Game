/*
 * 文件名: PlayerRepositoryDatabase.cpp
 * 说明: 基于通用数据库接口的玩家仓储实现
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 */
#include "player_service/PlayerRepositoryDatabase.h"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace strategy {

PlayerRepositoryDatabase::PlayerRepositoryDatabase(std::unique_ptr<IDatabaseConnection> connection)
    : BaseRepository(std::move(connection)) {
}

std::optional<Player> PlayerRepositoryDatabase::CreatePlayer(const std::string& username, const std::string& password_hash, const std::string& email) {
    try {
        std::string sql = "INSERT INTO players (username, password_hash, email, created_at) VALUES ($1, $2, $3, NOW()) RETURNING id, username, password_hash, email, created_at;";
        
        std::vector<std::any> params = {username, password_hash, email};
        auto result = ExecuteQuery(sql, params);
        
        if (!result.empty()) {
            return ConvertToPlayer(result[0]);
        }
    } catch (const std::exception& e) {
        std::cerr << "创建玩家失败: " << e.what() << std::endl;
    }
    return std::nullopt;
}

std::optional<Player> PlayerRepositoryDatabase::FindPlayerById(long long id) {
    try {
        std::string sql = "SELECT id, username, password_hash, email, created_at FROM players WHERE id = $1;";
        
        std::vector<std::any> params = {id};
        auto result = ExecuteQuery(sql, params);
        
        if (!result.empty()) {
            return ConvertToPlayer(result[0]);
        }
    } catch (const std::exception& e) {
        std::cerr << "查找玩家失败: " << e.what() << std::endl;
    }
    return std::nullopt;
}

std::optional<Player> PlayerRepositoryDatabase::FindPlayerByUsername(const std::string& username) {
    try {
        std::string sql = "SELECT id, username, password_hash, email, created_at FROM players WHERE username = $1;";
        
        std::vector<std::any> params = {username};
        auto result = ExecuteQuery(sql, params);
        
        if (!result.empty()) {
            return ConvertToPlayer(result[0]);
        }
    } catch (const std::exception& e) {
        std::cerr << "查找玩家失败: " << e.what() << std::endl;
    }
    return std::nullopt;
}

bool PlayerRepositoryDatabase::UpdatePlayer(const Player& player) {
    try {
        std::string sql = "UPDATE players SET username = $1, password_hash = $2, email = $3 WHERE id = $4;";
        
        std::vector<std::any> params = {player.username, player.password_hash, player.email, player.id};
        int affected_rows = ExecuteUpdate(sql, params);
        
        return affected_rows == 1;
    } catch (const std::exception& e) {
        std::cerr << "更新玩家失败: " << e.what() << std::endl;
        return false;
    }
}

bool PlayerRepositoryDatabase::DeletePlayer(long long id) {
    try {
        std::string sql = "DELETE FROM players WHERE id = $1;";
        
        std::vector<std::any> params = {id};
        int affected_rows = ExecuteUpdate(sql, params);
        
        return affected_rows == 1;
    } catch (const std::exception& e) {
        std::cerr << "删除玩家失败: " << e.what() << std::endl;
        return false;
    }
}

Player PlayerRepositoryDatabase::ConvertToPlayer(const std::map<std::string, std::any>& row) {
    Player player;
    
    try {
        player.id = std::any_cast<long long>(row.at("id"));
        player.username = std::any_cast<std::string>(row.at("username"));
        player.password_hash = std::any_cast<std::string>(row.at("password_hash"));
        player.email = std::any_cast<std::string>(row.at("email"));
        
        // 解析时间戳
        std::string created_at_str = std::any_cast<std::string>(row.at("created_at"));
        player.created_at = ParseTimestamp(created_at_str);
    } catch (const std::exception& e) {
        std::cerr << "转换玩家数据失败: " << e.what() << std::endl;
    }
    
    return player;
}

std::chrono::system_clock::time_point PlayerRepositoryDatabase::ParseTimestamp(const std::string& timestamp_str) {
    std::tm tm = {};
    std::istringstream ss(timestamp_str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

} // namespace strategy