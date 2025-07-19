/*
 * 文件名: IPlayerRepository.h
 * 说明: 定义了玩家数据仓库的接口。
 * 作者: 彭承康
 * 创建时间: 2025-07-13
 *
 * 该接口遵循仓库模式，将数据访问逻辑与业务逻辑解耦。
 */
#ifndef STRATEGY_PLAYER_SERVICE_IPLAYERREPOSITORY_H
#define STRATEGY_PLAYER_SERVICE_IPLAYERREPOSITORY_H

#include "player_service/Player.h"
#include <string>
#include <optional>
#include <memory>

namespace strategy {

class IPlayerRepository {
public:
    virtual ~IPlayerRepository() = default;

    /**
     * @brief 创建一个新玩家
     * @param username 用户名
     * @param password_hash 加密后的密码
     * @param email 电子邮箱
     * @return 返回创建的玩家对象，如果创建失败则返回std::nullopt
     */
    virtual std::optional<Player> CreatePlayer(const std::string& username, const std::string& password_hash, const std::string& email) = 0;

    /**
     * @brief 根据用户名查找玩家
     * @param username 要查找的用户名
     * @return 如果找到，返回玩家对象；否则返回std::nullopt
     */
    virtual std::optional<Player> FindPlayerByUsername(const std::string& username) = 0;
    
    /**
     * @brief 根据ID查找玩家
     * @param id 要查找的玩家ID
     * @return 如果找到，返回玩家对象；否则返回std::nullopt
     */
    virtual std::optional<Player> FindPlayerById(long long id) = 0;

    /**
     * @brief 更新玩家信息（例如，邮箱或密码）
     * @param player 包含要更新信息的玩家对象 (ID必须有效)
     * @return 如果更新成功，返回 true
     */
    virtual bool UpdatePlayer(const Player& player) = 0;

    /**
     * @brief 根据ID删除玩家
     * @param id 要删除的玩家ID
     * @return 如果删除成功，返回 true
     */
    virtual bool DeletePlayer(long long id) = 0;
};

} // namespace strategy

#endif // STRATEGY_PLAYER_SERVICE_IPLAYERREPOSITORY_H 