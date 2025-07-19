/*
 * 文件名: Player.h
 * 说明: 玩家实体类定义。
 * 作者: 彭承康
 * 创建时间: 2025-07-13
 */
#ifndef STRATEGY_PLAYER_SERVICE_PLAYER_H
#define STRATEGY_PLAYER_SERVICE_PLAYER_H

#include <string>
#include <chrono>

namespace strategy {

/**
 * @brief 玩家实体类
 * 表示游戏中的一个玩家
 */
struct Player {
    long long id;                    // 玩家ID
    std::string username;            // 用户名
    std::string password_hash;       // 密码哈希值
    std::string email;               // 邮箱地址
    std::chrono::system_clock::time_point created_at;  // 创建时间

    /**
     * @brief 默认构造函数
     */
    Player() : id(0) {}

    /**
     * @brief 构造函数
     * @param id 玩家ID
     * @param username 用户名
     * @param password_hash 密码哈希值
     * @param email 邮箱地址
     * @param created_at 创建时间
     */
    Player(long long id, const std::string& username, const std::string& password_hash, 
           const std::string& email, const std::chrono::system_clock::time_point& created_at)
        : id(id), username(username), password_hash(password_hash), email(email), created_at(created_at) {}
};

} // namespace strategy

#endif // STRATEGY_PLAYER_SERVICE_PLAYER_H
