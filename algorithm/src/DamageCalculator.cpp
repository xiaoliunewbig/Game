/*
 * 文件名: DamageCalculator.cpp
 * 说明: 伤害计算器的具体实现。
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 *
 * 本文件实现复杂的伤害计算算法，包括基础伤害、技能倍率、元素克制、
 * 暴击系统和随机波动等计算逻辑。
 */
#include "DamageCalculator.h"
#include "SkillTreeManager.h"
#include <random>
#include <sstream>
#include <memory>

namespace algorithm {

DamageCalculator::DamageCalculator() 
    : skill_manager_(std::make_unique<SkillTreeManager>()) {
    InitializeElementData();
}

DamageCalculator::~DamageCalculator() = default;

DamageResult DamageCalculator::CalculateDamage(const DamageRequest& request) {
    DamageResult result;
    
    // 获取技能信息
    const SkillNode* skill = skill_manager_->GetSkill(request.skill_id);
    float skill_multiplier = skill ? skill->damage_multiplier : 1.0f;
    
    // 基础伤害计算 (简化公式)
    float base_damage = 100.0f;
    base_damage *= skill_multiplier;
    
    // 元素克制计算
    float element_multiplier = GetElementMultiplier(request.attacker_id, request.defender_id);
    base_damage *= element_multiplier;
    
    // 暴击计算
    float crit_multiplier = GetCriticalMultiplier();
    bool is_critical = crit_multiplier > 1.0f;
    base_damage *= crit_multiplier;
    
    // 随机波动 (±10%)
    base_damage *= GetRandomMultiplier();
    
    result.damage = static_cast<int>(std::max(1.0f, base_damage));
    
    // 生成效果描述
    std::stringstream effect_desc;
    effect_desc << "造成 " << result.damage << " 点伤害";
    if (is_critical) {
        effect_desc << " (暴击!)";
    }
    if (element_multiplier > 1.0f) {
        effect_desc << " (克制效果)";
    } else if (element_multiplier < 1.0f) {
        effect_desc << " (抗性减免)";
    }
    
    result.effect = effect_desc.str();
    return result;
}

void DamageCalculator::InitializeElementData() {
    // 初始化元素克制数据
    // 实际实现中应该从配置文件加载
}

float DamageCalculator::GetElementMultiplier(int attacker_id, int defender_id) {
    // 简化的元素克制计算
    (void)attacker_id;  // 避免未使用参数警告
    (void)defender_id;
    
    // 随机返回0.8-1.2之间的倍率
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dis(0.8f, 1.2f);
    
    return dis(gen);
}

float DamageCalculator::GetCriticalMultiplier() {
    // 20%暴击率，暴击倍率1.5
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    
    return dis(gen) < 0.2f ? 1.5f : 1.0f;
}

float DamageCalculator::GetRandomMultiplier() {
    // ±10%随机波动
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dis(0.9f, 1.1f);
    
    return dis(gen);
}

} // namespace algorithm
