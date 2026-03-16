/*
 * File: StrategyService.h
 * Description: Strategy service implementation that orchestrates rule, world, and event modules.
 */
#ifndef STRATEGY_STRATEGYSERVICE_H
#define STRATEGY_STRATEGYSERVICE_H

#include "../IStrategyService.h"
#include "EventScheduler.h"
#include "GameRuleManager.h"
#include "WorldStateEngine.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace strategy {

class StrategyService : public IStrategyService {
public:
    StrategyService();
    ~StrategyService() override = default;

    WorldStateResult UpdateWorldState(const WorldStateUpdate& update) override;
    EventTriggerResult TriggerEvent(const EventTriggerRequest& request) override;

    GameRules GetGameRules(const std::string& rule_category);
    GameState QueryGameState(const std::string& query_type);

private:
    void UpdateEventRuleMapFromJson(const std::string& world_state_json);
    void UpdateEventRuleMetaFromJson(const std::string& world_state_json);
    std::string ResolveRuleIdForEvent(int event_id) const;
    std::string SerializeEventRuleMapJson() const;
    std::string SerializeEventRuleMetaJson() const;

    std::unique_ptr<GameRuleManager> rule_manager_;
    std::unique_ptr<WorldStateEngine> world_engine_;
    std::unique_ptr<EventScheduler> event_scheduler_;
    std::unordered_map<int, std::string> event_rule_map_;
    std::string event_rule_versions_json_;
    std::string event_rule_publish_history_json_;
};

} // namespace strategy

#endif // STRATEGY_STRATEGYSERVICE_H
