/*
 * 文件名: StrategyService.h
 * 说明: 策略服务实现类，提供游戏规则管理、世界状态管理和事件调度。
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 *
 * 本类实现IStrategyService接口，整合GameRuleManager、WorldStateEngine和EventScheduler，
 * 为应用层提供统一的策略服务入口，并与算法层进行数据交互。
 */
#ifndef STRATEGY_STRATEGYSERVICE_H
#define STRATEGY_STRATEGYSERVICE_H

#include "../IStrategyService.h"
#include "GameRuleManager.h"
#include "WorldStateEngine.h"
#include "EventScheduler.h"
#include <memory>

namespace strategy {

/**
 * @brief 策略服务实现类
 * 实现 IStrategyService 接口，提供具体的策略服务
 */
class StrategyService : public IStrategyService {
public:
    StrategyService();
    virtual ~StrategyService() = default;

    /**
     * @brief 更新世界状态
     */
    WorldStateResult UpdateWorldState(const WorldStateUpdate& update) override;

    /**
     * @brief 触发事件
     */
    EventTriggerResult TriggerEvent(const EventTriggerRequest& request) override;

    /**
     * @brief 获取游戏规则
     */
    GameRules GetGameRules(const std::string& rule_category);

    /**
     * @brief 查询当前游戏状态
     */
    GameState QueryGameState(const std::string& query_type);

private:
    std::unique_ptr<GameRuleManager> rule_manager_;
    std::unique_ptr<WorldStateEngine> world_engine_;
    std::unique_ptr<EventScheduler> event_scheduler_;
};

} // namespace strategy

#endif // STRATEGY_STRATEGYSERVICE_H
