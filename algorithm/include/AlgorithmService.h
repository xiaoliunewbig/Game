/*
 * 文件名: AlgorithmService.h
 * 说明: 算法服务实现类，提供伤害计算和AI决策的具体实现。
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 本类实现IAlgorithmService接口，整合DamageCalculator和AIDecisionEngine，
 * 为策略层提供统一的算法服务入口。作为算法层的门面模式实现，
 * 协调各个算法子模块的工作，提供高层次的算法服务接口。
 */
#ifndef ALGORITHM_ALGORITHMSERVICE_H
#define ALGORITHM_ALGORITHMSERVICE_H

#include "../IAlgorithmService.h"
#include "DamageCalculator.h"
#include "AIDecisionEngine.h"
#include <memory>

namespace algorithm {

/**
 * @brief 算法服务实现类
 * 
 * 实现 IAlgorithmService 接口，提供具体的算法服务功能。
 * 该类作为算法层的统一入口，整合了以下核心功能：
 * - 战斗伤害计算服务
 * - AI行为决策服务
 * - 算法资源管理和生命周期控制
 * 
 * 采用组合模式，将具体的算法实现委托给专门的子模块，
 * 保持代码的模块化和可维护性。
 */
class AlgorithmService : public IAlgorithmService {
public:
    /**
     * @brief 构造函数
     * 
     * 初始化算法服务，创建伤害计算器和AI决策引擎实例。
     * 使用智能指针管理子模块的生命周期，确保资源安全。
     */
    AlgorithmService();
    
    /**
     * @brief 虚析构函数
     * 
     * 确保派生类对象能够正确析构，释放所有相关资源。
     * 使用默认析构函数，智能指针会自动清理子模块。
     */
    virtual ~AlgorithmService() = default;

    /**
     * @brief 计算伤害
     * 
     * 实现IAlgorithmService接口的伤害计算功能。
     * 将请求委托给DamageCalculator进行具体计算。
     * 
     * @param request 伤害计算请求，包含攻击者、防御者和技能信息
     * @return DamageResult 伤害计算结果，包含伤害数值和效果描述
     * 
     * @note 该方法线程安全，可以在多线程环境中调用
     * @see DamageCalculator::CalculateDamage()
     */
    DamageResult CalculateDamage(const DamageRequest& request) override;

    /**
     * @brief AI决策
     * 
     * 实现IAlgorithmService接口的AI决策功能。
     * 将请求委托给AIDecisionEngine进行智能决策。
     * 
     * @param request AI决策请求，包含NPC信息和上下文环境
     * @return AIDecisionResult AI决策结果，包含推荐行动和描述
     * 
     * @note 决策基于行为树算法，支持复杂的条件判断和行为选择
     * @see AIDecisionEngine::MakeDecision()
     */
    AIDecisionResult MakeAIDecision(const AIDecisionRequest& request) override;

private:
    /**
     * @brief 伤害计算器实例
     * 
     * 负责处理所有战斗相关的伤害计算，包括：
     * - 基础伤害计算
     * - 技能倍率应用
     * - 元素克制计算
     * - 暴击和随机因子处理
     */
    std::unique_ptr<DamageCalculator> damage_calculator_;
    
    /**
     * @brief AI决策引擎实例
     * 
     * 负责处理所有NPC的智能决策，包括：
     * - 行为树评估
     * - 条件判断处理
     * - 最优行动选择
     * - 决策结果生成
     */
    std::unique_ptr<AIDecisionEngine> ai_engine_;
};

} // namespace algorithm

#endif // ALGORITHM_ALGORITHMSERVICE_H
