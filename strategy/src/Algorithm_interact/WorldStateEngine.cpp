/*
 * 文件名: WorldStateEngine.cpp
 * 说明: 世界状态引擎的具体实现。
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 *
 * 本文件实现世界状态管理的核心逻辑，包括NPC关系管理、世界状态持久化、
 * AI决策上下文生成等功能。
 */
#include "Algorithm_interact/WorldStateEngine.h"
#include <sstream>
#include <chrono>
#include <algorithm>

namespace strategy {

WorldStateEngine::WorldStateEngine() {
    InitializeDefaultState();
}

bool WorldStateEngine::UpdateWorldState(const std::string& world_state_json) {
    try {
        bool success = DeserializeWorldState(world_state_json);
        if (success) {
            current_state_.last_update_time = 
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
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
        // 只返回NPC关系数据
        WorldState filtered_state;
        filtered_state.npc_relations = current_state_.npc_relations;
        result.world_state = filtered_state;
    } else if (query_type == "global_vars") {
        // 只返回全局变量
        WorldState filtered_state;
        filtered_state.global_variables = current_state_.global_variables;
        result.world_state = filtered_state;
    }
    
    return result;
}

bool WorldStateEngine::UpdateNPCRelation(int npc_id, int target_id, 
                                        const std::string& relation_type, int trust_change) {
    int relation_key = npc_id * 10000 + target_id;  // 简单的复合键
    
    auto it = current_state_.npc_relations.find(relation_key);
    if (it != current_state_.npc_relations.end()) {
        // 更新现有关系
        it->second.relation_type = relation_type;
        it->second.trust_level = std::max(-100, std::min(100, it->second.trust_level + trust_change));
    } else {
        // 创建新关系
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
    int relation_key = npc_id * 10000 + target_id;
    
    auto it = current_state_.npc_relations.find(relation_key);
    if (it != current_state_.npc_relations.end()) {
        return it->second;
    }
    
    // 返回默认中性关系
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
    
    // 添加NPC相关的上下文信息
    context.push_back(npc_id);
    
    // 添加与玩家的关系
    NPCRelation player_relation = GetNPCRelation(npc_id, 1);  // 假设玩家ID为1
    context.push_back(player_relation.trust_level);
    
    // 添加重要的全局变量
    context.push_back(GetGlobalVariable("player_level"));
    context.push_back(GetGlobalVariable("combat_count"));
    context.push_back(GetGlobalVariable("story_progress"));
    
    // 添加重要的世界标志（转换为整数）
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
    
    current_state_.last_update_time = 
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string WorldStateEngine::SerializeWorldState() const {
    std::ostringstream oss;
    oss << "{";
    oss << "\"global_variables\":{";
    bool first = true;
    for (const auto& var : current_state_.global_variables) {
        if (!first) oss << ",";
        oss << "\"" << var.first << "\":" << var.second;
        first = false;
    }
    oss << "},";
    
    oss << "\"world_flags\":{";
    first = true;
    for (const auto& flag : current_state_.world_flags) {
        if (!first) oss << ",";
        oss << "\"" << flag.first << "\":" << (flag.second ? "true" : "false");
        first = false;
    }
    oss << "},";
    
    oss << "\"last_update_time\":" << current_state_.last_update_time;
    oss << "}";
    
    return oss.str();
}

bool WorldStateEngine::DeserializeWorldState(const std::string& json_data) {
    // 简化的JSON解析实现
    // 在实际项目中应该使用专业的JSON库如nlohmann/json
    
    if (json_data.empty()) {
        return false;
    }
    
    // 这里只是一个占位符实现
    // 实际应该解析JSON并更新current_state_
    return true;
}

} // namespace strategy
