/*
 * 文件名: PlayerService.h
 * 说明: 玩家服务类，提供玩家相关的业务逻辑。
 * 作者: 彭承康
 * 创建时间: 2025-07-13
 */
#ifndef STRATEGY_PLAYER_SERVICE_PLAYERSERVICE_H
#define STRATEGY_PLAYER_SERVICE_PLAYERSERVICE_H

#include "player_service/IPlayerRepository.h"
#include "player_service/Player.h"
#include <memory>
#include <string>
#include <optional>

namespace strategy {

struct LoginResult {
    bool success;
    std::string message;
    std::optional<Player> player;
};

struct RegisterResult {
    bool success;
    std::string message;
    std::optional<Player> player;
};

class PlayerService {
public:
    explicit PlayerService(std::unique_ptr<IPlayerRepository> repository);
    virtual ~PlayerService() = default;

    RegisterResult RegisterPlayer(const std::string& username, const std::string& password, const std::string& email);
    LoginResult LoginPlayer(const std::string& username, const std::string& password);
    std::optional<Player> GetPlayerById(long long id);
    bool UpdatePlayerEmail(long long id, const std::string& new_email);
    bool UpdatePlayerPassword(long long id, const std::string& old_password, const std::string& new_password);
    bool DeletePlayer(long long id);

private:
    std::string HashPassword(const std::string& password);
    bool VerifyPassword(const std::string& password, const std::string& hash);
    bool IsValidEmail(const std::string& email);
    bool IsValidUsername(const std::string& username);

    std::unique_ptr<IPlayerRepository> repository_;
};

} // namespace strategy

#endif // STRATEGY_PLAYER_SERVICE_PLAYERSERVICE_H