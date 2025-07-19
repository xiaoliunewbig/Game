/*
 * 文件名: AIDecisionEngine.cpp
 * 说明: AI决策引擎的具体实现。
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 *
 * 本文件实现基于行为树的AI决策算法，包括行为树构建、节点评估、
 * 动作选择和决策描述生成等核心功能。
 */
#include "AIDecisionEngine.h"
#include <random>
#include <algorithm>
#include <sstream>

namespace algorithm {

AIDecisionEngine::AIDecisionEngine() {
    InitializeBehaviorTrees();
    InitializeActionWeights();
}

AIDecisionResult AIDecisionEngine::MakeDecision(const AIDecisionRequest& request) {
    AIDecisionResult result;
    
    // 查找对应的行为树
    auto tree_it = npc_behavior_trees_.find(request.npc_id);
    if (tree_it == npc_behavior_trees_.end()) {
        // 使用默认行为
        result.action_id = 1; // 默认攻击
        result.description = "执行默认攻击行为";
        return result;
    }
    
    // 执行行为树决策
    if (EvaluateNode(tree_it->second, request.context)) {
        result.action_id = SelectBestAction(request.npc_id, request.context);
        result.description = GenerateActionDescription(result.action_id, request.npc_id);
    } else {
        // 行为树执行失败，使用待机行为
        result.action_id = 0;
        result.description = "NPC待机中";
    }
    
    return result;
}

void AIDecisionEngine::InitializeBehaviorTrees() {
    // 创建基础战士NPC的行为树 (ID: 1)
    auto warrior_root = std::make_shared<BehaviorNode>();
    warrior_root->type = NodeType::SELECTOR;
    warrior_root->node_id = 1;
    
    // 攻击分支
    auto attack_branch = std::make_shared<BehaviorNode>();
    attack_branch->type = NodeType::SEQUENCE;
    attack_branch->node_id = 2;
    
    auto enemy_check = std::make_shared<BehaviorNode>();
    enemy_check->type = NodeType::CONDITION;
    enemy_check->node_id = 3;
    enemy_check->condition = [this](const std::vector<int>& ctx) { return IsEnemyNearby(ctx); };
    
    auto attack_action = std::make_shared<BehaviorNode>();
    attack_action->type = NodeType::ACTION;
    attack_action->node_id = 4;
    attack_action->action_id = 1; // 攻击动作
    attack_action->description = "执行攻击";
    
    attack_branch->children = {enemy_check, attack_action};
    
    // 防御分支
    auto defend_branch = std::make_shared<BehaviorNode>();
    defend_branch->type = NodeType::SEQUENCE;
    defend_branch->node_id = 5;
    
    auto health_check = std::make_shared<BehaviorNode>();
    health_check->type = NodeType::CONDITION;
    health_check->node_id = 6;
    health_check->condition = [this](const std::vector<int>& ctx) { return IsHealthLow(ctx); };
    
    auto defend_action = std::make_shared<BehaviorNode>();
    defend_action->type = NodeType::ACTION;
    defend_action->node_id = 7;
    defend_action->action_id = 2; // 防御动作
    defend_action->description = "执行防御";
    
    defend_branch->children = {health_check, defend_action};
    
    warrior_root->children = {attack_branch, defend_branch};
    npc_behavior_trees_[1] = warrior_root;
    
    // 可以添加更多NPC类型的行为树...
}

void AIDecisionEngine::InitializeActionWeights() {
    action_weights_[0] = 0.1f;  // 待机
    action_weights_[1] = 1.0f;  // 攻击
    action_weights_[2] = 0.8f;  // 防御
    action_weights_[3] = 0.6f;  // 技能
    action_weights_[4] = 0.3f;  // 移动
}

bool AIDecisionEngine::EvaluateNode(const std::shared_ptr<BehaviorNode>& node, 
                                   const std::vector<int>& context) {
    switch (node->type) {
        case NodeType::SELECTOR:
            // 选择节点：任一子节点成功即成功
            for (const auto& child : node->children) {
                if (EvaluateNode(child, context)) {
                    return true;
                }
            }
            return false;
            
        case NodeType::SEQUENCE:
            // 序列节点：所有子节点都成功才成功
            for (const auto& child : node->children) {
                if (!EvaluateNode(child, context)) {
                    return false;
                }
            }
            return true;
            
        case NodeType::CONDITION:
            // 条件节点：执行条件判断
            return node->condition ? node->condition(context) : false;
            
        case NodeType::ACTION:
            // 动作节点：总是成功
            return true;
    }
    return false;
}

int AIDecisionEngine::SelectBestAction(int npc_id, const std::vector<int>& context) {
    // 简化的动作选择逻辑
    (void)npc_id;  // 避免未使用参数警告
    
    if (IsEnemyNearby(context)) {
        return HasMana(context) ? 3 : 1; // 有魔法用技能，否则普攻
    } else if (IsHealthLow(context)) {
        return 2; // 防御
    }
    return 4; // 移动
}

std::string AIDecisionEngine::GenerateActionDescription(int action_id, int npc_id) {
    std::stringstream desc;
    desc << "NPC[" << npc_id << "] ";
    
    switch (action_id) {
        case 0: desc << "进入待机状态"; break;
        case 1: desc << "发起攻击"; break;
        case 2: desc << "采取防御姿态"; break;
        case 3: desc << "释放技能"; break;
        case 4: desc << "移动到新位置"; break;
        default: desc << "执行未知动作[" << action_id << "]"; break;
    }
    
    return desc.str();
}

bool AIDecisionEngine::IsEnemyNearby(const std::vector<int>& context) {
    // context[0] = 敌人距离 (简化假设)
    return !context.empty() && context[0] <= 5;
}

bool AIDecisionEngine::IsHealthLow(const std::vector<int>& context) {
    // context[1] = 当前血量百分比
    return context.size() > 1 && context[1] < 30;
}

bool AIDecisionEngine::HasMana(const std::vector<int>& context) {
    // context[2] = 当前魔法值
    return context.size() > 2 && context[2] > 50;
}

} // namespace algorithm
