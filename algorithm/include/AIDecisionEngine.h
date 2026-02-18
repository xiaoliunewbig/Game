/*
 * 文件名: AIDecisionEngine.h
 * 说明: AI决策引擎，基于行为树实现多类型NPC智能决策系统
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 * 更新时间: 2025-07-24 — 支持5种行为树、决策记忆、Boss阶段转换
 */
#ifndef ALGORITHM_AIDECISIONENGINE_H
#define ALGORITHM_AIDECISIONENGINE_H

#include "../IAlgorithmService.h"
#include "CharacterStats.h"
#include <unordered_map>
#include <vector>
#include <deque>
#include <memory>
#include <functional>
#include <string>

namespace algorithm {

/**
 * @brief NPC类型枚举
 */
enum class NPCType {
    Warrior = 0,   // 战士NPC
    Mage = 1,      // 法师NPC
    Archer = 2,    // 弓箭手NPC
    Boss = 3,      // Boss NPC
    Passive = 4    // 被动NPC
};

/**
 * @brief AI决策引擎
 * 支持多行为树、决策记忆和Boss阶段转换
 */
class AIDecisionEngine {
public:
    AIDecisionEngine();

    /**
     * @brief 执行AI决策
     * @param request AI决策请求
     * @return 决策结果
     */
    AIDecisionResult MakeDecision(const AIDecisionRequest& request);

    /**
     * @brief 注册NPC类型
     * @param npc_id NPC ID
     * @param type NPC类型
     */
    void RegisterNPCType(int npc_id, NPCType type);

    /**
     * @brief 清除决策记忆
     * @param npc_id NPC ID (0=清除所有)
     */
    void ClearMemory(int npc_id = 0);

private:
    // 行为树节点类型
    enum class NodeType {
        SELECTOR,    // 选择节点：任一子节点成功即成功
        SEQUENCE,    // 序列节点：所有子节点都成功才成功
        CONDITION,   // 条件节点
        ACTION       // 动作节点
    };

    struct BehaviorNode {
        NodeType type;
        int node_id = 0;
        std::vector<std::shared_ptr<BehaviorNode>> children;
        std::function<bool(const std::vector<int>&)> condition;
        int action_id = 0;
        std::string description;
    };

    // 行为树存储: NPCType → 行为树根节点
    std::unordered_map<int, std::shared_ptr<BehaviorNode>> behavior_trees_;

    // NPC类型映射: npc_id → NPCType
    std::unordered_map<int, NPCType> npc_types_;

    // 决策记忆: npc_id → 最近N次决策的action_id
    std::unordered_map<int, std::deque<int>> decision_memory_;
    static constexpr int MEMORY_SIZE = 5;

    // 初始化
    void InitializeBehaviorTrees();
    void InitializeActionWeights();

    // 行为树构建
    std::shared_ptr<BehaviorNode> BuildWarriorTree();
    std::shared_ptr<BehaviorNode> BuildMageTree();
    std::shared_ptr<BehaviorNode> BuildArcherTree();
    std::shared_ptr<BehaviorNode> BuildBossTree();
    std::shared_ptr<BehaviorNode> BuildPassiveTree();

    // 行为树评估
    bool EvaluateNode(const std::shared_ptr<BehaviorNode>& node,
                      const std::vector<int>& context);

    // 决策选择
    int SelectBestAction(int npc_id, NPCType type, const std::vector<int>& context);
    float GetRepetitionPenalty(int npc_id, int action_id) const;
    void RecordDecision(int npc_id, int action_id);

    // Boss阶段判定
    int GetBossPhase(const std::vector<int>& context) const;

    // 条件判断函数
    bool IsEnemyNearby(const std::vector<int>& context) const;
    bool IsEnemyFar(const std::vector<int>& context) const;
    bool IsHealthLow(const std::vector<int>& context) const;
    bool IsHealthCritical(const std::vector<int>& context) const;
    bool IsHealthHigh(const std::vector<int>& context) const;
    bool HasMana(const std::vector<int>& context) const;
    bool HasLowMana(const std::vector<int>& context) const;
    bool IsInCombat(const std::vector<int>& context) const;
    bool IsHighThreat(const std::vector<int>& context) const;

    // 描述生成
    std::string GenerateActionDescription(int action_id, int npc_id, NPCType type) const;

    // 辅助构建
    std::shared_ptr<BehaviorNode> MakeSelector(int id,
        std::vector<std::shared_ptr<BehaviorNode>> children);
    std::shared_ptr<BehaviorNode> MakeSequence(int id,
        std::vector<std::shared_ptr<BehaviorNode>> children);
    std::shared_ptr<BehaviorNode> MakeCondition(int id,
        std::function<bool(const std::vector<int>&)> cond);
    std::shared_ptr<BehaviorNode> MakeAction(int id, int action_id, const std::string& desc);
};

} // namespace algorithm

#endif // ALGORITHM_AIDECISIONENGINE_H
