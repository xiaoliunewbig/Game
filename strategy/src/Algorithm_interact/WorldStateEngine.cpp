#include "Algorithm_interact/WorldStateEngine.h"

#include <algorithm>
#include <chrono>
#include <regex>
#include <sstream>

namespace {

std::string ExtractObjectSection(const std::string& json, const std::string& key) {
    const std::string token = "\"" + key + "\"";
    const std::size_t key_pos = json.find(token);
    if (key_pos == std::string::npos) {
        return {};
    }

    const std::size_t open_brace = json.find('{', key_pos);
    if (open_brace == std::string::npos) {
        return {};
    }

    int depth = 0;
    for (std::size_t i = open_brace; i < json.size(); ++i) {
        if (json[i] == '{') {
            ++depth;
        } else if (json[i] == '}') {
            --depth;
            if (depth == 0) {
                return json.substr(open_brace + 1, i - open_brace - 1);
            }
        }
    }

    return {};
}

std::string ExtractArraySection(const std::string& json, const std::string& key) {
    const std::string token = "\"" + key + "\"";
    const std::size_t key_pos = json.find(token);
    if (key_pos == std::string::npos) {
        return {};
    }

    const std::size_t open_bracket = json.find('[', key_pos);
    if (open_bracket == std::string::npos) {
        return {};
    }

    int depth = 0;
    for (std::size_t i = open_bracket; i < json.size(); ++i) {
        if (json[i] == '[') {
            ++depth;
        } else if (json[i] == ']') {
            --depth;
            if (depth == 0) {
                return json.substr(open_bracket + 1, i - open_bracket - 1);
            }
        }
    }

    return {};
}

void ParseIntMapSection(const std::string& section, std::unordered_map<std::string, int>& output) {
    const std::regex pair_regex("\"([^\"]+)\"\\s*:\\s*(-?\\d+)");
    for (std::sregex_iterator it(section.begin(), section.end(), pair_regex), end; it != end; ++it) {
        output[(*it)[1].str()] = std::stoi((*it)[2].str());
    }
}

void ParseBoolMapSection(const std::string& section, std::unordered_map<std::string, bool>& output) {
    const std::regex pair_regex("\"([^\"]+)\"\\s*:\\s*(true|false)");
    for (std::sregex_iterator it(section.begin(), section.end(), pair_regex), end; it != end; ++it) {
        output[(*it)[1].str()] = ((*it)[2].str() == "true");
    }
}

bool TryExtractInt(const std::string& json, const std::string& key, int& out_value) {
    const std::regex key_regex("\"" + key + "\"\\s*:\\s*(-?\\d+)");
    std::smatch match;
    if (std::regex_search(json, match, key_regex)) {
        out_value = std::stoi(match[1].str());
        return true;
    }
    return false;
}

bool TryExtractBool(const std::string& json, const std::string& key, bool& out_value) {
    const std::regex key_regex("\"" + key + "\"\\s*:\\s*(true|false)");
    std::smatch match;
    if (std::regex_search(json, match, key_regex)) {
        out_value = (match[1].str() == "true");
        return true;
    }
    return false;
}

} // namespace

namespace strategy {

WorldStateEngine::WorldStateEngine() {
    InitializeDefaultState();
}

bool WorldStateEngine::UpdateWorldState(const std::string& world_state_json) {
    try {
        const bool success = DeserializeWorldState(world_state_json);
        if (success) {
            current_state_.last_update_time =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
        }
        return success;
    } catch (const std::exception&) {
        return false;
    }
}

WorldState WorldStateEngine::GetCurrentWorldState() const {
    return current_state_;
}

GameState WorldStateEngine::QueryGameState(const std::string& query_type) {
    GameState result;
    result.world_state = current_state_;
    result.state_json = SerializeWorldState();
    result.is_valid = true;

    if (query_type == "npc_relations") {
        WorldState filtered_state;
        filtered_state.npc_relations = current_state_.npc_relations;
        filtered_state.last_update_time = current_state_.last_update_time;
        result.world_state = filtered_state;
    } else if (query_type == "global_vars") {
        WorldState filtered_state;
        filtered_state.global_variables = current_state_.global_variables;
        filtered_state.last_update_time = current_state_.last_update_time;
        result.world_state = filtered_state;
    } else if (query_type == "flags") {
        WorldState filtered_state;
        filtered_state.world_flags = current_state_.world_flags;
        filtered_state.last_update_time = current_state_.last_update_time;
        result.world_state = filtered_state;
    }

    return result;
}

bool WorldStateEngine::UpdateNPCRelation(
    int npc_id,
    int target_id,
    const std::string& relation_type,
    int trust_change) {
    const int relation_key = npc_id * 10000 + target_id;

    auto it = current_state_.npc_relations.find(relation_key);
    if (it != current_state_.npc_relations.end()) {
        it->second.relation_type = relation_type;
        it->second.trust_level = std::max(-100, std::min(100, it->second.trust_level + trust_change));
    } else {
        NPCRelation new_relation;
        new_relation.npc_id = npc_id;
        new_relation.target_id = target_id;
        new_relation.relation_type = relation_type;
        new_relation.trust_level = trust_change;
        current_state_.npc_relations[relation_key] = new_relation;
    }

    return true;
}

NPCRelation WorldStateEngine::GetNPCRelation(int npc_id, int target_id) const {
    const int relation_key = npc_id * 10000 + target_id;

    auto it = current_state_.npc_relations.find(relation_key);
    if (it != current_state_.npc_relations.end()) {
        return it->second;
    }

    NPCRelation default_relation;
    default_relation.npc_id = npc_id;
    default_relation.target_id = target_id;
    default_relation.relation_type = "neutral";
    default_relation.trust_level = 0;
    return default_relation;
}

void WorldStateEngine::SetGlobalVariable(const std::string& var_name, int value) {
    current_state_.global_variables[var_name] = value;
}

int WorldStateEngine::GetGlobalVariable(const std::string& var_name) const {
    auto it = current_state_.global_variables.find(var_name);
    return (it != current_state_.global_variables.end()) ? it->second : 0;
}

void WorldStateEngine::SetWorldFlag(const std::string& flag_name, bool value) {
    current_state_.world_flags[flag_name] = value;
}

bool WorldStateEngine::GetWorldFlag(const std::string& flag_name) const {
    auto it = current_state_.world_flags.find(flag_name);
    return (it != current_state_.world_flags.end()) ? it->second : false;
}

std::vector<int> WorldStateEngine::GetAIDecisionContext(int npc_id) const {
    std::vector<int> context;

    context.push_back(npc_id);

    const NPCRelation player_relation = GetNPCRelation(npc_id, 1);
    context.push_back(player_relation.trust_level);

    context.push_back(GetGlobalVariable("player_level"));
    context.push_back(GetGlobalVariable("combat_count"));
    context.push_back(GetGlobalVariable("story_progress"));

    context.push_back(GetWorldFlag("in_combat") ? 1 : 0);
    context.push_back(GetWorldFlag("tutorial_complete") ? 1 : 0);

    return context;
}

void WorldStateEngine::InitializeDefaultState() {
    current_state_.global_variables = {
        {"player_level", 1},
        {"combat_count", 0},
        {"story_progress", 0},
        {"player_exp", 0},
        {"monsters_killed", 0}
    };

    current_state_.world_flags = {
        {"tutorial_complete", false},
        {"in_combat", false},
        {"chapter_1_available", false},
        {"quest_complete", false}
    };

    current_state_.active_events = {};

    current_state_.last_update_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count();
}

std::string WorldStateEngine::SerializeWorldState() const {
    std::ostringstream oss;
    oss << "{";

    oss << "\"global_variables\":{";
    bool first = true;
    for (const auto& var : current_state_.global_variables) {
        if (!first) {
            oss << ",";
        }
        oss << "\"" << var.first << "\":" << var.second;
        first = false;
    }
    oss << "},";

    oss << "\"world_flags\":{";
    first = true;
    for (const auto& flag : current_state_.world_flags) {
        if (!first) {
            oss << ",";
        }
        oss << "\"" << flag.first << "\":" << (flag.second ? "true" : "false");
        first = false;
    }
    oss << "},";

    oss << "\"active_events\":[";
    for (std::size_t i = 0; i < current_state_.active_events.size(); ++i) {
        if (i != 0) {
            oss << ",";
        }
        oss << "\"" << current_state_.active_events[i] << "\"";
    }
    oss << "],";

    oss << "\"last_update_time\":" << current_state_.last_update_time;
    oss << "}";

    return oss.str();
}

bool WorldStateEngine::DeserializeWorldState(const std::string& json_data) {
    if (json_data.empty()) {
        return false;
    }

    const std::string global_vars_section = ExtractObjectSection(json_data, "global_variables");
    if (!global_vars_section.empty()) {
        ParseIntMapSection(global_vars_section, current_state_.global_variables);
    }

    const std::string world_flags_section = ExtractObjectSection(json_data, "world_flags");
    if (!world_flags_section.empty()) {
        ParseBoolMapSection(world_flags_section, current_state_.world_flags);
    }

    const std::string active_events_section = ExtractArraySection(json_data, "active_events");
    if (!active_events_section.empty()) {
        current_state_.active_events.clear();
        const std::regex event_regex("\"([^\"]+)\"");
        for (std::sregex_iterator it(active_events_section.begin(), active_events_section.end(), event_regex), end;
             it != end;
             ++it) {
            current_state_.active_events.push_back((*it)[1].str());
        }
    }

    // Map lightweight frontend payload keys.
    int int_value = 0;
    if (TryExtractInt(json_data, "level", int_value)) {
        current_state_.global_variables["player_level"] = int_value;
    }
    if (TryExtractInt(json_data, "story_progress", int_value)) {
        current_state_.global_variables["story_progress"] = int_value;
    } else if (TryExtractInt(json_data, "day", int_value)) {
        current_state_.global_variables["story_progress"] = std::max(0, int_value - 1);
    }
    if (TryExtractInt(json_data, "gold", int_value)) {
        current_state_.global_variables["player_exp"] = int_value;
    }
    if (TryExtractInt(json_data, "combat_count", int_value)) {
        current_state_.global_variables["combat_count"] = int_value;
    }

    bool bool_value = false;
    if (TryExtractBool(json_data, "in_combat", bool_value)) {
        current_state_.world_flags["in_combat"] = bool_value;
    }
    if (TryExtractBool(json_data, "tutorial_complete", bool_value)) {
        current_state_.world_flags["tutorial_complete"] = bool_value;
    }
    if (TryExtractBool(json_data, "quest_complete", bool_value)) {
        current_state_.world_flags["quest_complete"] = bool_value;
    }

    return true;
}

} // namespace strategy
