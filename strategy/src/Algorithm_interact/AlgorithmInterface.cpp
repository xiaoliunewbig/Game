/*
 * 文件名: AlgorithmInterface.cpp
 * 说明: 策略层与算法层交互接口的具体实现。
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 *
 * 本文件实现策略层与算法层的数据交互，包括伤害计算请求、AI决策请求等，
 * 实现两层之间的解耦通信。提供完整的错误处理和状态管理功能，
 * 确保算法服务调用的稳定性和可靠性。
 */
#include "Algorithm_interact/AlgorithmInterface.h"
#include "../../../algorithm/include/AlgorithmService.h"
#include "../../../algorithm/include/DamageCalculator.h"
#include "../../../algorithm/include/SkillTreeManager.h"
#include "../../../algorithm/include/AIDecisionEngine.h"

namespace strategy {

/**
 * @brief 构造函数实现
 * 
 * 初始化成员变量，设置初始状态为未初始化。
 * 算法服务指针初始化为nullptr，等待Initialize()调用。
 */
AlgorithmInterface::AlgorithmInterface() : is_initialized_(false) {
    // 算法服务指针将在Initialize()中创建
    // 初始状态设置为未初始化，确保使用前必须调用Initialize()
}

/**
 * @brief 析构函数实现
 * 
 * 使用默认析构函数，unique_ptr会自动清理算法服务对象。
 * 确保资源正确释放，避免内存泄漏。
 */
AlgorithmInterface::~AlgorithmInterface() = default;

/**
 * @brief 初始化算法服务连接实现
 * 
 * 创建算法服务实例并设置初始化状态。
 * 使用异常处理确保初始化过程的安全性。
 * 
 * @return bool 初始化结果
 */
bool AlgorithmInterface::Initialize() {
    try {
        // 创建算法服务实例
        CreateAlgorithmService();
        
        // 检查服务是否创建成功
        is_initialized_ = (algorithm_service_ != nullptr);
        
        return is_initialized_;
    } catch (const std::exception&) {
        // 初始化失败，重置状态
        is_initialized_ = false;
        return false;
    }
}

/**
 * @brief 伤害计算请求实现
 * 
 * 向算法层发送伤害计算请求，处理各种异常情况。
 * 如果服务不可用，返回安全的默认值。
 * 
 * @param attacker_id 攻击者ID
 * @param defender_id 防御者ID  
 * @param skill_id 技能ID
 * @return algorithm::DamageResult 伤害计算结果
 */
algorithm::DamageResult AlgorithmInterface::RequestDamageCalculation(int attacker_id, int defender_id, int skill_id) {
    // 创建默认结果，用于服务不可用时返回
    algorithm::DamageResult default_result;
    default_result.damage = 0;
    default_result.effect = "算法服务不可用";
    
    // 检查算法服务是否可用
    if (!IsAlgorithmServiceAvailable()) {
        return default_result;
    }
    
    try {
        // 构建伤害计算请求
        algorithm::DamageRequest request;
        request.attacker_id = attacker_id;
        request.defender_id = defender_id;
        request.skill_id = skill_id;
        
        // 调用算法服务进行伤害计算
        return algorithm_service_->CalculateDamage(request);
    } catch (const std::exception&) {
        // 计算过程中发生异常，返回默认结果
        return default_result;
    }
}

/**
 * @brief AI决策请求实现
 * 
 * 向算法层发送AI决策请求，获取NPC行为决策。
 * 提供完整的错误处理和默认行为。
 * 
 * @param npc_id NPC编号
 * @param context 决策上下文信息
 * @return algorithm::AIDecisionResult AI决策结果
 */
algorithm::AIDecisionResult AlgorithmInterface::RequestAIDecision(int npc_id, const std::vector<int>& context) {
    // 创建默认决策结果
    algorithm::AIDecisionResult default_result;
    default_result.action_id = 0;  // 0表示待机状态
    default_result.description = "算法服务不可用";
    
    // 检查算法服务状态
    if (!IsAlgorithmServiceAvailable()) {
        return default_result;
    }
    
    try {
        // 构建AI决策请求
        algorithm::AIDecisionRequest request;
        request.npc_id = npc_id;
        request.context = context;
        
        // 调用算法服务进行AI决策
        return algorithm_service_->MakeAIDecision(request);
    } catch (const std::exception&) {
        // 决策过程中发生异常，返回默认行为
        return default_result;
    }
}

/**
 * @brief 批量AI决策请求实现
 * 
 * 为多个NPC批量处理AI决策请求，提高处理效率。
 * 验证输入参数的有效性，确保数据一致性。
 * 
 * @param npc_ids NPC编号列表
 * @param contexts 对应的上下文信息列表
 * @return std::vector<algorithm::AIDecisionResult> 决策结果列表
 */
std::vector<algorithm::AIDecisionResult> AlgorithmInterface::RequestBatchAIDecision(
    const std::vector<int>& npc_ids, 
    const std::vector<std::vector<int>>& contexts) {
    
    std::vector<algorithm::AIDecisionResult> results;
    
    // 验证输入参数：NPC列表和上下文列表大小必须匹配
    if (npc_ids.size() != contexts.size()) {
        return results;  // 参数不匹配，返回空结果
    }
    
    // 为每个NPC进行决策处理
    for (size_t i = 0; i < npc_ids.size(); ++i) {
        algorithm::AIDecisionResult result = RequestAIDecision(npc_ids[i], contexts[i]);
        results.push_back(result);
    }
    
    return results;
}

/**
 * @brief 算法服务可用性检查实现
 * 
 * 检查算法服务的当前状态，确认是否可以正常使用。
 * 
 * @return bool 服务可用性状态
 */
bool AlgorithmInterface::IsAlgorithmServiceAvailable() const {
    // 检查初始化状态和服务对象是否存在
    return is_initialized_ && algorithm_service_ != nullptr;
}

/**
 * @brief 创建算法服务实例实现
 * 
 * 内部方法，负责创建具体的算法服务对象。
 * 使用make_unique确保异常安全和资源管理。
 */
void AlgorithmInterface::CreateAlgorithmService() {
    // 创建算法服务实例
    algorithm_service_ = std::make_unique<algorithm::AlgorithmService>();
}

} // namespace strategy
