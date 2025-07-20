/*
 * 文件名: AlgorithmService.cpp
 * 说明: 算法服务实现类的具体实现。
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 本文件实现AlgorithmService类的所有方法，作为算法层的统一入口，
 * 协调各个子模块完成伤害计算和AI决策功能。提供高效、稳定的
 * 算法服务，支持策略层的各种计算需求。
 */
#include "AlgorithmService.h"
#include "DamageCalculator.h"
#include "SkillTreeManager.h"
#include "AIDecisionEngine.h"

namespace algorithm {

/**
 * @brief 构造函数实现
 * 
 * 初始化算法服务的各个子模块，建立完整的算法服务体系。
 * 使用make_unique创建子模块实例，确保异常安全和资源管理。
 */
AlgorithmService::AlgorithmService() 
    : damage_calculator_(std::make_unique<DamageCalculator>())
    , ai_engine_(std::make_unique<AIDecisionEngine>()) {
    // 伤害计算器和AI引擎已通过初始化列表创建
    // 所有子模块都已准备就绪，可以提供服务
}

/**
 * @brief 伤害计算服务实现
 * 
 * 将伤害计算请求委托给专门的伤害计算器处理。
 * 该方法作为算法服务的门面，隐藏了具体的计算实现细节。
 * 
 * @param request 伤害计算请求参数
 * @return DamageResult 计算结果，包含伤害值和效果描述
 */
DamageResult AlgorithmService::CalculateDamage(const DamageRequest& request) {
    // 委托给伤害计算器进行具体计算
    // 伤害计算器会处理所有复杂的计算逻辑，包括：
    // - 基础属性计算
    // - 技能效果应用
    // - 元素克制处理
    // - 暴击和随机因子
    return damage_calculator_->CalculateDamage(request);
}

/**
 * @brief AI决策服务实现
 * 
 * 将AI决策请求委托给专门的AI决策引擎处理。
 * 该方法提供统一的AI决策接口，支持复杂的行为决策。
 * 
 * @param request AI决策请求参数
 * @return AIDecisionResult 决策结果，包含推荐行动和描述
 */
AIDecisionResult AlgorithmService::MakeAIDecision(const AIDecisionRequest& request) {
    // 委托给AI决策引擎进行智能决策
    // AI引擎会基于行为树算法进行决策，包括：
    // - 环境状态分析
    // - 条件判断处理
    // - 行为优先级评估
    // - 最优行动选择
    return ai_engine_->MakeDecision(request);
}

} // namespace algorithm
