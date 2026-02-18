/*
 * 文件名: PlayerRepositoryDatabase.h
 * 说明: 基于通用数据库接口的玩家仓储
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#ifndef STRATEGY_PLAYER_SERVICE_PLAYERREPOSITORYDATABASE_H
#define STRATEGY_PLAYER_SERVICE_PLAYERREPOSITORYDATABASE_H

#include "player_service/IPlayerRepository.h"
#include "database/BaseRepository.h"
#include "database/IDatabaseConnection.h"
#include <memory>

namespace strategy {

class PlayerRepositoryDatabase : public IPlayerRepository, public BaseRepository {
public:
    explicit PlayerRepositoryDatabase(std::unique_ptr<IDatabaseConnection> connection);
    virtual ~PlayerRepositoryDatabase() = default;

    std::optional<Player> CreatePlayer(const std::string& username, const std::string& password_hash, const std::string& email) override;
    std::optional<Player> FindPlayerById(long long id) override;
    std::optional<Player> FindPlayerByUsername(const std::string& username) override;
    bool UpdatePlayer(const Player& player) override;
    bool DeletePlayer(long long id) override;

private:
    Player ConvertToPlayer(const std::map<std::string, std::any>& row);
    std::chrono::system_clock::time_point ParseTimestamp(const std::string& timestamp_str);
};

} // namespace strategy

#endif // STRATEGY_PLAYER_SERVICE_PLAYERREPOSITORYDATABASE_H