/*
 * 文件名: InputValidator.h
 * 说明: 输入数据校验模块
 * 作者: 彭承康
 * 创建时间: 2025-07-24
 */

#ifndef ALGORITHM_INPUT_VALIDATOR_H
#define ALGORITHM_INPUT_VALIDATOR_H

#include "../IAlgorithmService.h"
#include "CharacterStats.h"
#include <string>
#include <vector>

namespace algorithm {

// 前向声明
class SkillTreeManager;

/**
 * @brief 校验结果
 */
struct ValidationResult {
    bool success = true;
    std::string error_message;

    static ValidationResult ok() {
        return {true, ""};
    }

    static ValidationResult fail(const std::string& msg) {
        return {false, msg};
    }
};

/**
 * @brief 输入校验器
 * 校验伤害请求、AI决策请求、技能学习请求等输入的合法性
 */
class InputValidator {
public:
    InputValidator();

    /**
     * @brief 校验伤害计算请求
     * @param request 伤害请求
     * @param registry 角色属性注册表
     * @param skill_manager 技能管理器
     * @return 校验结果
     */
    ValidationResult validateDamageRequest(
        const DamageRequest& request,
        const CharacterStatsRegistry& registry,
        const SkillTreeManager& skill_manager) const;

    /**
     * @brief 校验AI决策请求
     * @param request AI决策请求
     * @return 校验结果
     */
    ValidationResult validateAIDecisionRequest(const AIDecisionRequest& request) const;

    /**
     * @brief 校验技能学习请求
     * @param skill_id 目标技能ID
     * @param character_profession 角色职业
     * @param learned_skills 已学技能列表
     * @param skill_manager 技能管理器
     * @return 校验结果
     */
    ValidationResult validateSkillLearnRequest(
        int skill_id,
        Profession character_profession,
        const std::vector<int>& learned_skills,
        const SkillTreeManager& skill_manager) const;

    /**
     * @brief 校验角色属性合理性
     * @param stats 角色属性
     * @return 校验结果
     */
    ValidationResult validateCharacterStats(const CharacterStats& stats) const;

private:
    static constexpr int MAX_CHARACTER_ID = 100000;
    static constexpr int MAX_SKILL_ID = 10000;
    static constexpr int MAX_NPC_ID = 100000;
    static constexpr int MIN_CONTEXT_SIZE = 2;  // 最少需要距离+血量
    static constexpr float MAX_STAT_VALUE = 99999.0f;
};

} // namespace algorithm

#endif // ALGORITHM_INPUT_VALIDATOR_H
