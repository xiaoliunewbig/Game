/*
 * 文件名: SkillTreeManager.h
 * 说明: 技能树管理器类定义
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 */

#ifndef ALGORITHM_SKILL_TREE_MANAGER_H
#define ALGORITHM_SKILL_TREE_MANAGER_H

#include <string>
#include <unordered_map>
#include <vector>

namespace algorithm {

/**
 * @brief 技能节点结构
 */
struct SkillNode {
    int skill_id;                    ///< 技能ID
    std::string name;                ///< 技能名称
    std::string description;         ///< 技能描述
    float damage_multiplier;         ///< 伤害倍率
    int mana_cost;                   ///< 魔法消耗
    int cooldown;                    ///< 冷却时间
    std::vector<int> prerequisites;  ///< 前置技能ID列表
};

/**
 * @brief 技能树管理器
 * 负责管理游戏中的技能系统，包括技能数据、技能树结构等
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

private:
    std::unordered_map<int, SkillNode> skills_;  ///< 技能数据存储

    /**
     * @brief 初始化默认技能
     */
    void InitializeDefaultSkills();
};

} // namespace algorithm

#endif // ALGORITHM_SKILL_TREE_MANAGER_H
