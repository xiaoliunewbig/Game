/*
 * 文件名: AlgorithmGrpcServiceImpl.cpp
 * 说明: 算法层 gRPC 服务实现
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#include "AlgorithmGrpcServiceImpl.h"
#include "SkillTreeManager.h"
#include "InputValidator.h"
#include "CharacterStats.h"
#include <iostream>

namespace algorithm {

AlgorithmGrpcServiceImpl::AlgorithmGrpcServiceImpl() {
    std::cout << "AlgorithmGrpcServiceImpl: 服务初始化" << std::endl;
}

grpc::Status AlgorithmGrpcServiceImpl::CalculateDamage(
    grpc::ServerContext* context,
    const algorithm_proto::CalculationRequest* request,
    algorithm_proto::DamageResult* response)
{
    (void)context;

    DamageRequest internal_request;
    internal_request.attacker_id = request->attacker_id();
    internal_request.defender_id = request->defender_id();
    internal_request.skill_id = request->skill_id();

    auto result = service_.CalculateDamage(internal_request);

    response->set_damage(result.damage);
    response->set_effect(result.effect);
    response->set_is_critical(result.effect.find("暴击") != std::string::npos);
    response->set_element_multiplier(1.0f);

    return grpc::Status::OK;
}

grpc::Status AlgorithmGrpcServiceImpl::AIActionDecision(
    grpc::ServerContext* context,
    const algorithm_proto::AIDecisionRequest* request,
    algorithm_proto::ActionResponse* response)
{
    (void)context;

    AIDecisionRequest internal_request;
    internal_request.npc_id = request->npc_id();
    for (int i = 0; i < request->context_size(); ++i) {
        internal_request.context.push_back(request->context(i));
    }

    auto result = service_.MakeAIDecision(internal_request);

    response->set_action_id(result.action_id);
    response->set_description(result.description);
    response->set_confidence(0.8f);

    return grpc::Status::OK;
}

grpc::Status AlgorithmGrpcServiceImpl::GetSkillTree(
    grpc::ServerContext* context,
    const algorithm_proto::SkillTreeRequest* request,
    algorithm_proto::SkillTree* response)
{
    (void)context;

    response->set_character_id(request->character_id());

    // Use Profession::Warrior as default; a real implementation would
    // look up the character's profession from their ID.
    auto skills = service_.GetSkillsByProfession(Profession::Warrior);

    for (const auto* skill : skills) {
        auto* node = response->add_skills();
        node->set_skill_id(skill->skill_id);
        node->set_name(skill->name);
        node->set_description(skill->description);
        node->set_damage_multiplier(skill->damage_multiplier);
        node->set_mana_cost(skill->mana_cost);
        node->set_cooldown(skill->cooldown_ms);
        for (int prereq : skill->prerequisites) {
            node->add_prerequisites(prereq);
        }
        // Check if skill is learnable given the learned_skills list
        bool can_learn = true;
        for (int prereq : skill->prerequisites) {
            bool found = false;
            for (int i = 0; i < request->learned_skills_size(); ++i) {
                if (request->learned_skills(i) == prereq) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                can_learn = false;
                break;
            }
        }
        node->set_can_learn(can_learn);
    }

    return grpc::Status::OK;
}

grpc::Status AlgorithmGrpcServiceImpl::ValidateInput(
    grpc::ServerContext* context,
    const algorithm_proto::ValidationRequest* request,
    algorithm_proto::ValidationResult* response)
{
    (void)context;

    bool all_valid = true;
    auto* normalized = response->mutable_normalized_fields();

    for (const auto& [key, value] : request->fields()) {
        if (value.empty()) {
            response->add_errors("字段 '" + key + "' 不能为空");
            all_valid = false;
        } else {
            (*normalized)[key] = value;
        }
    }

    response->set_is_valid(all_valid);
    return grpc::Status::OK;
}

} // namespace algorithm
