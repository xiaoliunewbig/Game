/*
 * 文件名: InputValidator.cpp
 * 说明: 输入数据校验模块实现
 * 作者: 彭承康
 * 创建时间: 2025-07-24
 */

#include "InputValidator.h"
#include "SkillTreeManager.h"
#include <sstream>

namespace algorithm {

InputValidator::InputValidator() = default;

ValidationResult InputValidator::validateDamageRequest(
    const DamageRequest& request,
    const CharacterStatsRegistry& registry,
    const SkillTreeManager& skill_manager) const
{
    // 校验攻击者ID
    if (request.attacker_id < 0 || request.attacker_id > MAX_CHARACTER_ID) {
        return ValidationResult::fail(
            "攻击者ID无效: " + std::to_string(request.attacker_id));
    }

    // 校验防御者ID
    if (request.defender_id < 0 || request.defender_id > MAX_CHARACTER_ID) {
        return ValidationResult::fail(
            "防御者ID无效: " + std::to_string(request.defender_id));
    }

    // 校验攻击者和防御者不同
    if (request.attacker_id == request.defender_id) {
        return ValidationResult::fail("攻击者和防御者不能是同一角色");
    }

    // 校验技能ID
    if (request.skill_id < 0 || request.skill_id > MAX_SKILL_ID) {
        return ValidationResult::fail(
            "技能ID无效: " + std::to_string(request.skill_id));
    }

    // 校验技能是否存在
    if (skill_manager.GetSkill(request.skill_id) == nullptr) {
        return ValidationResult::fail(
            "技能不存在: " + std::to_string(request.skill_id));
    }

    // 校验攻击者是否已注册
    if (registry.getCharacterStats(request.attacker_id) == nullptr) {
        return ValidationResult::fail(
            "攻击者未注册: " + std::to_string(request.attacker_id));
    }

    // 校验防御者是否已注册
    if (registry.getCharacterStats(request.defender_id) == nullptr) {
        return ValidationResult::fail(
            "防御者未注册: " + std::to_string(request.defender_id));
    }

    return ValidationResult::ok();
}

ValidationResult InputValidator::validateAIDecisionRequest(const AIDecisionRequest& request) const {
    // 校验NPC ID
    if (request.npc_id < 0 || request.npc_id > MAX_NPC_ID) {
        return ValidationResult::fail(
            "NPC ID无效: " + std::to_string(request.npc_id));
    }

    // 校验上下文最小长度
    if (static_cast<int>(request.context.size()) < MIN_CONTEXT_SIZE) {
        return ValidationResult::fail(
            "上下文信息不足，需要至少" + std::to_string(MIN_CONTEXT_SIZE) + "个元素");
    }

    // context[0] = 距离，应为非负
    if (request.context[0] < 0) {
        return ValidationResult::fail("距离值不能为负");
    }

    // context[1] = 血量百分比，应在 0-100
    if (request.context[1] < 0 || request.context[1] > 100) {
        return ValidationResult::fail(
            "血量百分比应在0-100之间，当前: " + std::to_string(request.context[1]));
    }

    return ValidationResult::ok();
}

ValidationResult InputValidator::validateSkillLearnRequest(
    int skill_id,
    Profession character_profession,
    const std::vector<int>& learned_skills,
    const SkillTreeManager& skill_manager) const
{
    // 校验技能ID
    if (skill_id < 0 || skill_id > MAX_SKILL_ID) {
        return ValidationResult::fail(
            "技能ID无效: " + std::to_string(skill_id));
    }

    // 校验技能是否存在
    const SkillNode* skill = skill_manager.GetSkill(skill_id);
    if (!skill) {
        return ValidationResult::fail(
            "技能不存在: " + std::to_string(skill_id));
    }

    // 校验职业限制
    if (skill->profession != Profession::None &&
        skill->profession != character_profession) {
        return ValidationResult::fail(
            "职业不匹配，无法学习技能: " + skill->name);
    }

    // 校验前置技能
    if (!skill_manager.CanLearnSkill(skill_id, learned_skills)) {
        return ValidationResult::fail(
            "前置技能未满足，无法学习: " + skill->name);
    }

    // 校验是否已学习
    for (int sid : learned_skills) {
        if (sid == skill_id) {
            return ValidationResult::fail(
                "技能已学习: " + skill->name);
        }
    }

    return ValidationResult::ok();
}

ValidationResult InputValidator::validateCharacterStats(const CharacterStats& stats) const {
    // 校验ID
    if (stats.character_id < 0 || stats.character_id > MAX_CHARACTER_ID) {
        return ValidationResult::fail(
            "角色ID无效: " + std::to_string(stats.character_id));
    }

    // 校验HP
    if (stats.max_hp <= 0 || stats.max_hp > MAX_STAT_VALUE) {
        return ValidationResult::fail("最大HP值异常");
    }
    if (stats.current_hp < 0 || stats.current_hp > stats.max_hp) {
        return ValidationResult::fail("当前HP值异常");
    }

    // 校验MP
    if (stats.max_mp < 0 || stats.max_mp > MAX_STAT_VALUE) {
        return ValidationResult::fail("最大MP值异常");
    }

    // 校验攻击/防御
    if (stats.attack < 0 || stats.attack > MAX_STAT_VALUE) {
        return ValidationResult::fail("攻击力值异常");
    }
    if (stats.defense < 0 || stats.defense > MAX_STAT_VALUE) {
        return ValidationResult::fail("防御力值异常");
    }
    if (stats.magic_attack < 0 || stats.magic_attack > MAX_STAT_VALUE) {
        return ValidationResult::fail("魔法攻击力值异常");
    }
    if (stats.magic_defense < 0 || stats.magic_defense > MAX_STAT_VALUE) {
        return ValidationResult::fail("魔法防御力值异常");
    }

    // 校验暴击率 (0-1)
    if (stats.crit_rate < 0.0f || stats.crit_rate > 1.0f) {
        return ValidationResult::fail("暴击率应在0.0-1.0之间");
    }

    // 校验暴击伤害 (>=0)
    if (stats.crit_damage < 0.0f) {
        return ValidationResult::fail("暴击伤害加成不能为负");
    }

    return ValidationResult::ok();
}

} // namespace algorithm
