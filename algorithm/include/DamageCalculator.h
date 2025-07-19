/*
 * 文件名: DamageCalculator.h
 * 说明: 伤害计算器类定义
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 */

#ifndef ALGORITHM_DAMAGE_CALCULATOR_H
#define ALGORITHM_DAMAGE_CALCULATOR_H

#include "../IAlgorithmService.h"
#include <memory>
#include <unordered_map>

namespace algorithm {

// 前向声明
class SkillTreeManager;

/**
 * @brief 伤害计算器
 * 负责计算战斗中的伤害值，包括基础伤害、技能倍率、元素克制等
 */
class DamageCalculator {
public:
    DamageCalculator();
    ~DamageCalculator();

    /**
     * @brief 计算伤害
     * @param request 伤害计算请求
     * @return 伤害计算结果
     */
    DamageResult CalculateDamage(const DamageRequest& request);

private:
    std::unique_ptr<SkillTreeManager> skill_manager_;

    // 元素克制关系
    std::unordered_map<std::string, float> element_effectiveness_;
    
    void InitializeElementData();
    float GetCriticalMultiplier();
    float GetElementMultiplier(int attacker_id, int defender_id);
    float GetRandomMultiplier();
    std::string GetCharacterElement(int character_id);
};

} // namespace algorithm

#endif // ALGORITHM_DAMAGE_CALCULATOR_H
