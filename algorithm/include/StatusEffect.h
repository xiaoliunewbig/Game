/*
 * 文件名: StatusEffect.h
 * 说明: 状态效果系统定义（头文件 only）
 * 作者: 彭承康
 * 创建时间: 2025-07-24
 */

#ifndef ALGORITHM_STATUS_EFFECT_H
#define ALGORITHM_STATUS_EFFECT_H

#include "CharacterStats.h"
#include <vector>
#include <string>
#include <cmath>

namespace algorithm {

/**
 * @brief 状态效果类型枚举
 */
enum class StatusEffectType {
    None = 0,
    Burn = 1,          // 灼烧：每回合火元素伤害
    Freeze = 2,        // 冰冻：降低速度，概率跳过回合
    Poison = 3,        // 中毒：每回合百分比伤害
    Bleed = 4,         // 流血：每回合固定伤害
    Stun = 5,          // 眩晕：跳过回合
    DefenseDown = 6,   // 破防：降低防御力
    AttackUp = 7,      // 强化：提升攻击力
    Shield = 8,        // 护盾：吸收伤害
    Regeneration = 9   // 回复：每回合恢复HP
};

/**
 * @brief 状态效果结构体
 */
struct StatusEffect {
    StatusEffectType type = StatusEffectType::None;
    int duration_turns = 0;      ///< 持续回合数
    float magnitude = 0.0f;      ///< 效果强度 (百分比或固定值)
    Element source_element = Element::None;  ///< 来源元素
    std::string name;            ///< 效果名称

    /**
     * @brief 获取攻击力加成倍率
     * @return 1.0f 表示无加成
     */
    float getAttackMultiplier() const {
        if (type == StatusEffectType::AttackUp) {
            return 1.0f + magnitude;  // magnitude=0.3 → 1.3x
        }
        return 1.0f;
    }

    /**
     * @brief 获取防御力修正倍率
     * @return 1.0f 表示无修正
     */
    float getDefenseMultiplier() const {
        if (type == StatusEffectType::DefenseDown) {
            return 1.0f - magnitude;  // magnitude=0.3 → 0.7x (防御降低30%)
        }
        return 1.0f;
    }

    /**
     * @brief 获取每回合持续伤害
     * @param target_max_hp 目标最大HP（用于百分比伤害）
     * @return 伤害值
     */
    float getDamageOverTime(float target_max_hp) const {
        switch (type) {
            case StatusEffectType::Burn:
                return magnitude;  // 固定火伤
            case StatusEffectType::Poison:
                return target_max_hp * magnitude;  // 百分比毒伤
            case StatusEffectType::Bleed:
                return magnitude;  // 固定流血伤害
            default:
                return 0.0f;
        }
    }

    /**
     * @brief 获取每回合恢复量
     * @param target_max_hp 目标最大HP
     * @return 恢复值
     */
    float getHealOverTime(float target_max_hp) const {
        if (type == StatusEffectType::Regeneration) {
            return target_max_hp * magnitude;  // 百分比回复
        }
        return 0.0f;
    }

    /**
     * @brief 获取护盾吸收量
     * @return 剩余护盾值
     */
    float getShieldAmount() const {
        if (type == StatusEffectType::Shield) {
            return magnitude;  // 护盾剩余值
        }
        return 0.0f;
    }

    /**
     * @brief 是否导致无法行动
     */
    bool preventsAction() const {
        return type == StatusEffectType::Stun ||
               type == StatusEffectType::Freeze;
    }

    /**
     * @brief 速度修正倍率
     */
    float getSpeedMultiplier() const {
        if (type == StatusEffectType::Freeze) {
            return 1.0f - magnitude;  // magnitude=0.5 → 速度减半
        }
        return 1.0f;
    }

    /**
     * @brief 回合结束时更新，返回是否仍然有效
     */
    bool tick() {
        if (duration_turns > 0) {
            duration_turns--;
        }
        return duration_turns > 0;
    }
};

/**
 * @brief 状态效果管理辅助函数
 */
struct StatusEffectHelper {
    /**
     * @brief 计算所有攻击buff的累积倍率
     */
    static float getTotalAttackMultiplier(const std::vector<StatusEffect>& effects) {
        float mult = 1.0f;
        for (const auto& e : effects) {
            mult *= e.getAttackMultiplier();
        }
        return mult;
    }

    /**
     * @brief 计算所有防御debuff的累积倍率
     */
    static float getTotalDefenseMultiplier(const std::vector<StatusEffect>& effects) {
        float mult = 1.0f;
        for (const auto& e : effects) {
            mult *= e.getDefenseMultiplier();
        }
        return mult;
    }

    /**
     * @brief 计算总持续伤害
     */
    static float getTotalDamageOverTime(const std::vector<StatusEffect>& effects, float max_hp) {
        float total = 0.0f;
        for (const auto& e : effects) {
            total += e.getDamageOverTime(max_hp);
        }
        return total;
    }

    /**
     * @brief 计算总护盾值
     */
    static float getTotalShieldAmount(const std::vector<StatusEffect>& effects) {
        float total = 0.0f;
        for (const auto& e : effects) {
            total += e.getShieldAmount();
        }
        return total;
    }

    /**
     * @brief 检查是否被控制（无法行动）
     */
    static bool isControlled(const std::vector<StatusEffect>& effects) {
        for (const auto& e : effects) {
            if (e.preventsAction()) return true;
        }
        return false;
    }

    /**
     * @brief 创建灼烧效果
     */
    static StatusEffect createBurn(float damage_per_turn, int duration) {
        return {StatusEffectType::Burn, duration, damage_per_turn, Element::Fire, "灼烧"};
    }

    /**
     * @brief 创建冰冻效果
     */
    static StatusEffect createFreeze(float slow_percent, int duration) {
        return {StatusEffectType::Freeze, duration, slow_percent, Element::Water, "冰冻"};
    }

    /**
     * @brief 创建中毒效果
     */
    static StatusEffect createPoison(float hp_percent_per_turn, int duration) {
        return {StatusEffectType::Poison, duration, hp_percent_per_turn, Element::Grass, "中毒"};
    }

    /**
     * @brief 创建流血效果
     */
    static StatusEffect createBleed(float damage_per_turn, int duration) {
        return {StatusEffectType::Bleed, duration, damage_per_turn, Element::None, "流血"};
    }

    /**
     * @brief 创建眩晕效果
     */
    static StatusEffect createStun(int duration) {
        return {StatusEffectType::Stun, duration, 1.0f, Element::None, "眩晕"};
    }

    /**
     * @brief 创建破防效果
     */
    static StatusEffect createDefenseDown(float percent, int duration) {
        return {StatusEffectType::DefenseDown, duration, percent, Element::None, "破防"};
    }

    /**
     * @brief 创建攻击强化效果
     */
    static StatusEffect createAttackUp(float percent, int duration) {
        return {StatusEffectType::AttackUp, duration, percent, Element::None, "攻击强化"};
    }

    /**
     * @brief 创建护盾效果
     */
    static StatusEffect createShield(float amount, int duration) {
        return {StatusEffectType::Shield, duration, amount, Element::None, "护盾"};
    }

    /**
     * @brief 创建回复效果
     */
    static StatusEffect createRegeneration(float hp_percent, int duration) {
        return {StatusEffectType::Regeneration, duration, hp_percent, Element::None, "回复"};
    }
};

} // namespace algorithm

#endif // ALGORITHM_STATUS_EFFECT_H
