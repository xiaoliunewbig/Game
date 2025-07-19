/*
 * 文件名: IAlgorithmService.h
 * 说明: 算法层对外服务接口，定义与策略层的数据交互方式。
 * 作者: 彭承康
 * 创建时间: 2025-07-13
 *
 * 本接口用于算法层与策略层之间的数据交互，包括伤害计算、AI决策等核心功能。
 */
#ifndef ALGORITHM_IALGORITHMSERVICE_H
#define ALGORITHM_IALGORITHMSERVICE_H

#include <string>
#include <vector>

namespace algorithm {

/**
 * @brief 伤害计算请求参数
 * 包含攻击者、被攻击者、技能等信息。
 */
struct DamageRequest {
    int attacker_id;   ///< 攻击者ID
    int defender_id;   ///< 防御者ID
    int skill_id;      ///< 技能ID
};

/**
 * @brief 伤害计算结果
 * 包含伤害数值和效果描述。
 */
struct DamageResult {
    int damage;            ///< 计算出的伤害值
    std::string effect;    ///< 效果描述（如暴击、元素反应等）
};

/**
 * @brief AI决策请求参数
 * 包含NPC编号及决策上下文。
 */
struct AIDecisionRequest {
    int npc_id;                ///< NPC编号
    std::vector<int> context;  ///< 决策上下文（如环境、状态等）
};

/**
 * @brief AI决策结果
 * 包含动作编号及描述。
 */
struct AIDecisionResult {
    int action_id;             ///< 动作编号
    std::string description;   ///< 动作描述
};

/**
 * @brief 算法服务接口（抽象类）
 * 提供伤害计算、AI决策等核心算法服务。
 */
class IAlgorithmService {
public:
    virtual ~IAlgorithmService() = default;
    /**
     * @brief 计算伤害
     * @param request 伤害计算请求参数
     * @return DamageResult 伤害计算结果
     */
    virtual DamageResult CalculateDamage(const DamageRequest& request) = 0;
    /**
     * @brief AI决策
     * @param request AI决策请求参数
     * @return AIDecisionResult AI决策结果
     */
    virtual AIDecisionResult MakeAIDecision(const AIDecisionRequest& request) = 0;
};

} // namespace algorithm

#endif // ALGORITHM_IALGORITHMSERVICE_H 
