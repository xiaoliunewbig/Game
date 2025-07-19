/*
 * 文件名: AlgorithmInterface.h
 * 说明: 策略层与算法层的交互接口，封装算法服务调用。
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 *
 * 本类封装对算法层服务的调用，提供伤害计算、AI决策等功能的访问接口，
 * 实现策略层与算法层的解耦。
 */
#ifndef STRATEGY_ALGORITHMINTERFACE_H
#define STRATEGY_ALGORITHMINTERFACE_H

#include <memory>
#include <vector>
#include <string>

// 前向声明算法层接口
namespace algorithm {
    class IAlgorithmService;
    struct DamageRequest;
    struct DamageResult;
    struct AIDecisionRequest;
    struct AIDecisionResult;
}

namespace strategy {

/**
 * @brief 算法层交互接口
 */
class AlgorithmInterface {
public:
    AlgorithmInterface();
    ~AlgorithmInterface();

    /**
     * @brief 初始化算法服务连接
     */
    bool Initialize();

    /**
     * @brief 请求伤害计算
     */
    algorithm::DamageResult RequestDamageCalculation(int attacker_id, int defender_id, int skill_id);

    /**
     * @brief 请求AI决策
     */
    algorithm::AIDecisionResult RequestAIDecision(int npc_id, const std::vector<int>& context);

    /**
     * @brief 批量AI决策请求
     */
    std::vector<algorithm::AIDecisionResult> RequestBatchAIDecision(
        const std::vector<int>& npc_ids, 
        const std::vector<std::vector<int>>& contexts);

    /**
     * @brief 检查算法服务是否可用
     */
    bool IsAlgorithmServiceAvailable() const;

private:
    std::unique_ptr<algorithm::IAlgorithmService> algorithm_service_;
    bool is_initialized_;
    
    void CreateAlgorithmService();
};

} // namespace strategy

#endif // STRATEGY_ALGORITHMINTERFACE_H