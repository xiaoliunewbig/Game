/*
 * 文件名: AlgorithmInterface.cpp
 * 说明: 策略层与算法层交互接口的具体实现。
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 *
 * 本文件实现策略层与算法层的数据交互，包括伤害计算请求、AI决策请求等，
 * 实现两层之间的解耦通信。
 */
#include "Algorithm_interact/AlgorithmInterface.h"
#include "../../../algorithm/include/AlgorithmService.h"
#include "../../../algorithm/include/DamageCalculator.h"
#include "../../../algorithm/include/SkillTreeManager.h"
#include "../../../algorithm/include/AIDecisionEngine.h"

namespace strategy {

AlgorithmInterface::AlgorithmInterface() : is_initialized_(false) {
}

AlgorithmInterface::~AlgorithmInterface() = default;

bool AlgorithmInterface::Initialize() {
    try {
        CreateAlgorithmService();
        is_initialized_ = (algorithm_service_ != nullptr);
        return is_initialized_;
    } catch (const std::exception&) {
        is_initialized_ = false;
        return false;
    }
}

algorithm::DamageResult AlgorithmInterface::RequestDamageCalculation(int attacker_id, int defender_id, int skill_id) {
    algorithm::DamageResult default_result;
    default_result.damage = 0;
    default_result.effect = "算法服务不可用";
    
    if (!IsAlgorithmServiceAvailable()) {
        return default_result;
    }
    
    try {
        algorithm::DamageRequest request;
        request.attacker_id = attacker_id;
        request.defender_id = defender_id;
        request.skill_id = skill_id;
        
        return algorithm_service_->CalculateDamage(request);
    } catch (const std::exception&) {
        return default_result;
    }
}

algorithm::AIDecisionResult AlgorithmInterface::RequestAIDecision(int npc_id, const std::vector<int>& context) {
    algorithm::AIDecisionResult default_result;
    default_result.action_id = 0;
    default_result.description = "算法服务不可用";
    
    if (!IsAlgorithmServiceAvailable()) {
        return default_result;
    }
    
    try {
        algorithm::AIDecisionRequest request;
        request.npc_id = npc_id;
        request.context = context;
        
        return algorithm_service_->MakeAIDecision(request);
    } catch (const std::exception&) {
        return default_result;
    }
}

std::vector<algorithm::AIDecisionResult> AlgorithmInterface::RequestBatchAIDecision(
    const std::vector<int>& npc_ids, 
    const std::vector<std::vector<int>>& contexts) {
    
    std::vector<algorithm::AIDecisionResult> results;
    
    if (npc_ids.size() != contexts.size()) {
        return results;  // 参数不匹配
    }
    
    for (size_t i = 0; i < npc_ids.size(); ++i) {
        algorithm::AIDecisionResult result = RequestAIDecision(npc_ids[i], contexts[i]);
        results.push_back(result);
    }
    
    return results;
}

bool AlgorithmInterface::IsAlgorithmServiceAvailable() const {
    return is_initialized_ && algorithm_service_ != nullptr;
}

void AlgorithmInterface::CreateAlgorithmService() {
    algorithm_service_ = std::make_unique<algorithm::AlgorithmService>();
}

} // namespace strategy
