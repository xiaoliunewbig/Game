/*
 * File: StrategyService.cpp
 * Description: Strategy service implementation.
 */

#include "Algorithm_interact/StrategyService.h"

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>

namespace {

std::unordered_map<int, std::string> BuildDefaultEventRuleMap() {
    return {
        {1001, "story_chapter_1"},
        {2001, "combat_start"},
        {3001, "quest_kill_monsters"}
    };
}

std::unordered_map<int, std::string> LoadEventRuleMapFromEnv() {
    std::unordered_map<int, std::string> mapping = BuildDefaultEventRuleMap();

    const char* raw = std::getenv("STRATEGY_EVENT_RULE_MAP");
    if (raw == nullptr) {
        return mapping;
    }

    std::string text(raw);
    std::replace(text.begin(), text.end(), ',', ';');

    std::stringstream ss(text);
    std::string token;
    while (std::getline(ss, token, ';')) {
        if (token.empty()) {
            continue;
        }

        const std::size_t sep = token.find('=');
        if (sep == std::string::npos) {
            continue;
        }

        const std::string id_text = token.substr(0, sep);
        const std::string rule_id = token.substr(sep + 1);
        if (id_text.empty() || rule_id.empty()) {
            continue;
        }

        try {
            const int event_id = std::stoi(id_text);
            mapping[event_id] = rule_id;
        } catch (...) {
        }
    }

    return mapping;
}

std::string ExtractJsonArrayByKey(const std::string& json, const std::string& key) {
    const std::string quoted_key = "\"" + key + "\"";
    const std::size_t key_pos = json.find(quoted_key);
    if (key_pos == std::string::npos) {
        return "";
    }

    const std::size_t start = json.find('[', key_pos);
    if (start == std::string::npos) {
        return "";
    }

    bool in_string = false;
    bool escape = false;
    int depth = 0;

    for (std::size_t i = start; i < json.size(); ++i) {
        const char ch = json[i];
        if (escape) {
            escape = false;
            continue;
        }

        if (ch == '\\') {
            escape = true;
            continue;
        }

        if (ch == '\"') {
            in_string = !in_string;
            continue;
        }

        if (in_string) {
            continue;
        }

        if (ch == '[') {
            ++depth;
        } else if (ch == ']') {
            --depth;
            if (depth == 0) {
                return json.substr(start, i - start + 1);
            }
        }
    }

    return "";
}

std::unordered_map<int, std::string> ParseEventRuleMapFromJson(const std::string& json) {
    std::unordered_map<int, std::string> parsed;

    const std::size_t key_pos = json.find("\"event_rule_map\"");
    if (key_pos == std::string::npos) {
        return parsed;
    }

    const std::size_t start = json.find('{', key_pos);
    if (start == std::string::npos) {
        return parsed;
    }

    int depth = 0;
    std::size_t end = std::string::npos;
    for (std::size_t i = start; i < json.size(); ++i) {
        if (json[i] == '{') {
            ++depth;
        } else if (json[i] == '}') {
            --depth;
            if (depth == 0) {
                end = i;
                break;
            }
        }
    }

    if (end == std::string::npos || end <= start) {
        return parsed;
    }

    const std::string body = json.substr(start + 1, end - start - 1);
    const std::regex pair_regex("\"(\\d+)\"\\s*:\\s*\"([^\"]+)\"");
    for (std::sregex_iterator it(body.begin(), body.end(), pair_regex), regex_end; it != regex_end; ++it) {
        parsed[std::stoi((*it)[1].str())] = (*it)[2].str();
    }

    return parsed;
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

    for (std::size_t i = 0; i < request.params.size(); ++i) {
        context["param_" + std::to_string(i)] = std::to_string(request.params[i]);
    }

    if (request.event_id == 1001 && !request.params.empty()) {
        context["player_level"] = std::to_string(request.params[0]);
    } else if (request.event_id == 2001) {
        context["player_health"] = request.params.empty() ? "100" : std::to_string(request.params[0]);
        context["enemy_distance"] = "3";
    } else if (request.event_id == 3001 && !request.params.empty()) {
        const int killed = request.params.size() > 1 ? request.params[1] : request.params[0];
        context["monsters_killed"] = std::to_string(killed);
    }

    if (context.find("monsters_killed") == context.end() && !request.params.empty()) {
        context["monsters_killed"] = std::to_string(request.params.back());
    }
    if (context.find("player_level") == context.end() && !request.params.empty()) {
        context["player_level"] = std::to_string(request.params[0]);
    }
    if (context.find("player_health") == context.end()) {
        context["player_health"] = request.params.empty() ? "100" : std::to_string(request.params[0]);
    }
    if (context.find("enemy_distance") == context.end()) {
        context["enemy_distance"] = "3";
    }

    return context;
}

} // namespace

namespace strategy {

StrategyService::StrategyService()
    : rule_manager_(std::make_unique<GameRuleManager>()),
      world_engine_(std::make_unique<WorldStateEngine>()),
      event_scheduler_(std::make_unique<EventScheduler>()),
      event_rule_map_(LoadEventRuleMapFromEnv()),
      event_rule_versions_json_("[]"),
      event_rule_publish_history_json_("[]") {
}

WorldStateResult StrategyService::UpdateWorldState(const WorldStateUpdate& update) {
    WorldStateResult result;

    try {
        UpdateEventRuleMapFromJson(update.world_state_json);
        UpdateEventRuleMetaFromJson(update.world_state_json);
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

void StrategyService::UpdateEventRuleMapFromJson(const std::string& world_state_json) {
    const auto parsed = ParseEventRuleMapFromJson(world_state_json);
    for (const auto& item : parsed) {
        event_rule_map_[item.first] = item.second;
    }
}

void StrategyService::UpdateEventRuleMetaFromJson(const std::string& world_state_json) {
    const std::string versions = ExtractJsonArrayByKey(world_state_json, "event_rule_versions");
    if (!versions.empty()) {
        event_rule_versions_json_ = versions;
    }

    const std::string publish_history = ExtractJsonArrayByKey(world_state_json, "event_rule_publish_history");
    if (!publish_history.empty()) {
        event_rule_publish_history_json_ = publish_history;
    }
}

std::string StrategyService::ResolveRuleIdForEvent(int event_id) const {
    const auto it = event_rule_map_.find(event_id);
    if (it != event_rule_map_.end()) {
        return it->second;
    }
    return "event_" + std::to_string(event_id);
}

GameRules StrategyService::GetGameRules(const std::string& rule_category) {
    return rule_manager_->GetRulesByCategory(rule_category);
}

GameState StrategyService::QueryGameState(const std::string& query_type) {
    if (query_type == "event_rule_map") {
        GameState state;
        state.is_valid = true;
        state.state_json = SerializeEventRuleMapJson();
        return state;
    }

    if (query_type == "event_rule_meta") {
        GameState state;
        state.is_valid = true;
        state.state_json = SerializeEventRuleMetaJson();
        return state;
    }

    return world_engine_->QueryGameState(query_type);
}

std::string StrategyService::SerializeEventRuleMapJson() const {
    std::ostringstream oss;
    oss << "{\"event_rule_map\":{";

    bool first = true;
    for (const auto& item : event_rule_map_) {
        if (!first) {
            oss << ",";
        }
        oss << "\"" << item.first << "\":\"" << item.second << "\"";
        first = false;
    }
    oss << "}}";

    return oss.str();
}

std::string StrategyService::SerializeEventRuleMetaJson() const {
    std::ostringstream oss;
    oss << "{\"event_rule_versions\":" << event_rule_versions_json_
        << ",\"event_rule_publish_history\":" << event_rule_publish_history_json_ << "}";
    return oss.str();
}
} // namespace strategy
