/*
 * 文件名: SkillTreeManager.cpp
 * 说明: 技能树管理器实现，包含33+技能（通用3 + 战士10 + 法师10 + 弓箭手10）
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 * 更新时间: 2025-07-24 — 全面扩展技能体系
 */

#include "SkillTreeManager.h"
#include <algorithm>
#include <cmath>

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
    if (!skill) return false;

    for (int prereq : skill->prerequisites) {
        if (std::find(learned_skills.begin(), learned_skills.end(), prereq) == learned_skills.end()) {
            return false;
        }
    }
    return true;
}

std::vector<const SkillNode*> SkillTreeManager::GetSkillsByProfession(Profession profession) const {
    std::vector<const SkillNode*> result;
    for (const auto& [id, skill] : skills_) {
        if (skill.profession == profession || skill.profession == Profession::None) {
            result.push_back(&skill);
        }
    }
    return result;
}

std::vector<const SkillNode*> SkillTreeManager::GetAllSkills() const {
    std::vector<const SkillNode*> result;
    for (const auto& [id, skill] : skills_) {
        result.push_back(&skill);
    }
    return result;
}

float SkillTreeManager::GetScaledMultiplier(int skill_id, int skill_level) const {
    const SkillNode* skill = GetSkill(skill_id);
    if (!skill) return 1.0f;

    int level = std::clamp(skill_level, 1, skill->max_level);
    // 每级+10%倍率
    float bonus = 1.0f + (level - 1) * 0.10f;
    return skill->damage_multiplier * bonus;
}

int SkillTreeManager::GetScaledCooldown(int skill_id, int skill_level) const {
    const SkillNode* skill = GetSkill(skill_id);
    if (!skill) return 0;

    int level = std::clamp(skill_level, 1, skill->max_level);
    // 每级-5%冷却
    float reduction = 1.0f - (level - 1) * 0.05f;
    reduction = std::max(0.5f, reduction);  // 最多减50%
    return static_cast<int>(skill->cooldown_ms * reduction);
}

void SkillTreeManager::InitializeDefaultSkills() {
    InitializeCommonSkills();
    InitializeWarriorSkills();
    InitializeMageSkills();
    InitializeArcherSkills();
}

// ============================================================================
// 通用技能 (ID: 1-99)
// ============================================================================
void SkillTreeManager::InitializeCommonSkills() {
    // 1 - 基础攻击
    AddSkill({1, "基础攻击", "普通的物理攻击",
              Profession::None, Element::None, DamageType::Physical,
              1.0f, 0, 0, 5, {}, false});

    // 2 - 防御
    AddSkill({2, "防御", "进入防御姿态，降低受到的伤害",
              Profession::None, Element::None, DamageType::Physical,
              0.0f, 0, 0, 5, {}, false});

    // 3 - 闪避
    AddSkill({3, "闪避", "快速移动躲避攻击",
              Profession::None, Element::None, DamageType::Physical,
              0.0f, 0, 2000, 3, {}, false});
}

// ============================================================================
// 战士技能 (ID: 100-199)
// ============================================================================
void SkillTreeManager::InitializeWarriorSkills() {
    // 100 - 强力斩
    AddSkill({100, "强力斩", "蓄力后发出强力的一击",
              Profession::Warrior, Element::None, DamageType::Physical,
              1.8f, 5, 3000, 5, {1}, false});

    // 101 - 盾击
    AddSkill({101, "盾击", "用盾牌猛击敌人，有概率眩晕",
              Profession::Warrior, Element::None, DamageType::Physical,
              1.2f, 8, 4000, 5, {2}, false});

    // 102 - 冲锋
    AddSkill({102, "冲锋", "向敌人冲锋，缩短距离并造成伤害",
              Profession::Warrior, Element::None, DamageType::Physical,
              1.5f, 10, 5000, 5, {100}, false});

    // 103 - 旋风斩
    AddSkill({103, "旋风斩", "旋转武器攻击周围所有敌人",
              Profession::Warrior, Element::None, DamageType::Physical,
              1.3f, 15, 6000, 5, {100}, false});

    // 104 - 战吼
    AddSkill({104, "战吼", "提升自身攻击力",
              Profession::Warrior, Element::None, DamageType::Physical,
              0.0f, 12, 8000, 5, {101}, false});

    // 105 - 格挡
    AddSkill({105, "格挡", "完美格挡下一次攻击",
              Profession::Warrior, Element::None, DamageType::Physical,
              0.0f, 5, 3000, 5, {2}, false});

    // 106 - 重击
    AddSkill({106, "重击", "全力一击，高伤害低速度",
              Profession::Warrior, Element::None, DamageType::Physical,
              2.5f, 20, 8000, 5, {102, 103}, false});

    // 107 - 狂暴
    AddSkill({107, "狂暴", "进入狂暴状态，大幅提升攻击但降低防御",
              Profession::Warrior, Element::None, DamageType::Physical,
              0.0f, 25, 15000, 5, {104}, false});

    // 108 - 钢铁堡垒
    AddSkill({108, "钢铁堡垒", "极大提升防御力，持续数回合",
              Profession::Warrior, Element::None, DamageType::Physical,
              0.0f, 20, 12000, 5, {105, 104}, false});

    // 109 - 终极斩击
    AddSkill({109, "终极斩击", "战士终极技能，造成毁灭性伤害",
              Profession::Warrior, Element::None, DamageType::Physical,
              3.5f, 35, 20000, 5, {106, 107}, false});
}

// ============================================================================
// 法师技能 (ID: 200-299)
// ============================================================================
void SkillTreeManager::InitializeMageSkills() {
    // 200 - 火球术
    AddSkill({200, "火球术", "发射火球攻击敌人，有灼烧效果",
              Profession::Mage, Element::Fire, DamageType::Magical,
              1.5f, 10, 2000, 5, {1}, false});

    // 201 - 冰矛
    AddSkill({201, "冰矛", "发射冰矛，有减速效果",
              Profession::Mage, Element::Water, DamageType::Magical,
              1.4f, 12, 2500, 5, {1}, false});

    // 202 - 雷击
    AddSkill({202, "雷击", "召唤雷电打击敌人",
              Profession::Mage, Element::Light, DamageType::Magical,
              1.6f, 15, 3000, 5, {200}, false});

    // 203 - 治愈术
    AddSkill({203, "治愈术", "恢复目标生命值",
              Profession::Mage, Element::Light, DamageType::Magical,
              0.0f, 15, 4000, 5, {1}, false});

    // 204 - 魔法盾
    AddSkill({204, "魔法盾", "生成魔法护盾吸收伤害",
              Profession::Mage, Element::None, DamageType::Magical,
              0.0f, 20, 8000, 5, {203}, false});

    // 205 - 暴风雪
    AddSkill({205, "暴风雪", "召唤暴风雪攻击范围内敌人，有冰冻效果",
              Profession::Mage, Element::Water, DamageType::Magical,
              1.8f, 25, 8000, 5, {201}, false});

    // 206 - 陨石坠落
    AddSkill({206, "陨石坠落", "召唤陨石从天而降，大范围高伤害",
              Profession::Mage, Element::Fire, DamageType::Magical,
              2.8f, 35, 12000, 5, {200, 202}, false});

    // 207 - 瞬移
    AddSkill({207, "瞬移", "瞬间传送到目标位置",
              Profession::Mage, Element::None, DamageType::Magical,
              0.0f, 15, 6000, 3, {204}, false});

    // 208 - 奥术爆发
    AddSkill({208, "奥术爆发", "释放奥术能量造成范围伤害",
              Profession::Mage, Element::None, DamageType::Magical,
              2.2f, 30, 10000, 5, {205, 206}, false});

    // 209 - 复活术
    AddSkill({209, "复活术", "复活一个已阵亡的队友",
              Profession::Mage, Element::Light, DamageType::Magical,
              0.0f, 50, 30000, 3, {203, 204}, false});
}

// ============================================================================
// 弓箭手技能 (ID: 300-399)
// ============================================================================
void SkillTreeManager::InitializeArcherSkills() {
    // 300 - 快速射击
    AddSkill({300, "快速射击", "快速发射一箭",
              Profession::Archer, Element::None, DamageType::Physical,
              1.3f, 5, 1500, 5, {1}, false});

    // 301 - 毒箭
    AddSkill({301, "毒箭", "发射涂有毒液的箭矢，造成持续伤害",
              Profession::Archer, Element::Grass, DamageType::Physical,
              1.1f, 10, 3000, 5, {300}, false});

    // 302 - 多重射击
    AddSkill({302, "多重射击", "同时射出多支箭矢攻击多个目标",
              Profession::Archer, Element::None, DamageType::Physical,
              0.8f, 15, 4000, 5, {300}, false});

    // 303 - 狙击
    AddSkill({303, "狙击", "瞄准要害一击，高暴击率",
              Profession::Archer, Element::None, DamageType::Physical,
              2.2f, 12, 5000, 5, {300}, false});

    // 304 - 陷阱设置
    AddSkill({304, "陷阱设置", "在地面放置陷阱",
              Profession::Archer, Element::None, DamageType::Physical,
              1.0f, 8, 6000, 5, {1}, false});

    // 305 - 闪避射击
    AddSkill({305, "闪避射击", "后跳闪避的同时射出一箭",
              Profession::Archer, Element::None, DamageType::Physical,
              1.2f, 10, 3500, 5, {3, 300}, false});

    // 306 - 箭雨
    AddSkill({306, "箭雨", "向天空射出大量箭矢，覆盖大范围",
              Profession::Archer, Element::None, DamageType::Physical,
              1.5f, 25, 10000, 5, {302}, false});

    // 307 - 爆裂箭
    AddSkill({307, "爆裂箭", "发射爆炸箭矢，造成范围伤害",
              Profession::Archer, Element::Fire, DamageType::Physical,
              2.0f, 20, 7000, 5, {301, 303}, false});

    // 308 - 暗影步
    AddSkill({308, "暗影步", "隐入阴影瞬移到目标身后",
              Profession::Archer, Element::Dark, DamageType::Physical,
              0.0f, 15, 8000, 3, {305}, false});

    // 309 - 穿透射击
    AddSkill({309, "穿透射击", "弓箭手终极技能，箭矢穿透所有障碍",
              Profession::Archer, Element::None, DamageType::Pure,
              3.0f, 30, 18000, 5, {306, 307}, false});
}

} // namespace algorithm
