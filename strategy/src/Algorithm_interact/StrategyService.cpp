/*
 * File: StrategyService.cpp
 * Description: Strategy service implementation.
 */

#include "Algorithm_interact/StrategyService.h"

#include <exception>
#include <string>
#include <unordered_map>

namespace {

std::string ResolveRuleIdForEvent(int event_id) {
    switch (event_id) {
        case 1001:
            return "story_chapter_1";
        case 2001:
            return "combat_start";
        case 3001:
            return "quest_kill_monsters";
        default:
            return "event_" + std::to_string(event_id);
    }
}

int ApplyNumericEffectValue(const std::string& effect_value, int current_value) {
    if (!effect_value.empty() && (effect_value[0] == '+' || effect_value[0] == '-')) {
        return current_value + std::stoi(effect_value);
    }
    return std::stoi(effect_value);
}

bool ParseBoolEffectValue(const std::string& value) {
    return value == "true" || value == "1" || value == "on";
}

std::unordered_map<std::string, std::string> BuildRuleContext(
    const strategy::WorldState& state,
    const strategy::EventTriggerRequest& request) {
    std::unordered_map<std::string, std::string> context;

    for (const auto& var : state.global_variables) {
        context[var.first] = std::to_string(var.second);
    }

    for (const auto& flag : state.world_flags) {
        context["flag_" + flag.first] = flag.second ? "true" : "false";
    }

    if (request.event_id == 1001 && !request.params.empty()) {
        context["player_level"] = std::to_string(request.params[0]);
    } else if (request.event_id == 2001) {
        context["player_health"] = request.params.empty() ? "100" : std::to_string(request.params[0]);
        if (context.find("enemy_distance") == context.end()) {
            context["enemy_distance"] = "3";
        }
    } else if (request.event_id == 3001 && !request.params.empty()) {
        const int killed = request.params.size() > 1 ? request.params[1] : request.params[0];
        context["monsters_killed"] = std::to_string(killed);
    }

    return context;
}

} // namespace

namespace strategy {

StrategyService::StrategyService()
    : rule_manager_(std::make_unique<GameRuleManager>()),
      world_engine_(std::make_unique<WorldStateEngine>()),
      event_scheduler_(std::make_unique<EventScheduler>()) {
}

WorldStateResult StrategyService::UpdateWorldState(const WorldStateUpdate& update) {
    WorldStateResult result;

    try {
        const bool success = world_engine_->UpdateWorldState(update.world_state_json);
        if (success) {
            result.success = true;
            result.message = "World state updated successfully";
            event_scheduler_->Update();
        } else {
            result.success = false;
            result.message = "World state update failed: invalid payload";
        }
    } catch (const std::exception& e) {
        result.success = false;
        result.message = std::string("World state update exception: ") + e.what();
    }

    return result;
}

EventTriggerResult StrategyService::TriggerEvent(const EventTriggerRequest& request) {
    EventTriggerResult result;

    try {
        const std::string rule_id = ResolveRuleIdForEvent(request.event_id);
        const WorldState state = world_engine_->GetCurrentWorldState();
        const auto rule_context = BuildRuleContext(state, request);

        const bool can_trigger = rule_manager_->CheckRuleCondition(rule_id, rule_context);
        if (!can_trigger) {
            result.triggered = false;
            result.result_json = "{\"error\":\"event rule conditions not satisfied\"}";
            return result;
        }

        result = event_scheduler_->TriggerEvent(request.event_id, request.params);
        if (!result.triggered) {
            return result;
        }

        const auto effects = rule_manager_->ApplyRuleEffect(rule_id);
        for (const auto& effect : effects) {
            if (effect.first.rfind("global_", 0) == 0) {
                const std::string var_name = effect.first.substr(7);
                const int current_value = world_engine_->GetGlobalVariable(var_name);
                const int next_value = ApplyNumericEffectValue(effect.second, current_value);
                world_engine_->SetGlobalVariable(var_name, next_value);
                result.state_changes.push_back("global:" + var_name);
            } else if (effect.first.rfind("flag_", 0) == 0) {
                const std::string flag_name = effect.first.substr(5);
                world_engine_->SetWorldFlag(flag_name, ParseBoolEffectValue(effect.second));
                result.state_changes.push_back("flag:" + flag_name);
            }
        }
    } catch (const std::exception& e) {
        result.triggered = false;
        result.result_json = std::string("{\"error\":\"") + e.what() + "\"}";
    }

    return result;
}

GameRules StrategyService::GetGameRules(const std::string& rule_category) {
    return rule_manager_->GetRulesByCategory(rule_category);
}

GameState StrategyService::QueryGameState(const std::string& query_type) {
    return world_engine_->QueryGameState(query_type);
}

} // namespace strategy
