/*
 * File: GameRuleManager.h
 * Description: Game rule management interface.
 */
#ifndef STRATEGY_GAMERULEMANAGER_H
#define STRATEGY_GAMERULEMANAGER_H

#include <string>
#include <unordered_map>
#include <vector>

namespace strategy {

/**
 * @brief Single game rule definition.
 */
struct GameRule {
    std::string rule_id;
    std::string description;
    std::string category;  // combat, story, quest
    std::unordered_map<std::string, std::string> conditions;
    std::unordered_map<std::string, std::string> effects;
    int priority;
    bool is_active;
};

/**
 * @brief Group of rules returned to callers.
 */
struct GameRules {
    std::vector<GameRule> rules;
    std::string category;
    int version;
};

/**
 * @brief Manages built-in and dynamic game rules.
 */
class GameRuleManager {
public:
    GameRuleManager();

    GameRules GetRulesByCategory(const std::string& category);

    bool CheckRuleCondition(
        const std::string& rule_id,
        const std::unordered_map<std::string, std::string>& context);

    std::unordered_map<std::string, std::string> ApplyRuleEffect(const std::string& rule_id);

    bool AddRule(const GameRule& rule);

    bool UpdateRuleStatus(const std::string& rule_id, bool is_active);

private:
    std::unordered_map<std::string, GameRule> rules_;

    void InitializeDefaultRules();
    void InitializeCombatRules();
    void InitializeStoryRules();
    void InitializeQuestRules();
};

} // namespace strategy

#endif // STRATEGY_GAMERULEMANAGER_H
