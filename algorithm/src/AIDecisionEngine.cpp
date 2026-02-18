/*
 * 文件名: AIDecisionEngine.cpp
 * 说明: AI决策引擎实现 — 5种行为树、决策记忆、Boss阶段转换
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 * 更新时间: 2025-07-24 — 全面重写
 *
 * 行为树类型:
 *   Warrior  — 近战优先，低血防御/用药
 *   Mage     — 远程法术优先，低蓝普攻
 *   Archer   — 中远程，低血闪避/陷阱
 *   Boss     — 阶段转换（正常/狂暴/绝望）
 *   Passive  — 优先治疗/防御，被攻击时反击
 *
 * context 约定:
 *   [0] = 与玩家距离
 *   [1] = NPC血量百分比 (0-100)
 *   [2] = 是否战斗状态 (0/1)
 *   [3] = 玩家威胁等级 (0-10)
 *   [4] = NPC蓝量百分比 (0-100)（可选）
 */
#include "AIDecisionEngine.h"
#include <random>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace algorithm {

static thread_local std::mt19937 s_ai_rng{std::random_device{}()};

// ============================================================================
// 构造
// ============================================================================
AIDecisionEngine::AIDecisionEngine() {
    InitializeBehaviorTrees();
}

void AIDecisionEngine::RegisterNPCType(int npc_id, NPCType type) {
    npc_types_[npc_id] = type;
}

void AIDecisionEngine::ClearMemory(int npc_id) {
    if (npc_id == 0) {
        decision_memory_.clear();
    } else {
        decision_memory_.erase(npc_id);
    }
}

// ============================================================================
// 主决策入口
// ============================================================================
AIDecisionResult AIDecisionEngine::MakeDecision(const AIDecisionRequest& request) {
    AIDecisionResult result;

    // 确定NPC类型
    NPCType type = NPCType::Warrior;  // 默认战士
    auto type_it = npc_types_.find(request.npc_id);
    if (type_it != npc_types_.end()) {
        type = type_it->second;
    }

    // 查找行为树
    int tree_key = static_cast<int>(type);
    auto tree_it = behavior_trees_.find(tree_key);
    if (tree_it == behavior_trees_.end()) {
        result.action_id = 0;
        result.description = "NPC待机中（无行为树）";
        return result;
    }

    // 评估行为树
    if (EvaluateNode(tree_it->second, request.context)) {
        result.action_id = SelectBestAction(request.npc_id, type, request.context);
    } else {
        result.action_id = 0;  // 待机
    }

    // 记录决策
    RecordDecision(request.npc_id, result.action_id);

    // 生成描述
    result.description = GenerateActionDescription(result.action_id, request.npc_id, type);

    return result;
}

// ============================================================================
// 辅助构建方法
// ============================================================================
std::shared_ptr<AIDecisionEngine::BehaviorNode> AIDecisionEngine::MakeSelector(
    int id, std::vector<std::shared_ptr<BehaviorNode>> children) {
    auto node = std::make_shared<BehaviorNode>();
    node->type = NodeType::SELECTOR;
    node->node_id = id;
    node->children = std::move(children);
    return node;
}

std::shared_ptr<AIDecisionEngine::BehaviorNode> AIDecisionEngine::MakeSequence(
    int id, std::vector<std::shared_ptr<BehaviorNode>> children) {
    auto node = std::make_shared<BehaviorNode>();
    node->type = NodeType::SEQUENCE;
    node->node_id = id;
    node->children = std::move(children);
    return node;
}

std::shared_ptr<AIDecisionEngine::BehaviorNode> AIDecisionEngine::MakeCondition(
    int id, std::function<bool(const std::vector<int>&)> cond) {
    auto node = std::make_shared<BehaviorNode>();
    node->type = NodeType::CONDITION;
    node->node_id = id;
    node->condition = std::move(cond);
    return node;
}

std::shared_ptr<AIDecisionEngine::BehaviorNode> AIDecisionEngine::MakeAction(
    int id, int action_id, const std::string& desc) {
    auto node = std::make_shared<BehaviorNode>();
    node->type = NodeType::ACTION;
    node->node_id = id;
    node->action_id = action_id;
    node->description = desc;
    return node;
}

// ============================================================================
// 初始化所有行为树
// ============================================================================
void AIDecisionEngine::InitializeBehaviorTrees() {
    behavior_trees_[static_cast<int>(NPCType::Warrior)] = BuildWarriorTree();
    behavior_trees_[static_cast<int>(NPCType::Mage)]    = BuildMageTree();
    behavior_trees_[static_cast<int>(NPCType::Archer)]  = BuildArcherTree();
    behavior_trees_[static_cast<int>(NPCType::Boss)]    = BuildBossTree();
    behavior_trees_[static_cast<int>(NPCType::Passive)] = BuildPassiveTree();
}

void AIDecisionEngine::InitializeActionWeights() {
    // 权重现在通过行为树结构体现，不再单独管理
}

// ============================================================================
// 战士行为树: 近战优先，低血防御/用药，高血冲锋
// ============================================================================
std::shared_ptr<AIDecisionEngine::BehaviorNode> AIDecisionEngine::BuildWarriorTree() {
    return MakeSelector(1, {
        // 分支1: 危急 — 血量<20%时防御
        MakeSequence(10, {
            MakeCondition(11, [this](const auto& ctx) { return IsHealthCritical(ctx); }),
            MakeAction(12, 2, "采取防御姿态")
        }),
        // 分支2: 低血 — 血量<40%时用药或防御
        MakeSequence(20, {
            MakeCondition(21, [this](const auto& ctx) { return IsHealthLow(ctx); }),
            MakeSelector(22, {
                MakeAction(23, 5, "使用药品恢复"),
                MakeAction(24, 2, "采取防御姿态")
            })
        }),
        // 分支3: 近距离 — 攻击
        MakeSequence(30, {
            MakeCondition(31, [this](const auto& ctx) { return IsEnemyNearby(ctx); }),
            MakeSelector(32, {
                // 有蓝用技能
                MakeSequence(33, {
                    MakeCondition(34, [this](const auto& ctx) { return HasMana(ctx); }),
                    MakeAction(35, 3, "释放战士技能")
                }),
                // 无蓝普攻
                MakeAction(36, 1, "发起物理攻击")
            })
        }),
        // 分支4: 远距离 — 冲锋
        MakeSequence(40, {
            MakeCondition(41, [this](const auto& ctx) { return IsEnemyFar(ctx); }),
            MakeAction(42, 4, "冲锋接近敌人")
        }),
        // 分支5: 默认 — 巡逻
        MakeAction(50, 0, "巡逻待机")
    });
}

// ============================================================================
// 法师行为树: 远程法术优先，低蓝普攻
// ============================================================================
std::shared_ptr<AIDecisionEngine::BehaviorNode> AIDecisionEngine::BuildMageTree() {
    return MakeSelector(100, {
        // 分支1: 危急 — 瞬移逃跑
        MakeSequence(110, {
            MakeCondition(111, [this](const auto& ctx) { return IsHealthCritical(ctx); }),
            MakeCondition(112, [this](const auto& ctx) { return IsEnemyNearby(ctx); }),
            MakeAction(113, 5, "瞬移逃跑")
        }),
        // 分支2: 低血 — 治疗或护盾
        MakeSequence(120, {
            MakeCondition(121, [this](const auto& ctx) { return IsHealthLow(ctx); }),
            MakeSelector(122, {
                MakeSequence(123, {
                    MakeCondition(124, [this](const auto& ctx) { return HasMana(ctx); }),
                    MakeAction(125, 7, "施放治疗术")
                }),
                MakeAction(126, 2, "防御")
            })
        }),
        // 分支3: 有蓝量 — 释放法术
        MakeSequence(130, {
            MakeCondition(131, [this](const auto& ctx) { return HasMana(ctx); }),
            MakeCondition(132, [this](const auto& ctx) { return IsInCombat(ctx); }),
            MakeAction(133, 3, "释放法师技能")
        }),
        // 分支4: 低蓝 — 普攻
        MakeSequence(140, {
            MakeCondition(141, [this](const auto& ctx) { return HasLowMana(ctx); }),
            MakeAction(142, 1, "普通攻击节省蓝量")
        }),
        // 分支5: 敌人远 — 接近
        MakeSequence(150, {
            MakeCondition(151, [this](const auto& ctx) { return IsEnemyFar(ctx); }),
            MakeAction(152, 4, "移动到施法距离")
        }),
        // 默认
        MakeAction(160, 0, "待机冥想")
    });
}

// ============================================================================
// 弓箭手行为树: 中远程，低血闪避/陷阱，高血狙击
// ============================================================================
std::shared_ptr<AIDecisionEngine::BehaviorNode> AIDecisionEngine::BuildArcherTree() {
    return MakeSelector(200, {
        // 分支1: 近身危急 — 闪避后退
        MakeSequence(210, {
            MakeCondition(211, [this](const auto& ctx) { return IsEnemyNearby(ctx); }),
            MakeCondition(212, [this](const auto& ctx) { return IsHealthLow(ctx); }),
            MakeAction(213, 5, "后跳闪避脱离")
        }),
        // 分支2: 敌人近身 — 放陷阱+闪避
        MakeSequence(220, {
            MakeCondition(221, [this](const auto& ctx) { return IsEnemyNearby(ctx); }),
            MakeSelector(222, {
                MakeAction(223, 8, "放置陷阱"),
                MakeAction(224, 5, "闪避后退")
            })
        }),
        // 分支3: 远距离+高血 — 狙击
        MakeSequence(230, {
            MakeCondition(231, [this](const auto& ctx) { return IsEnemyFar(ctx); }),
            MakeCondition(232, [this](const auto& ctx) { return IsHealthHigh(ctx); }),
            MakeAction(233, 3, "远距离狙击")
        }),
        // 分支4: 中距离 — 快速射击
        MakeSequence(240, {
            MakeCondition(241, [this](const auto& ctx) { return IsInCombat(ctx); }),
            MakeSelector(242, {
                MakeSequence(243, {
                    MakeCondition(244, [this](const auto& ctx) { return HasMana(ctx); }),
                    MakeAction(245, 3, "释放射击技能")
                }),
                MakeAction(246, 1, "快速射击")
            })
        }),
        // 默认
        MakeAction(250, 0, "潜行巡逻")
    });
}

// ============================================================================
// Boss行为树: 阶段转换（>50%正常 / ≤50%狂暴 / ≤20%绝望）
// ============================================================================
std::shared_ptr<AIDecisionEngine::BehaviorNode> AIDecisionEngine::BuildBossTree() {
    return MakeSelector(300, {
        // 阶段3: 绝望模式 (HP ≤ 20%) — 必杀技
        MakeSequence(310, {
            MakeCondition(311, [this](const auto& ctx) { return IsHealthCritical(ctx); }),
            MakeAction(312, 9, "Boss绝望必杀技")
        }),
        // 阶段2: 狂暴模式 (HP ≤ 50%) — 高频技能
        MakeSequence(320, {
            MakeCondition(321, [this](const auto& ctx) { return IsHealthLow(ctx); }),
            MakeSelector(322, {
                MakeSequence(323, {
                    MakeCondition(324, [this](const auto& ctx) { return HasMana(ctx); }),
                    MakeAction(325, 3, "Boss狂暴连续技能")
                }),
                MakeAction(326, 1, "Boss狂暴攻击")
            })
        }),
        // 阶段1: 正常模式 (HP > 50%)
        MakeSequence(330, {
            MakeCondition(331, [this](const auto& ctx) { return IsHealthHigh(ctx); }),
            MakeSelector(332, {
                MakeSequence(333, {
                    MakeCondition(334, [this](const auto& ctx) { return IsEnemyNearby(ctx); }),
                    MakeAction(335, 1, "Boss普通攻击")
                }),
                MakeSequence(336, {
                    MakeCondition(337, [this](const auto& ctx) { return HasMana(ctx); }),
                    MakeAction(338, 3, "Boss释放技能")
                }),
                MakeAction(339, 4, "Boss移动")
            })
        }),
        // 默认
        MakeAction(340, 1, "Boss攻击")
    });
}

// ============================================================================
// 被动NPC行为树: 优先治疗/防御，仅被攻击时反击
// ============================================================================
std::shared_ptr<AIDecisionEngine::BehaviorNode> AIDecisionEngine::BuildPassiveTree() {
    return MakeSelector(400, {
        // 分支1: 自身低血 — 治疗
        MakeSequence(410, {
            MakeCondition(411, [this](const auto& ctx) { return IsHealthLow(ctx); }),
            MakeSelector(412, {
                MakeSequence(413, {
                    MakeCondition(414, [this](const auto& ctx) { return HasMana(ctx); }),
                    MakeAction(415, 7, "自我治疗")
                }),
                MakeAction(416, 2, "防御姿态")
            })
        }),
        // 分支2: 被攻击（高威胁） — 反击
        MakeSequence(420, {
            MakeCondition(421, [this](const auto& ctx) { return IsHighThreat(ctx); }),
            MakeCondition(422, [this](const auto& ctx) { return IsInCombat(ctx); }),
            MakeAction(423, 1, "被迫反击")
        }),
        // 分支3: 战斗中 — 防御
        MakeSequence(430, {
            MakeCondition(431, [this](const auto& ctx) { return IsInCombat(ctx); }),
            MakeAction(432, 2, "防御姿态")
        }),
        // 默认: 和平待机
        MakeAction(440, 0, "和平待机")
    });
}

// ============================================================================
// 行为树评估
// ============================================================================
bool AIDecisionEngine::EvaluateNode(const std::shared_ptr<BehaviorNode>& node,
                                    const std::vector<int>& context) {
    if (!node) return false;

    switch (node->type) {
        case NodeType::SELECTOR:
            for (const auto& child : node->children) {
                if (EvaluateNode(child, context)) return true;
            }
            return false;

        case NodeType::SEQUENCE:
            for (const auto& child : node->children) {
                if (!EvaluateNode(child, context)) return false;
            }
            return true;

        case NodeType::CONDITION:
            return node->condition ? node->condition(context) : false;

        case NodeType::ACTION:
            return true;
    }
    return false;
}

// ============================================================================
// 动作选择（含反重复惩罚）
// ============================================================================
int AIDecisionEngine::SelectBestAction(int npc_id, NPCType type, const std::vector<int>& context) {
    // 候选动作和基础权重
    struct Candidate {
        int action_id;
        float weight;
    };

    std::vector<Candidate> candidates;

    switch (type) {
        case NPCType::Warrior:
            if (IsHealthCritical(context)) {
                candidates = {{2, 1.0f}, {5, 0.8f}};
            } else if (IsHealthLow(context)) {
                candidates = {{5, 1.0f}, {2, 0.8f}, {1, 0.3f}};
            } else if (IsEnemyNearby(context)) {
                candidates = {{3, 1.0f}, {1, 0.9f}, {3, 0.7f}};
            } else {
                candidates = {{4, 1.0f}, {0, 0.5f}};
            }
            break;

        case NPCType::Mage:
            if (IsHealthCritical(context) && IsEnemyNearby(context)) {
                candidates = {{5, 1.0f}};
            } else if (IsHealthLow(context)) {
                candidates = {{7, 1.0f}, {2, 0.6f}};
            } else if (HasMana(context)) {
                candidates = {{3, 1.0f}, {3, 0.8f}};
            } else {
                candidates = {{1, 1.0f}, {4, 0.5f}};
            }
            break;

        case NPCType::Archer:
            if (IsEnemyNearby(context) && IsHealthLow(context)) {
                candidates = {{5, 1.0f}, {8, 0.7f}};
            } else if (IsEnemyFar(context)) {
                candidates = {{3, 1.0f}};
            } else {
                candidates = {{1, 1.0f}, {3, 0.8f}};
            }
            break;

        case NPCType::Boss: {
            int phase = GetBossPhase(context);
            if (phase == 3) {
                candidates = {{9, 1.0f}, {3, 0.5f}};
            } else if (phase == 2) {
                candidates = {{3, 1.0f}, {1, 0.8f}, {3, 0.6f}};
            } else {
                candidates = {{1, 1.0f}, {3, 0.7f}, {4, 0.3f}};
            }
            break;
        }

        case NPCType::Passive:
            if (IsHealthLow(context)) {
                candidates = {{7, 1.0f}, {2, 0.8f}};
            } else if (IsHighThreat(context) && IsInCombat(context)) {
                candidates = {{1, 1.0f}};
            } else {
                candidates = {{0, 1.0f}, {2, 0.5f}};
            }
            break;
    }

    if (candidates.empty()) {
        return 0;
    }

    // 应用反重复惩罚
    for (auto& c : candidates) {
        c.weight *= GetRepetitionPenalty(npc_id, c.action_id);
    }

    // 加权随机选择
    float total_weight = 0.0f;
    for (const auto& c : candidates) {
        total_weight += c.weight;
    }

    if (total_weight <= 0.0f) {
        return candidates[0].action_id;
    }

    std::uniform_real_distribution<float> dist(0.0f, total_weight);
    float roll = dist(s_ai_rng);
    float cumulative = 0.0f;

    for (const auto& c : candidates) {
        cumulative += c.weight;
        if (roll <= cumulative) {
            return c.action_id;
        }
    }

    return candidates.back().action_id;
}

// ============================================================================
// 反重复惩罚
// ============================================================================
float AIDecisionEngine::GetRepetitionPenalty(int npc_id, int action_id) const {
    auto it = decision_memory_.find(npc_id);
    if (it == decision_memory_.end()) return 1.0f;

    int count = 0;
    for (int past : it->second) {
        if (past == action_id) count++;
    }

    // 每重复一次权重×0.6
    return std::pow(0.6f, static_cast<float>(count));
}

void AIDecisionEngine::RecordDecision(int npc_id, int action_id) {
    auto& memory = decision_memory_[npc_id];
    memory.push_back(action_id);
    while (static_cast<int>(memory.size()) > MEMORY_SIZE) {
        memory.pop_front();
    }
}

// ============================================================================
// Boss阶段判定
// ============================================================================
int AIDecisionEngine::GetBossPhase(const std::vector<int>& context) const {
    int hp_percent = (context.size() > 1) ? context[1] : 100;
    if (hp_percent <= 20) return 3;  // 绝望
    if (hp_percent <= 50) return 2;  // 狂暴
    return 1;                         // 正常
}

// ============================================================================
// 条件判断函数
// ============================================================================
bool AIDecisionEngine::IsEnemyNearby(const std::vector<int>& context) const {
    return !context.empty() && context[0] <= 5;
}

bool AIDecisionEngine::IsEnemyFar(const std::vector<int>& context) const {
    return !context.empty() && context[0] > 10;
}

bool AIDecisionEngine::IsHealthLow(const std::vector<int>& context) const {
    return context.size() > 1 && context[1] < 40;
}

bool AIDecisionEngine::IsHealthCritical(const std::vector<int>& context) const {
    return context.size() > 1 && context[1] <= 20;
}

bool AIDecisionEngine::IsHealthHigh(const std::vector<int>& context) const {
    return context.size() > 1 && context[1] > 50;
}

bool AIDecisionEngine::HasMana(const std::vector<int>& context) const {
    // context[4] = 蓝量百分比（可选，默认充足）
    if (context.size() > 4) {
        return context[4] > 20;
    }
    return true;  // 无蓝量信息时默认有蓝
}

bool AIDecisionEngine::HasLowMana(const std::vector<int>& context) const {
    if (context.size() > 4) {
        return context[4] <= 20;
    }
    return false;
}

bool AIDecisionEngine::IsInCombat(const std::vector<int>& context) const {
    return context.size() > 2 && context[2] == 1;
}

bool AIDecisionEngine::IsHighThreat(const std::vector<int>& context) const {
    return context.size() > 3 && context[3] >= 7;
}

// ============================================================================
// 动作描述生成
// ============================================================================
std::string AIDecisionEngine::GenerateActionDescription(int action_id, int npc_id, NPCType type) const {
    std::stringstream desc;

    // NPC类型名称
    const char* type_name = "NPC";
    switch (type) {
        case NPCType::Warrior: type_name = "战士"; break;
        case NPCType::Mage:    type_name = "法师"; break;
        case NPCType::Archer:  type_name = "弓箭手"; break;
        case NPCType::Boss:    type_name = "Boss"; break;
        case NPCType::Passive: type_name = "NPC"; break;
    }

    desc << type_name << "[" << npc_id << "] ";

    switch (action_id) {
        case 0: desc << "进入待机状态"; break;
        case 1: desc << "发起攻击"; break;
        case 2: desc << "采取防御姿态"; break;
        case 3: desc << "释放技能"; break;
        case 4: desc << "移动到新位置"; break;
        case 5: desc << "紧急回避/逃跑"; break;
        case 6: desc << "寻求援助"; break;
        case 7: desc << "施放治疗"; break;
        case 8: desc << "放置陷阱"; break;
        case 9: desc << "释放必杀技"; break;
        default: desc << "执行未知动作[" << action_id << "]"; break;
    }

    return desc.str();
}

} // namespace algorithm
