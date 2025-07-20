/*
 * 文件名: AlgorithmInterface.h
 * 说明: 策略层与算法层的交互接口，封装算法服务调用。
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 本类封装对算法层服务的调用，提供伤害计算、AI决策等功能的访问接口，
 * 实现策略层与算法层的解耦。通过统一的接口管理算法服务的生命周期，
 * 并提供错误处理和服务状态检查功能。
 */
#ifndef STRATEGY_ALGORITHMINTERFACE_H
#define STRATEGY_ALGORITHMINTERFACE_H

#include <memory>
#include <vector>
#include <string>

// 前向声明算法层接口
namespace algorithm {
    class IAlgorithmService;    ///< 算法服务接口
    struct DamageRequest;       ///< 伤害计算请求结构
    struct DamageResult;        ///< 伤害计算结果结构
    struct AIDecisionRequest;   ///< AI决策请求结构
    struct AIDecisionResult;    ///< AI决策结果结构
}

namespace strategy {

/**
 * @brief 算法层交互接口
 * 
 * 该类作为策略层与算法层之间的桥梁，封装了所有对算法服务的调用。
 * 主要功能包括：
 * - 管理算法服务的连接和生命周期
 * - 提供伤害计算服务的访问接口
 * - 提供AI决策服务的访问接口
 * - 支持批量操作以提高性能
 * - 提供服务状态检查和错误处理
 */
class AlgorithmInterface {
public:
    /**
     * @brief 构造函数
     * 
     * 初始化算法接口对象，设置初始状态为未初始化。
     * 不会立即创建算法服务连接，需要调用Initialize()方法。
     */
    AlgorithmInterface();

    /**
     * @brief 析构函数
     * 
     * 清理资源，断开算法服务连接，释放相关内存。
     * 确保算法服务对象被正确销毁。
     */
    ~AlgorithmInterface();

    /**
     * @brief 初始化算法服务连接
     * 
     * 创建并初始化算法服务实例，建立与算法层的连接。
     * 该方法必须在使用其他功能之前调用。
     * 
     * @return bool 初始化是否成功
     * @retval true 初始化成功，算法服务可用
     * @retval false 初始化失败，算法服务不可用
     * 
     * @note 如果初始化失败，后续的算法调用将返回默认值
     * @see IsAlgorithmServiceAvailable()
     */
    bool Initialize();

    /**
     * @brief 请求伤害计算
     * 
     * 向算法层发送伤害计算请求，获取战斗中的伤害数值。
     * 计算考虑攻击者属性、防御者属性、技能效果等因素。
     * 
     * @param attacker_id 攻击者ID，用于获取攻击者属性
     * @param defender_id 防御者ID，用于获取防御者属性
     * @param skill_id 技能ID，用于获取技能伤害倍率和特效
     * 
     * @return algorithm::DamageResult 伤害计算结果
     * @retval DamageResult.damage 计算出的伤害数值
     * @retval DamageResult.effect 伤害效果描述（如暴击、元素反应等）
     * 
     * @note 如果算法服务不可用，返回默认结果（伤害为0）
     * @see IsAlgorithmServiceAvailable()
     */
    algorithm::DamageResult RequestDamageCalculation(int attacker_id, int defender_id, int skill_id);

    /**
     * @brief 请求AI决策
     * 
     * 向算法层发送AI决策请求，获取NPC的行为决策。
     * 基于当前游戏状态和上下文信息，返回最适合的行动方案。
     * 
     * @param npc_id NPC编号，用于识别具体的NPC和其行为模式
     * @param context 决策上下文信息，包含环境状态、玩家位置等
     *                context[0]: 通常为玩家距离
     *                context[1]: 通常为NPC当前血量百分比
     *                context[2]: 通常为战斗状态标志
     * 
     * @return algorithm::AIDecisionResult AI决策结果
     * @retval AIDecisionResult.action_id 推荐的行动ID
     * @retval AIDecisionResult.description 行动描述文本
     * 
     * @note 如果算法服务不可用，返回默认行为（待机状态）
     * @see RequestBatchAIDecision() 批量决策接口
     */
    algorithm::AIDecisionResult RequestAIDecision(int npc_id, const std::vector<int>& context);

    /**
     * @brief 批量AI决策请求
     * 
     * 一次性为多个NPC请求AI决策，提高批量处理效率。
     * 适用于需要同时处理多个NPC行为的场景，如大规模战斗。
     * 
     * @param npc_ids NPC编号列表，包含所有需要决策的NPC
     * @param contexts 对应的上下文信息列表，与npc_ids一一对应
     * 
     * @return std::vector<algorithm::AIDecisionResult> AI决策结果列表
     * 
     * @pre npc_ids.size() == contexts.size() 两个参数数组大小必须相等
     * @post 返回结果数组大小等于输入NPC数量
     * 
     * @note 如果参数数组大小不匹配，返回空结果列表
     * @note 内部实现为循环调用RequestAIDecision()，未来可优化为真正的批量处理
     */
    std::vector<algorithm::AIDecisionResult> RequestBatchAIDecision(
        const std::vector<int>& npc_ids, 
        const std::vector<std::vector<int>>& contexts);

    /**
     * @brief 检查算法服务是否可用
     * 
     * 检查算法服务的当前状态，确认是否可以正常提供服务。
     * 用于在调用算法功能前进行状态验证。
     * 
     * @return bool 算法服务可用性状态
     * @retval true 算法服务已初始化且可用
     * @retval false 算法服务未初始化或不可用
     * 
     * @note 该方法不会尝试重新连接服务，仅检查当前状态
     * @see Initialize() 初始化服务连接
     */
    bool IsAlgorithmServiceAvailable() const;

private:
    /**
     * @brief 算法服务实例指针
     * 
     * 指向算法层服务实现的智能指针，管理算法服务的生命周期。
     * 使用unique_ptr确保资源的自动管理和异常安全。
     */
    std::unique_ptr<algorithm::IAlgorithmService> algorithm_service_;
    
    /**
     * @brief 初始化状态标志
     * 
     * 标记算法接口是否已经成功初始化。
     * true表示已初始化且算法服务可用，false表示未初始化或初始化失败。
     */
    bool is_initialized_;
    
    /**
     * @brief 创建算法服务实例
     * 
     * 内部方法，负责创建具体的算法服务实现对象。
     * 封装了算法服务的创建逻辑，隐藏实现细节。
     * 
     * @note 该方法由Initialize()调用，不应直接使用
     * @throws std::exception 如果服务创建失败可能抛出异常
     */
    void CreateAlgorithmService();
};

} // namespace strategy

#endif // STRATEGY_ALGORITHMINTERFACE_H
