/*
 * 文件名: AlgorithmService.h
 * 说明: 算法服务实现类，整合所有算法子模块
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 * 更新时间: 2025-07-24 — 集成角色属性、输入校验、冷却追踪、状态效果
 */
#ifndef ALGORITHM_ALGORITHMSERVICE_H
#define ALGORITHM_ALGORITHMSERVICE_H

#include "../IAlgorithmService.h"
#include "CharacterStats.h"
#include "StatusEffect.h"
#include "CooldownTracker.h"
#include "InputValidator.h"
#include "DamageCalculator.h"
#include "AIDecisionEngine.h"
#include <memory>
#include <vector>
#include <unordered_map>

namespace algorithm {

/**
 * @brief 算法服务实现类
 *
 * 实现 IAlgorithmService 接口，整合以下子系统：
 * - CharacterStatsRegistry: 角色属性管理
 * - DamageCalculator: 伤害计算（含元素/防御/暴击/状态效果）
 * - AIDecisionEngine: AI行为决策（5种行为树）
 * - SkillTreeManager: 技能树管理（33+技能）
 * - InputValidator: 输入校验
 * - CooldownTracker: 技能冷却追踪
 */
class AlgorithmService : public IAlgorithmService {
public:
    AlgorithmService();
    virtual ~AlgorithmService() = default;

    // ============ IAlgorithmService 接口实现 ============

    DamageResult CalculateDamage(const DamageRequest& request) override;
    AIDecisionResult MakeAIDecision(const AIDecisionRequest& request) override;

    // ============ 扩展接口 ============

    /**
     * @brief 扩展伤害计算
     */
    ExtendedDamageResult CalculateExtendedDamage(const ExtendedDamageRequest& request);

    /**
     * @brief 注册角色属性
     */
    void RegisterCharacter(const CharacterStats& stats);

    /**
     * @brief 查询角色属性
     */
    const CharacterStats* GetCharacterStats(int character_id) const;

    /**
     * @brief 获取职业默认属性
     */
    CharacterStats GetDefaultStats(Profession profession, int level = 1) const;

    /**
     * @brief 注册NPC类型
     */
    void RegisterNPCType(int npc_id, NPCType type);

    /**
     * @brief 获取技能信息
     */
    const SkillNode* GetSkillInfo(int skill_id) const;

    /**
     * @brief 获取职业技能列表
     */
    std::vector<const SkillNode*> GetSkillsByProfession(Profession profession) const;

    /**
     * @brief 检查技能冷却
     */
    bool IsSkillReady(int character_id, int skill_id) const;

    /**
     * @brief 启动技能冷却
     */
    void StartSkillCooldown(int character_id, int skill_id, int cooldown_ms);

    /**
     * @brief 更新冷却计时
     */
    void TickCooldowns(int delta_ms);

    /**
     * @brief 管理角色状态效果
     */
    void AddStatusEffect(int character_id, const StatusEffect& effect);
    std::vector<StatusEffect> GetStatusEffects(int character_id) const;
    void TickStatusEffects(int character_id);
    void ClearStatusEffects(int character_id);

    /**
     * @brief 校验技能学习请求
     */
    ValidationResult ValidateSkillLearn(int skill_id, Profession profession,
                                        const std::vector<int>& learned_skills) const;

    /**
     * @brief 获取元素克制倍率
     */
    float GetElementMultiplier(Element attacker, Element defender) const;

private:
    std::unique_ptr<DamageCalculator> damage_calculator_;
    std::unique_ptr<AIDecisionEngine> ai_engine_;
    std::unique_ptr<CharacterStatsRegistry> stats_registry_;
    std::unique_ptr<InputValidator> validator_;
    CooldownTracker cooldown_tracker_;

    // 角色状态效果: character_id → 状态列表
    std::unordered_map<int, std::vector<StatusEffect>> status_effects_;
};

} // namespace algorithm

#endif // ALGORITHM_ALGORITHMSERVICE_H
