/*
 * 文件名: SkillTreeManager.cpp
 * 说明: 技能树管理器实现
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 */

#include "SkillTreeManager.h"
#include <algorithm>

namespace algorithm {

SkillTreeManager::SkillTreeManager() {
    InitializeDefaultSkills();
}

const SkillNode* SkillTreeManager::GetSkill(int skill_id) const {
    auto it = skills_.find(skill_id);
    return it != skills_.end() ? &it->second : nullptr;
}

void SkillTreeManager::AddSkill(const SkillNode& skill) {
    skills_[skill.skill_id] = skill;
}

bool SkillTreeManager::CanLearnSkill(int skill_id, const std::vector<int>& learned_skills) const {
    const SkillNode* skill = GetSkill(skill_id);
    if (!skill) {
        return false;
    }

    // 检查前置技能是否都已学习
    for (int prerequisite : skill->prerequisites) {
        if (std::find(learned_skills.begin(), learned_skills.end(), prerequisite) == learned_skills.end()) {
            return false;
        }
    }

    return true;
}

void SkillTreeManager::InitializeDefaultSkills() {
    // 基础攻击
    SkillNode basic_attack;
    basic_attack.skill_id = 1;
    basic_attack.name = "基础攻击";
    basic_attack.description = "普通的物理攻击";
    basic_attack.damage_multiplier = 1.0f;
    basic_attack.mana_cost = 0;
    basic_attack.cooldown = 0;
    AddSkill(basic_attack);

    // 防御
    SkillNode defend;
    defend.skill_id = 2;
    defend.name = "防御";
    defend.description = "提高防御力，减少受到的伤害";
    defend.damage_multiplier = 0.0f;
    defend.mana_cost = 0;
    defend.cooldown = 0;
    AddSkill(defend);

    // 火球术
    SkillNode fireball;
    fireball.skill_id = 3;
    fireball.name = "火球术";
    fireball.description = "发射火球攻击敌人";
    fireball.damage_multiplier = 1.5f;
    fireball.mana_cost = 10;
    fireball.cooldown = 3;
    AddSkill(fireball);

    // 移动
    SkillNode move;
    move.skill_id = 4;
    move.name = "移动";
    move.description = "移动到新位置";
    move.damage_multiplier = 0.0f;
    move.mana_cost = 0;
    move.cooldown = 0;
    AddSkill(move);
}

} // namespace algorithm
