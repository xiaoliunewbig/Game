/*
 * 文件名: GameRuleManager.h
 * 说明: 游戏规则管理器，负责战斗规则、剧情触发条件等规则管理。
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 *
 * 本类管理游戏中的各种规则，包括战斗规则、剧情触发条件、任务规则等，
 * 为世界状态引擎和事件调度器提供规则支持。
 */
#ifndef STRATEGY_GAMERULEMANAGER_H
#define STRATEGY_GAMERULEMANAGER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace strategy {

/**
 * @brief 游戏规则结构
 */
struct GameRule {
    std::string rule_id;
    std::string description;
    std::string category;  // "combat", "story", "quest"
    std::unordered_map<std::string, std::string> conditions;
    std::unordered_map<std::string, std::string> effects;
    int priority;
    bool is_active;
};

/**
 * @brief 游戏规则集合
 */
struct GameRules {
    std::vector<GameRule> rules;
    std::string category;
    int version;
};

/**
 * @brief 游戏规则管理器
 */
class GameRuleManager {
public:
    GameRuleManager();

    /**
     * @brief 获取指定类别的规则
     */
    GameRules GetRulesByCategory(const std::string& category);

    /**
     * @brief 检查规则条件是否满足
     */
    bool CheckRuleCondition(const std::string& rule_id, const std::unordered_map<std::string, std::string>& context);

    /**
     * @brief 应用规则效果
     */
    std::unordered_map<std::string, std::string> ApplyRuleEffect(const std::string& rule_id);

    /**
     * @brief 添加自定义规则
     */
    bool AddRule(const GameRule& rule);

    /**
     * @brief 更新规则状态
     */
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