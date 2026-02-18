/*
 * 文件名: AlgorithmService.cpp
 * 说明: 算法服务实现 — 整合所有子模块
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 * 更新时间: 2025-07-24 — 集成角色属性、输入校验、冷却追踪、状态效果
 */
#include "AlgorithmService.h"
#include "SkillTreeManager.h"

namespace algorithm {

AlgorithmService::AlgorithmService()
    : damage_calculator_(std::make_unique<DamageCalculator>())
    , ai_engine_(std::make_unique<AIDecisionEngine>())
    , stats_registry_(std::make_unique<CharacterStatsRegistry>())
    , validator_(std::make_unique<InputValidator>()) {
    // 将属性注册表注入伤害计算器
    damage_calculator_->setStatsRegistry(stats_registry_.get());
}

// ============================================================================
// IAlgorithmService 接口实现
// ============================================================================

DamageResult AlgorithmService::CalculateDamage(const DamageRequest& request) {
    // 输入校验
    auto validation = validator_->validateDamageRequest(
        request, *stats_registry_, *damage_calculator_->getSkillManager());
    if (!validation.success) {
        return {0, "校验失败: " + validation.error_message};
    }

    // 获取角色状态效果
    auto atk_effects = GetStatusEffects(request.attacker_id);
    auto def_effects = GetStatusEffects(request.defender_id);

    // 构造扩展请求
    const CharacterStats* atk = stats_registry_->getCharacterStats(request.attacker_id);
    const CharacterStats* def = stats_registry_->getCharacterStats(request.defender_id);

    if (atk && def) {
        ExtendedDamageRequest ext;
        ext.attacker = *atk;
        ext.defender = *def;
        ext.skill_id = request.skill_id;
        ext.skill_level = 1;
        ext.attacker_effects = atk_effects;
        ext.defender_effects = def_effects;

        auto ext_result = damage_calculator_->CalculateExtendedDamage(ext);

        // 自动启动技能冷却
        const SkillNode* skill = damage_calculator_->getSkillManager()->GetSkill(request.skill_id);
        if (skill && skill->cooldown_ms > 0) {
            cooldown_tracker_.startCooldown(request.attacker_id, request.skill_id, skill->cooldown_ms);
        }

        // 自动附加状态效果
        for (const auto& eff : ext_result.applied_effects) {
            AddStatusEffect(request.defender_id, eff);
        }

        return {ext_result.damage, ext_result.effect};
    }

    // 兜底
    return damage_calculator_->CalculateDamage(request);
}

AIDecisionResult AlgorithmService::MakeAIDecision(const AIDecisionRequest& request) {
    // 输入校验
    auto validation = validator_->validateAIDecisionRequest(request);
    if (!validation.success) {
        return {0, "校验失败: " + validation.error_message};
    }

    return ai_engine_->MakeDecision(request);
}

// ============================================================================
// 扩展接口
// ============================================================================

ExtendedDamageResult AlgorithmService::CalculateExtendedDamage(const ExtendedDamageRequest& request) {
    return damage_calculator_->CalculateExtendedDamage(request);
}

void AlgorithmService::RegisterCharacter(const CharacterStats& stats) {
    auto validation = validator_->validateCharacterStats(stats);
    if (validation.success) {
        stats_registry_->registerCharacter(stats);
    }
}

const CharacterStats* AlgorithmService::GetCharacterStats(int character_id) const {
    return stats_registry_->getCharacterStats(character_id);
}

CharacterStats AlgorithmService::GetDefaultStats(Profession profession, int level) const {
    return stats_registry_->getDefaultStats(profession, level);
}

void AlgorithmService::RegisterNPCType(int npc_id, NPCType type) {
    ai_engine_->RegisterNPCType(npc_id, type);
}

const SkillNode* AlgorithmService::GetSkillInfo(int skill_id) const {
    return damage_calculator_->getSkillManager()->GetSkill(skill_id);
}

std::vector<const SkillNode*> AlgorithmService::GetSkillsByProfession(Profession profession) const {
    return damage_calculator_->getSkillManager()->GetSkillsByProfession(profession);
}

bool AlgorithmService::IsSkillReady(int character_id, int skill_id) const {
    return cooldown_tracker_.isReady(character_id, skill_id);
}

void AlgorithmService::StartSkillCooldown(int character_id, int skill_id, int cooldown_ms) {
    cooldown_tracker_.startCooldown(character_id, skill_id, cooldown_ms);
}

void AlgorithmService::TickCooldowns(int delta_ms) {
    cooldown_tracker_.tick(delta_ms);
}

// ============================================================================
// 状态效果管理
// ============================================================================

void AlgorithmService::AddStatusEffect(int character_id, const StatusEffect& effect) {
    status_effects_[character_id].push_back(effect);
}

std::vector<StatusEffect> AlgorithmService::GetStatusEffects(int character_id) const {
    auto it = status_effects_.find(character_id);
    if (it != status_effects_.end()) {
        return it->second;
    }
    return {};
}

void AlgorithmService::TickStatusEffects(int character_id) {
    auto it = status_effects_.find(character_id);
    if (it == status_effects_.end()) return;

    auto& effects = it->second;
    // 移除过期效果
    effects.erase(
        std::remove_if(effects.begin(), effects.end(),
            [](StatusEffect& e) { return !e.tick(); }),
        effects.end());

    if (effects.empty()) {
        status_effects_.erase(it);
    }
}

void AlgorithmService::ClearStatusEffects(int character_id) {
    status_effects_.erase(character_id);
}

// ============================================================================
// 校验和查询
// ============================================================================

ValidationResult AlgorithmService::ValidateSkillLearn(int skill_id, Profession profession,
                                                       const std::vector<int>& learned_skills) const {
    return validator_->validateSkillLearnRequest(
        skill_id, profession, learned_skills, *damage_calculator_->getSkillManager());
}

float AlgorithmService::GetElementMultiplier(Element attacker, Element defender) const {
    return stats_registry_->getElementMultiplier(attacker, defender);
}

} // namespace algorithm
