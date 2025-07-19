/*
 * 文件名: AIDecisionEngine.h
 * 说明: AI决策引擎，基于行为树实现NPC智能决策系统。
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 *
 * 本类实现基于行为树的AI决策算法，为不同类型的NPC提供智能行为决策，
 * 支持条件判断、动作选择和战术分析。
 */
#ifndef ALGORITHM_AIDECISIONENGINE_H
#define ALGORITHM_AIDECISIONENGINE_H

#include "../IAlgorithmService.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
#include <map>

namespace algorithm {

/**
 * @brief AI决策引擎
 * 负责NPC的行为决策和战术分析
 */
class AIDecisionEngine {
public:
    AIDecisionEngine();
    
    /**
     * @brief 执行AI决策
     * @param request AI决策请求
     * @return AIDecisionResult 决策结果
     */
    AIDecisionResult MakeDecision(const AIDecisionRequest& request);

private:
    // 行为树节点类型
    enum class NodeType {
        SELECTOR,    // 选择节点
        SEQUENCE,    // 序列节点
        CONDITION,   // 条件节点
        ACTION       // 动作节点
    };

    // 行为树节点
    struct BehaviorNode {
        NodeType type;
        int node_id;
        std::vector<std::shared_ptr<BehaviorNode>> children;
        std::function<bool(const std::vector<int>&)> condition;
        int action_id;
        std::string description;
    };

    // NPC行为树映射
    std::unordered_map<int, std::shared_ptr<BehaviorNode>> npc_behavior_trees_;
    
    // 动作权重表
    std::unordered_map<int, float> action_weights_;

    void InitializeBehaviorTrees();
    void InitializeActionWeights();
    
    bool EvaluateNode(const std::shared_ptr<BehaviorNode>& node, 
                     const std::vector<int>& context);
    
    int SelectBestAction(int npc_id, const std::vector<int>& context);
    std::string GenerateActionDescription(int action_id, int npc_id);
    
    // 具体的条件判断函数
    bool IsEnemyNearby(const std::vector<int>& context);
    bool IsHealthLow(const std::vector<int>& context);
    bool HasMana(const std::vector<int>& context);
};

} // namespace algorithm

#endif // ALGORITHM_AIDECISIONENGINE_H
