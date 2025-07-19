/*
 * 文件名: AlgorithmService.cpp
 * 说明: 算法服务实现类的具体实现。
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 *
 * 本文件实现AlgorithmService类的所有方法，作为算法层的统一入口，
 * 协调各个子模块完成伤害计算和AI决策功能。
 */
#include "AlgorithmService.h"
#include "DamageCalculator.h"
#include "SkillTreeManager.h"
#include "AIDecisionEngine.h"

namespace algorithm {

AlgorithmService::AlgorithmService() 
    : damage_calculator_(std::make_unique<DamageCalculator>())
    , ai_engine_(std::make_unique<AIDecisionEngine>()) {
}

DamageResult AlgorithmService::CalculateDamage(const DamageRequest& request) {
    return damage_calculator_->CalculateDamage(request);
}

AIDecisionResult AlgorithmService::MakeAIDecision(const AIDecisionRequest& request) {
    return ai_engine_->MakeDecision(request);
}

} // namespace algorithm
