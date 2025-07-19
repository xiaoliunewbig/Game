/*
 * 文件名: AlgorithmService.h
 * 说明: 算法服务实现类，提供伤害计算和AI决策的具体实现。
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 *
 * 本类实现IAlgorithmService接口，整合DamageCalculator和AIDecisionEngine，
 * 为策略层提供统一的算法服务入口。
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
 * 实现 IAlgorithmService 接口，提供具体的算法服务
 */
class AlgorithmService : public IAlgorithmService {
public:
    AlgorithmService();
    virtual ~AlgorithmService() = default;

    /**
     * @brief 计算伤害
     */
    DamageResult CalculateDamage(const DamageRequest& request) override;

    /**
     * @brief AI决策
     */
    AIDecisionResult MakeAIDecision(const AIDecisionRequest& request) override;

private:
    std::unique_ptr<DamageCalculator> damage_calculator_;
    std::unique_ptr<AIDecisionEngine> ai_engine_;
};

} // namespace algorithm

#endif // ALGORITHM_ALGORITHMSERVICE_H
