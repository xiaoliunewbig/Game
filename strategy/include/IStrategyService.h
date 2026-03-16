/*
 * File: IStrategyService.h
 * Description: Strategy service interface and DTOs.
 */
#ifndef STRATEGY_ISTRATEGYSERVICE_H
#define STRATEGY_ISTRATEGYSERVICE_H

#include <string>
#include <vector>

namespace strategy {

struct WorldStateUpdate {
    std::string world_state_json;
};

struct WorldStateResult {
    bool success;
    std::string message;
};

struct EventTriggerRequest {
    int event_id;
    std::vector<int> params;
};

struct EventTriggerResult {
    bool triggered;
    std::string result_json;
    std::vector<int> affected_npcs;
    std::vector<std::string> state_changes;
};

class IStrategyService {
public:
    virtual ~IStrategyService() = default;

    virtual WorldStateResult UpdateWorldState(const WorldStateUpdate& update) = 0;
    virtual EventTriggerResult TriggerEvent(const EventTriggerRequest& request) = 0;
};

} // namespace strategy

#endif // STRATEGY_ISTRATEGYSERVICE_H
