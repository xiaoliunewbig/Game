/*
 * 文件名: PlayerServiceFactory.cpp
 * 说明: 玩家服务工厂类的实现。
 * 作者: 彭承康
 * 创建时间: 2025-07-13
 */
#include "player_service/PlayerServiceFactory.h"
#include "player_service/PlayerRepositoryDatabase.h"
#include "database/DatabaseFactory.h"
#include "database/DatabaseConfigManager.h"
#include <stdexcept>

namespace strategy {

std::unique_ptr<PlayerService> PlayerServiceFactory::CreatePostgresPlayerService(const std::string& connection_string) {
#ifdef HAS_PQXX
    // 为了兼容性保留旧接口
    auto repository = std::make_unique<PlayerRepositoryPostgres>(connection_string);
    return std::make_unique<PlayerService>(std::move(repository));
#else
    (void)connection_string;
    throw std::runtime_error("PostgreSQL支持未启用（需要编译时定义HAS_PQXX）");
#endif
}

std::unique_ptr<PlayerService> PlayerServiceFactory::CreatePlayerService(std::unique_ptr<IPlayerRepository> repository) {
    return std::make_unique<PlayerService>(std::move(repository));
}

std::unique_ptr<PlayerService> PlayerServiceFactory::CreatePlayerServiceWithConfig(const std::string& config_name) {
    auto connection = DatabaseFactory::CreateConnection(config_name);
    auto repository = std::make_unique<PlayerRepositoryDatabase>(std::move(connection));
    return std::make_unique<PlayerService>(std::move(repository));
}

std::unique_ptr<PlayerService> PlayerServiceFactory::CreateDefaultPlayerService() {
    auto connection = DatabaseFactory::CreateDefaultConnection();
    auto repository = std::make_unique<PlayerRepositoryDatabase>(std::move(connection));
    return std::make_unique<PlayerService>(std::move(repository));
}

} // namespace strategy
