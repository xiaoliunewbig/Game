/*
 * 文件名: GameRuleManager.cpp
 * 说明: 游戏规则管理器的具体实现。
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 *
 * 本文件实现游戏规则管理的核心逻辑，包括规则初始化、条件检查、
 * 效果应用等功能，为策略层提供规则支持。
 */
#include "Algorithm_interact/GameRuleManager.h"
#include <algorithm>

namespace strategy {

GameRuleManager::GameRuleManager() {
    InitializeDefaultRules();
}

GameRules GameRuleManager::GetRulesByCategory(const std::string& category) {
    GameRules result;
    result.category = category;
    result.version = 1;
    
    for (const auto& pair : rules_) {
        if (pair.second.category == category && pair.second.is_active) {
            result.rules.push_back(pair.second);
        }
    }
    
    // 按优先级排序
    std::sort(result.rules.begin(), result.rules.end(), 
              [](const GameRule& a, const GameRule& b) {
                  return a.priority > b.priority;
              });
    
    return result;
}

bool GameRuleManager::CheckRuleCondition(const std::string& rule_id, 
                                        const std::unordered_map<std::string, std::string>& context) {
    auto it = rules_.find(rule_id);
    if (it == rules_.end() || !it->second.is_active) {
        return false;
    }
    
    const GameRule& rule = it->second;
    
    // 检查所有条件
    for (const auto& condition : rule.conditions) {
        auto context_it = context.find(condition.first);
        if (context_it == context.end()) {
            return false;  // 缺少必要的上下文
        }
        
        // 简化的条件检查逻辑
        if (condition.second.find(">=") == 0) {
            int required_value = std::stoi(condition.second.substr(2));
            int actual_value = std::stoi(context_it->second);
            if (actual_value < required_value) return false;
        } else if (condition.second.find("==") == 0) {
            std::string required_value = condition.second.substr(2);
            if (context_it->second != required_value) return false;
        }
    }
    
    return true;
}

std::unordered_map<std::string, std::string> GameRuleManager::ApplyRuleEffect(const std::string& rule_id) {
    auto it = rules_.find(rule_id);
    if (it == rules_.end() || !it->second.is_active) {
        return {};
    }
    
    return it->second.effects;
}

bool GameRuleManager::AddRule(const GameRule& rule) {
    rules_[rule.rule_id] = rule;
    return true;
}

bool GameRuleManager::UpdateRuleStatus(const std::string& rule_id, bool is_active) {
    auto it = rules_.find(rule_id);
    if (it == rules_.end()) {
        return false;
    }
    
    it->second.is_active = is_active;
    return true;
}

void GameRuleManager::InitializeDefaultRules() {
    InitializeCombatRules();
    InitializeStoryRules();
    InitializeQuestRules();
}

void GameRuleManager::InitializeCombatRules() {
    // 战斗开始规则
    GameRule combat_start_rule;
    combat_start_rule.rule_id = "combat_start";
    combat_start_rule.description = "战斗开始条件";
    combat_start_rule.category = "combat";
    combat_start_rule.conditions = {
        {"enemy_distance", "<=5"},
        {"player_health", ">=1"}
    };
    combat_start_rule.effects = {
        {"flag_in_combat", "true"},
        {"global_combat_count", "+1"}
    };
    combat_start_rule.priority = 100;
    combat_start_rule.is_active = true;
    
    rules_[combat_start_rule.rule_id] = combat_start_rule;
}

void GameRuleManager::InitializeStoryRules() {
    // 剧情触发规则
    GameRule story_trigger_rule;
    story_trigger_rule.rule_id = "story_chapter_1";
    story_trigger_rule.description = "第一章剧情触发";
    story_trigger_rule.category = "story";
    story_trigger_rule.conditions = {
        {"player_level", ">=5"},
        {"flag_tutorial_complete", "==true"}
    };
    story_trigger_rule.effects = {
        {"flag_chapter_1_available", "true"},
        {"global_story_progress", "1"}
    };
    story_trigger_rule.priority = 80;
    story_trigger_rule.is_active = true;
    
    rules_[story_trigger_rule.rule_id] = story_trigger_rule;
}

void GameRuleManager::InitializeQuestRules() {
    // 任务完成规则
    GameRule quest_complete_rule;
    quest_complete_rule.rule_id = "quest_kill_monsters";
    quest_complete_rule.description = "击杀怪物任务完成";
    quest_complete_rule.category = "quest";
    quest_complete_rule.conditions = {
        {"monsters_killed", ">=10"}
    };
    quest_complete_rule.effects = {
        {"flag_quest_complete", "true"},
        {"global_player_exp", "+100"}
    };
    quest_complete_rule.priority = 60;
    quest_complete_rule.is_active = true;
    
    rules_[quest_complete_rule.rule_id] = quest_complete_rule;
}

} // namespace strategy
