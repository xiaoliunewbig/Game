/*
 * 文件名: SkillTreeManager.h
 * 说明: 技能树管理器类定义，支持多职业技能体系
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 * 更新时间: 2025-07-24 — 扩展至33+技能，支持职业/元素/升级
 */

#ifndef ALGORITHM_SKILL_TREE_MANAGER_H
#define ALGORITHM_SKILL_TREE_MANAGER_H

#include "CharacterStats.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace algorithm {

/**
 * @brief 技能节点结构
 */
struct SkillNode {
    int skill_id = 0;                    ///< 技能ID
    std::string name;                    ///< 技能名称
    std::string description;             ///< 技能描述
    Profession profession = Profession::None;  ///< 所属职业 (None=通用)
    Element element = Element::None;     ///< 技能元素
    DamageType damage_type = DamageType::Physical;  ///< 伤害类型
    float damage_multiplier = 1.0f;      ///< 基础伤害倍率
    int mana_cost = 0;                   ///< 魔法消耗
    int cooldown_ms = 0;                 ///< 冷却时间（毫秒）
    int max_level = 5;                   ///< 最大等级
    std::vector<int> prerequisites;      ///< 前置技能ID列表
    bool is_passive = false;             ///< 是否被动技能
};

/**
 * @brief 技能树管理器
 * 负责管理游戏中的技能系统，包括技能数据、技能树结构、技能升级等
 */
class SkillTreeManager {
public:
    SkillTreeManager();
    ~SkillTreeManager() = default;

    /**
     * @brief 获取技能信息
     * @param skill_id 技能ID
     * @return 技能节点指针，如果不存在返回nullptr
     */
    const SkillNode* GetSkill(int skill_id) const;

    /**
     * @brief 添加技能
     * @param skill 技能节点
     */
    void AddSkill(const SkillNode& skill);

    /**
     * @brief 检查技能是否可学习
     * @param skill_id 技能ID
     * @param learned_skills 已学习的技能ID列表
     * @return 是否可学习
     */
    bool CanLearnSkill(int skill_id, const std::vector<int>& learned_skills) const;

    /**
     * @brief 获取某职业的所有技能
     * @param profession 职业
     * @return 技能列表
     */
    std::vector<const SkillNode*> GetSkillsByProfession(Profession profession) const;

    /**
     * @brief 获取所有技能
     * @return 技能列表
     */
    std::vector<const SkillNode*> GetAllSkills() const;

    /**
     * @brief 计算技能在指定等级的伤害倍率
     * @param skill_id 技能ID
     * @param skill_level 技能等级 (1-max_level)
     * @return 伤害倍率（含等级加成）
     */
    float GetScaledMultiplier(int skill_id, int skill_level) const;

    /**
     * @brief 计算技能在指定等级的冷却时间
     * @param skill_id 技能ID
     * @param skill_level 技能等级
     * @return 冷却时间（毫秒）
     */
    int GetScaledCooldown(int skill_id, int skill_level) const;

    /**
     * @brief 获取技能数量
     */
    int GetSkillCount() const { return static_cast<int>(skills_.size()); }

private:
    std::unordered_map<int, SkillNode> skills_;

    void InitializeDefaultSkills();
    void InitializeCommonSkills();
    void InitializeWarriorSkills();
    void InitializeMageSkills();
    void InitializeArcherSkills();
};

} // namespace algorithm

#endif // ALGORITHM_SKILL_TREE_MANAGER_H
