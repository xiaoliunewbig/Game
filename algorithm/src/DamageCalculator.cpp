/*
 * 文件名: DamageCalculator.cpp
 * 说明: 完整的伤害计算系统实现
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 * 更新时间: 2025-07-24 — 重写伤害公式
 *
 * 伤害公式:
 *   base_damage = stat_value * skill_multiplier * level_bonus
 *   element_mult = ElementMatrix[atk_elem][def_elem]
 *   defense_reduction = defense / (defense + 100)
 *   final = base * element_mult * (1 - defense_reduction) * crit * attack_buff * variance
 */
#include "DamageCalculator.h"
#include "SkillTreeManager.h"
#include <random>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace algorithm {

// 线程局部随机数生成器
static thread_local std::mt19937 s_rng{std::random_device{}()};

DamageCalculator::DamageCalculator()
    : skill_manager_(std::make_unique<SkillTreeManager>()) {
}

DamageCalculator::~DamageCalculator() = default;

// ============================================================================
// 兼容旧接口: 通过角色ID查找属性
// ============================================================================
DamageResult DamageCalculator::CalculateDamage(const DamageRequest& request) {
    DamageResult result;

    // 如果有属性注册表，使用完整计算
    if (stats_registry_) {
        const CharacterStats* atk = stats_registry_->getCharacterStats(request.attacker_id);
        const CharacterStats* def = stats_registry_->getCharacterStats(request.defender_id);

        if (atk && def) {
            ExtendedDamageRequest ext;
            ext.attacker = *atk;
            ext.defender = *def;
            ext.skill_id = request.skill_id;
            ext.skill_level = 1;
            auto ext_result = CalculateExtendedDamage(ext);
            result.damage = ext_result.damage;
            result.effect = ext_result.effect;
            return result;
        }
    }

    // 兜底: 无注册表时使用简化计算
    const SkillNode* skill = skill_manager_->GetSkill(request.skill_id);
    float multiplier = skill ? skill->damage_multiplier : 1.0f;
    float base = 50.0f * multiplier;  // 默认基础攻击力50

    std::uniform_real_distribution<float> var_dist(0.95f, 1.05f);
    base *= var_dist(s_rng);

    result.damage = static_cast<int>(std::max(1.0f, base));

    std::stringstream desc;
    desc << "造成 " << result.damage << " 点伤害";
    if (skill) {
        desc << " [" << skill->name << "]";
    }
    result.effect = desc.str();
    return result;
}

// ============================================================================
// 核心: 完整伤害计算
// ============================================================================
ExtendedDamageResult DamageCalculator::CalculateExtendedDamage(const ExtendedDamageRequest& request) {
    ExtendedDamageResult result;

    const SkillNode* skill = skill_manager_->GetSkill(request.skill_id);
    if (!skill) {
        result.damage = 0;
        result.effect = "无效技能";
        return result;
    }

    // 0伤害技能（防御、治愈等）
    if (skill->damage_multiplier <= 0.0f) {
        result.damage = 0;
        result.damage_type = skill->damage_type;
        result.applied_effects = DetermineAppliedEffects(skill);
        result.effect = skill->name + " (非伤害技能)";
        return result;
    }

    DamageType dmg_type = skill->damage_type;
    result.damage_type = dmg_type;

    // 步骤1: 基础伤害 = 属性值 × 技能倍率 × 等级加成
    float base_damage = CalculateBaseDamage(request.attacker, skill,
                                            request.skill_level, dmg_type);

    // 步骤2: 元素克制
    Element atk_elem = (skill->element != Element::None) ? skill->element : request.attacker.element;
    float element_mult = ApplyElementMultiplier(atk_elem, request.defender.element);
    result.element_multiplier = element_mult;

    // 步骤3: 防御减免
    float defense_mult = 1.0f;
    if (dmg_type != DamageType::Pure) {
        float defense_reduction = ApplyDefenseReduction(request.defender, dmg_type,
                                                        request.defender_effects);
        defense_mult = 1.0f - defense_reduction;
    }

    // 步骤4: 暴击
    bool is_crit = RollCritical(request.attacker);
    float crit_mult = is_crit ? GetCriticalMultiplier(request.attacker) : 1.0f;
    result.is_critical = is_crit;

    // 步骤5: 攻击buff
    float attack_buff = GetAttackBuffMultiplier(request.attacker_effects);

    // 步骤6: 随机浮动 (±5%)
    float variance = GetRandomVariance();

    // 最终伤害
    float final_damage = base_damage * element_mult * defense_mult
                         * crit_mult * attack_buff * variance;

    // 步骤7: 护盾吸收
    float shield = ApplyShieldAbsorption(final_damage, request.defender_effects);
    result.shield_absorbed = shield;
    final_damage -= shield;

    // 最低1点伤害
    result.damage = static_cast<int>(std::max(1.0f, std::round(final_damage)));

    // 附加状态效果
    result.applied_effects = DetermineAppliedEffects(skill);

    // 生成描述
    result.effect = GenerateEffectDescription(result, skill);

    return result;
}

// ============================================================================
// 基础伤害计算
// ============================================================================
float DamageCalculator::CalculateBaseDamage(const CharacterStats& attacker,
                                            const SkillNode* skill,
                                            int skill_level,
                                            DamageType damage_type) const {
    float stat_value;
    switch (damage_type) {
        case DamageType::Physical:
            stat_value = attacker.attack;
            break;
        case DamageType::Magical:
            stat_value = attacker.magic_attack;
            break;
        case DamageType::Pure:
            stat_value = std::max(attacker.attack, attacker.magic_attack);
            break;
        default:
            stat_value = attacker.attack;
            break;
    }

    float scaled_multiplier = skill_manager_->GetScaledMultiplier(skill->skill_id, skill_level);
    return stat_value * scaled_multiplier;
}

// ============================================================================
// 元素克制
// ============================================================================
float DamageCalculator::ApplyElementMultiplier(Element attacker_elem, Element defender_elem) const {
    if (stats_registry_) {
        return stats_registry_->getElementMultiplier(attacker_elem, defender_elem);
    }

    // 无注册表时的简化克制
    if (attacker_elem == Element::None || defender_elem == Element::None) {
        return 1.0f;
    }
    if (attacker_elem == defender_elem) {
        return 0.75f;
    }
    return 1.0f;
}

// ============================================================================
// 防御减免: defense / (defense + 100)
// ============================================================================
float DamageCalculator::ApplyDefenseReduction(const CharacterStats& defender,
                                               DamageType damage_type,
                                               const std::vector<StatusEffect>& defender_effects) const {
    float defense_stat;
    if (damage_type == DamageType::Physical) {
        defense_stat = defender.defense;
    } else {
        defense_stat = defender.magic_defense;
    }

    // 应用破防debuff
    float def_mult = StatusEffectHelper::getTotalDefenseMultiplier(defender_effects);
    defense_stat *= def_mult;

    // 防御不低于0
    defense_stat = std::max(0.0f, defense_stat);

    // 递减收益公式
    return defense_stat / (defense_stat + 100.0f);
}

// ============================================================================
// 暴击判定
// ============================================================================
bool DamageCalculator::RollCritical(const CharacterStats& attacker) const {
    // 暴击率 = 基础暴击率 + 幸运值加成
    float crit_rate = attacker.crit_rate + attacker.luck * 0.003f;
    crit_rate = std::min(0.75f, crit_rate);  // 上限75%

    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(s_rng) < crit_rate;
}

float DamageCalculator::GetCriticalMultiplier(const CharacterStats& attacker) const {
    return 1.0f + attacker.crit_damage;
}

// ============================================================================
// 攻击buff累积
// ============================================================================
float DamageCalculator::GetAttackBuffMultiplier(const std::vector<StatusEffect>& effects) const {
    return StatusEffectHelper::getTotalAttackMultiplier(effects);
}

// ============================================================================
// 随机浮动 ±5%
// ============================================================================
float DamageCalculator::GetRandomVariance() const {
    std::uniform_real_distribution<float> dist(0.95f, 1.05f);
    return dist(s_rng);
}

// ============================================================================
// 护盾吸收
// ============================================================================
float DamageCalculator::ApplyShieldAbsorption(float damage,
                                               const std::vector<StatusEffect>& defender_effects) const {
    float shield = StatusEffectHelper::getTotalShieldAmount(defender_effects);
    return std::min(shield, damage);
}

// ============================================================================
// 根据技能元素附加状态效果
// ============================================================================
std::vector<StatusEffect> DamageCalculator::DetermineAppliedEffects(const SkillNode* skill) const {
    std::vector<StatusEffect> effects;
    if (!skill || skill->damage_multiplier <= 0.0f) return effects;

    // 30%概率附加元素效果
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    if (dist(s_rng) > 0.3f) return effects;

    switch (skill->element) {
        case Element::Fire:
            effects.push_back(StatusEffectHelper::createBurn(
                skill->damage_multiplier * 5.0f, 3));
            break;
        case Element::Water:
            effects.push_back(StatusEffectHelper::createFreeze(0.3f, 2));
            break;
        case Element::Grass:
            effects.push_back(StatusEffectHelper::createPoison(0.05f, 4));
            break;
        case Element::Dark:
            effects.push_back(StatusEffectHelper::createDefenseDown(0.2f, 3));
            break;
        default:
            break;
    }

    return effects;
}

// ============================================================================
// 效果描述文本生成
// ============================================================================
std::string DamageCalculator::GenerateEffectDescription(const ExtendedDamageResult& result,
                                                         const SkillNode* skill) const {
    std::stringstream desc;
    desc << "使用 [" << (skill ? skill->name : "未知技能") << "] ";
    desc << "造成 " << result.damage << " 点";

    switch (result.damage_type) {
        case DamageType::Physical: desc << "物理"; break;
        case DamageType::Magical:  desc << "魔法"; break;
        case DamageType::Pure:     desc << "纯粹"; break;
    }
    desc << "伤害";

    if (result.is_critical) {
        desc << " (暴击!)";
    }

    if (result.element_multiplier > 1.1f) {
        desc << " (克制效果 x" << std::fixed;
        desc.precision(1);
        desc << result.element_multiplier << ")";
    } else if (result.element_multiplier < 0.9f) {
        desc << " (抗性减免)";
    }

    if (result.shield_absorbed > 0) {
        desc << " (护盾吸收 " << static_cast<int>(result.shield_absorbed) << ")";
    }

    for (const auto& eff : result.applied_effects) {
        desc << " [附加: " << eff.name << "]";
    }

    return desc.str();
}

} // namespace algorithm
