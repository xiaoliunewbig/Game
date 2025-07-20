/*
 * 文件名: IAlgorithmService.h
 * 说明: 算法层对外服务接口，定义与策略层的数据交互方式。
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 本接口用于算法层与策略层之间的数据交互，包括伤害计算、AI决策等核心功能。
 * 定义了标准化的数据结构和服务接口，确保层间通信的一致性和可扩展性。
 * 采用抽象接口设计，支持多种算法实现的灵活切换。
 */
#ifndef ALGORITHM_IALGORITHMSERVICE_H
#define ALGORITHM_IALGORITHMSERVICE_H

#include <string>
#include <vector>

namespace algorithm {

/**
 * @brief 伤害计算请求参数
 * 
 * 包含进行伤害计算所需的所有基础信息。
 * 该结构体定义了战斗系统中伤害计算的标准输入格式。
 */
struct DamageRequest {
    int attacker_id;   ///< 攻击者ID，用于查询攻击者的属性信息（攻击力、等级等）
    int defender_id;   ///< 防御者ID，用于查询防御者的属性信息（防御力、抗性等）
    int skill_id;      ///< 技能ID，用于获取技能的伤害倍率、元素类型、特殊效果等
};

/**
 * @brief 伤害计算结果
 * 
 * 包含伤害计算的完整结果信息。
 * 不仅提供数值结果，还包含效果描述用于UI显示。
 */
struct DamageResult {
    int damage;            ///< 计算出的最终伤害值，已考虑所有修正因子
    std::string effect;    ///< 效果描述文本，如"暴击！"、"火元素灼烧"、"免疫"等
};

/**
 * @brief AI决策请求参数
 * 
 * 包含AI决策所需的环境信息和上下文数据。
 * 支持灵活的上下文信息传递，适应不同类型的AI决策需求。
 */
struct AIDecisionRequest {
    int npc_id;                ///< NPC编号，用于识别具体的NPC类型和行为模式
    std::vector<int> context;  ///< 决策上下文信息数组，包含环境状态、玩家信息等
                              ///< context[0]: 与玩家的距离（游戏单位）
                              ///< context[1]: NPC当前血量百分比（0-100）
                              ///< context[2]: 是否处于战斗状态（0=非战斗，1=战斗）
                              ///< context[3]: 玩家威胁等级（0-10）
                              ///< 更多上下文信息可根据需要扩展
};

/**
 * @brief AI决策结果
 * 
 * 包含AI决策的完整结果信息。
 * 提供行动ID和描述，支持游戏逻辑处理和UI显示。
 */
struct AIDecisionResult {
    int action_id;             ///< 推荐的行动编号
                              ///< 0: 待机/巡逻
                              ///< 1: 攻击玩家
                              ///< 2: 防御姿态
                              ///< 3: 使用技能
                              ///< 4: 移动到新位置
                              ///< 5: 逃跑
                              ///< 6: 寻求帮助
    std::string description;   ///< 行动的详细描述，用于日志记录和调试显示
};

/**
 * @brief 算法服务接口（抽象类）
 * 
 * 定义算法层对外提供的核心服务接口。
 * 该接口采用抽象基类设计，支持多种算法实现的灵活切换，
 * 为策略层提供稳定、统一的算法服务访问方式。
 * 
 * 主要服务包括：
 * - 战斗伤害计算服务
 * - NPC智能决策服务
 * - 未来可扩展更多算法服务
 */
class IAlgorithmService {
public:
    /**
     * @brief 虚析构函数
     * 
     * 确保派生类对象能够正确析构，避免资源泄漏。
     * 作为抽象基类，必须提供虚析构函数。
     */
    virtual ~IAlgorithmService() = default;
    
    /**
     * @brief 计算伤害
     * 
     * 纯虚函数，由具体实现类提供伤害计算功能。
     * 基于攻击者、防御者和技能信息计算最终伤害值。
     * 
     * @param request 伤害计算请求参数，包含所有必要的计算信息
     * @return DamageResult 伤害计算结果，包含伤害值和效果描述
     * 
     * @note 实现类应确保该方法的线程安全性
     * @note 如果计算失败，应返回合理的默认值而不是抛出异常
     */
    virtual DamageResult CalculateDamage(const DamageRequest& request) = 0;
    
    /**
     * @brief AI决策
     * 
     * 纯虚函数，由具体实现类提供AI决策功能。
     * 基于NPC信息和环境上下文，返回最适合的行动决策。
     * 
     * @param request AI决策请求参数，包含NPC信息和环境上下文
     * @return AIDecisionResult AI决策结果，包含推荐行动和描述
     * 
     * @note 实现类应基于行为树或状态机等AI算法进行决策
     * @note 决策应考虑NPC的性格、能力和当前状态
     * @note 如果决策失败，应返回安全的默认行为（如待机）
     */
    virtual AIDecisionResult MakeAIDecision(const AIDecisionRequest& request) = 0;
};

} // namespace algorithm

#endif // ALGORITHM_IALGORITHMSERVICE_H 
