/*
 * 文件名: DamageCalculator.h
 * 说明: 伤害计算器类定义，支持属性驱动的完整伤害公式
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 * 更新时间: 2025-07-24 — 重写为属性/元素/防御/暴击/状态效果驱动的伤害系统
 */

#ifndef ALGORITHM_DAMAGE_CALCULATOR_H
#define ALGORITHM_DAMAGE_CALCULATOR_H

#include "../IAlgorithmService.h"
#include "CharacterStats.h"
#include "StatusEffect.h"
#include <memory>
#include <vector>

namespace algorithm {

class SkillTreeManager;
struct SkillNode;

/**
 * @brief 扩展伤害计算请求（内部使用）
 */
struct ExtendedDamageRequest {
    CharacterStats attacker;
    CharacterStats defender;
    int skill_id = 1;
    int skill_level = 1;
    std::vector<StatusEffect> attacker_effects;  // 攻击者身上的状态效果
    std::vector<StatusEffect> defender_effects;   // 防御者身上的状态效果
};

/**
 * @brief 扩展伤害结果（内部使用）
 */
struct ExtendedDamageResult {
    int damage = 0;
    std::string effect;
    bool is_critical = false;
    float element_multiplier = 1.0f;
    DamageType damage_type = DamageType::Physical;
    float shield_absorbed = 0.0f;      // 被护盾吸收的伤害
    std::vector<StatusEffect> applied_effects;  // 本次攻击附加的状态效果
};

/**
 * @brief 伤害计算器
 * 基于角色属性、技能、元素克制、暴击和状态效果的完整伤害计算系统
 */
class DamageCalculator {
public:
    DamageCalculator();
    ~DamageCalculator();

    /**
     * @brief 计算伤害（兼容旧接口）
     * @param request 伤害计算请求
     * @return 伤害计算结果
     */
    DamageResult CalculateDamage(const DamageRequest& request);

    /**
     * @brief 计算伤害（扩展版本）
     * @param request 扩展伤害请求
     * @return 扩展伤害结果
     */
    ExtendedDamageResult CalculateExtendedDamage(const ExtendedDamageRequest& request);

    /**
     * @brief 设置角色属性注册表引用
     */
    void setStatsRegistry(CharacterStatsRegistry* registry) { stats_registry_ = registry; }

    /**
     * @brief 获取技能管理器
     */
    SkillTreeManager* getSkillManager() { return skill_manager_.get(); }

private:
    std::unique_ptr<SkillTreeManager> skill_manager_;
    CharacterStatsRegistry* stats_registry_ = nullptr;  // 不拥有

    // 核心计算步骤
    float CalculateBaseDamage(const CharacterStats& attacker, const SkillNode* skill,
                              int skill_level, DamageType damage_type) const;
    float ApplyElementMultiplier(Element attacker_elem, Element defender_elem) const;
    float ApplyDefenseReduction(const CharacterStats& defender, DamageType damage_type,
                                const std::vector<StatusEffect>& defender_effects) const;
    bool RollCritical(const CharacterStats& attacker) const;
    float GetCriticalMultiplier(const CharacterStats& attacker) const;
    float GetAttackBuffMultiplier(const std::vector<StatusEffect>& effects) const;
    float GetRandomVariance() const;
    float ApplyShieldAbsorption(float damage, const std::vector<StatusEffect>& defender_effects) const;
    std::vector<StatusEffect> DetermineAppliedEffects(const SkillNode* skill) const;
    std::string GenerateEffectDescription(const ExtendedDamageResult& result,
                                          const SkillNode* skill) const;
};

} // namespace algorithm

#endif // ALGORITHM_DAMAGE_CALCULATOR_H
