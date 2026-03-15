/*
 * File: GameRuleManager.cpp
 * Description: Game rule manager implementation.
 */

#include "Algorithm_interact/GameRuleManager.h"

#include <algorithm>

namespace {

bool TryParseInt(const std::string& input, int& value) {
    try {
        std::size_t parsed = 0;
        value = std::stoi(input, &parsed);
        return parsed == input.size();
    } catch (...) {
        return false;
    }
}

bool EvaluateCondition(const std::string& expected, const std::string& actual) {
    if (expected.rfind(">=", 0) == 0) {
        int expected_value = 0;
        int actual_value = 0;
        return TryParseInt(expected.substr(2), expected_value) &&
               TryParseInt(actual, actual_value) &&
               actual_value >= expected_value;
    }

    if (expected.rfind("<=", 0) == 0) {
        int expected_value = 0;
        int actual_value = 0;
        return TryParseInt(expected.substr(2), expected_value) &&
               TryParseInt(actual, actual_value) &&
               actual_value <= expected_value;
    }

    if (expected.rfind(">", 0) == 0) {
        int expected_value = 0;
        int actual_value = 0;
        return TryParseInt(expected.substr(1), expected_value) &&
               TryParseInt(actual, actual_value) &&
               actual_value > expected_value;
    }

    if (expected.rfind("<", 0) == 0) {
        int expected_value = 0;
        int actual_value = 0;
        return TryParseInt(expected.substr(1), expected_value) &&
               TryParseInt(actual, actual_value) &&
               actual_value < expected_value;
    }

    if (expected.rfind("==", 0) == 0) {
        return actual == expected.substr(2);
    }

    return actual == expected;
}

} // namespace

namespace strategy {

GameRuleManager::GameRuleManager() {
    InitializeDefaultRules();
}

GameRules GameRuleManager::GetRulesByCategory(const std::string& category) {
    GameRules result;
    result.category = category;
    result.version = 1;

    for (const auto& pair : rules_) {
        if ((category.empty() || category == "all" || pair.second.category == category) && pair.second.is_active) {
            result.rules.push_back(pair.second);
        }
    }

    std::sort(result.rules.begin(), result.rules.end(), [](const GameRule& a, const GameRule& b) {
        return a.priority > b.priority;
    });

    return result;
}

bool GameRuleManager::CheckRuleCondition(
    const std::string& rule_id,
    const std::unordered_map<std::string, std::string>& context) {
    auto it = rules_.find(rule_id);
    if (it == rules_.end() || !it->second.is_active) {
        return false;
    }

    const GameRule& rule = it->second;

    for (const auto& condition : rule.conditions) {
        auto context_it = context.find(condition.first);
        if (context_it == context.end()) {
            return false;
        }

        if (!EvaluateCondition(condition.second, context_it->second)) {
            return false;
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
    GameRule combat_start_rule;
    combat_start_rule.rule_id = "combat_start";
    combat_start_rule.description = "Combat start condition";
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
    GameRule story_trigger_rule;
    story_trigger_rule.rule_id = "story_chapter_1";
    story_trigger_rule.description = "Chapter 1 story trigger";
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
    GameRule quest_complete_rule;
    quest_complete_rule.rule_id = "quest_kill_monsters";
    quest_complete_rule.description = "Kill monsters quest completed";
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
