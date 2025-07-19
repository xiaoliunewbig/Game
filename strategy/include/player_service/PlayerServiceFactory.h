/*
 * 文件名: PlayerServiceFactory.h
 * 说明: 玩家服务工厂类，用于创建和配置玩家服务。
 * 作者: 彭承康
 * 创建时间: 2025-07-13
 */
#ifndef STRATEGY_PLAYER_SERVICE_PLAYERSERVICEFACTORY_H
#define STRATEGY_PLAYER_SERVICE_PLAYERSERVICEFACTORY_H

#include "player_service/PlayerService.h"
#include "player_service/PlayerRepositoryPostgres.h"
#include <memory>
#include <string>

namespace strategy {

class PlayerServiceFactory {
public:
    /**
     * @brief 创建基于PostgreSQL的玩家服务（兼容性接口）
     */
    static std::unique_ptr<PlayerService> CreatePostgresPlayerService(const std::string& connection_string);
    
    /**
     * @brief 使用自定义仓储创建玩家服务
     */
    static std::unique_ptr<PlayerService> CreatePlayerService(std::unique_ptr<IPlayerRepository> repository);
    
    /**
     * @brief 根据配置名称创建玩家服务
     */
    static std::unique_ptr<PlayerService> CreatePlayerServiceWithConfig(const std::string& config_name);
    
    /**
     * @brief 创建默认配置的玩家服务
     */
    static std::unique_ptr<PlayerService> CreateDefaultPlayerService();
};

} // namespace strategy

#endif // STRATEGY_PLAYER_SERVICE_PLAYERSERVICEFACTORY_H
