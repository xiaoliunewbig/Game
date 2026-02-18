/*
 * 文件名: PlayerRepositoryPostgres.h
 * 说明: 玩家数据仓库的PostgreSQL实现头文件。
 * 作者: 彭承康
 * 创建时间: 2025-07-13
 */
#ifndef STRATEGY_PLAYER_SERVICE_PLAYERREPOSITORYPOSTGRES_H
#define STRATEGY_PLAYER_SERVICE_PLAYERREPOSITORYPOSTGRES_H

#ifdef HAS_PQXX

#include "player_service/IPlayerRepository.h"
#include <pqxx/pqxx>
#include <memory>

namespace strategy {

class PlayerRepositoryPostgres : public IPlayerRepository {
public:
    /**
     * @brief 构造函数
     * @param connection_string PostgreSQL的连接字符串
     */
    explicit PlayerRepositoryPostgres(const std::string& connection_string);

    virtual ~PlayerRepositoryPostgres() = default;

    std::optional<Player> CreatePlayer(const std::string& username, const std::string& password_hash, const std::string& email) override;
    std::optional<Player> FindPlayerByUsername(const std::string& username) override;
    std::optional<Player> FindPlayerById(long long id) override;
    bool UpdatePlayer(const Player& player) override;
    bool DeletePlayer(long long id) override;

private:
    /**
     * @brief 将pqxx::row转换为Player对象
     */
    Player RowToPlayer(const pqxx::row& row);

    std::unique_ptr<pqxx::connection> connection_;
};

} // namespace strategy

#endif // HAS_PQXX

#endif // STRATEGY_PLAYER_SERVICE_PLAYERREPOSITORYPOSTGRES_H 