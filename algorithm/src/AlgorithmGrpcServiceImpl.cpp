/*
 * File: AlgorithmGrpcServiceImpl.cpp
 * Description: gRPC service bridge for algorithm module.
 */

#include "AlgorithmGrpcServiceImpl.h"
#include "CharacterStats.h"
#include "InputValidator.h"
#include "SkillTreeManager.h"

#include <algorithm>
#include <cctype>
#include <exception>
#include <iostream>
#include <string>

namespace algorithm {

AlgorithmGrpcServiceImpl::AlgorithmGrpcServiceImpl() {
    std::cout << "AlgorithmGrpcServiceImpl: service initialized" << std::endl;
}

grpc::Status AlgorithmGrpcServiceImpl::CalculateDamage(
    grpc::ServerContext* context,
    const algorithm_proto::CalculationRequest* request,
    algorithm_proto::DamageResult* response) {
    (void)context;

    try {
        DamageRequest internal_request;
        internal_request.attacker_id = request->attacker_id();
        internal_request.defender_id = request->defender_id();
        internal_request.skill_id = request->skill_id();

        const auto result = service_.CalculateDamage(internal_request);

        std::string normalized_effect = result.effect;
        std::transform(
            normalized_effect.begin(),
            normalized_effect.end(),
            normalized_effect.begin(),
            [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });

        const bool is_critical =
            normalized_effect.find("critical") != std::string::npos ||
            normalized_effect.find("crit") != std::string::npos;

        response->set_damage(result.damage);
        response->set_effect(result.effect);
        response->set_is_critical(is_critical);
        response->set_element_multiplier(1.0f);

        return grpc::Status::OK;
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AlgorithmGrpcServiceImpl::AIActionDecision(
    grpc::ServerContext* context,
    const algorithm_proto::AIDecisionRequest* request,
    algorithm_proto::ActionResponse* response) {
    (void)context;

    try {
        AIDecisionRequest internal_request;
        internal_request.npc_id = request->npc_id();
        for (int i = 0; i < request->context_size(); ++i) {
            internal_request.context.push_back(request->context(i));
        }

        const auto result = service_.MakeAIDecision(internal_request);

        response->set_action_id(result.action_id);
        response->set_description(result.description);
        response->set_confidence(0.8f);

        return grpc::Status::OK;
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AlgorithmGrpcServiceImpl::GetSkillTree(
    grpc::ServerContext* context,
    const algorithm_proto::SkillTreeRequest* request,
    algorithm_proto::SkillTree* response) {
    (void)context;

    try {
        response->set_character_id(request->character_id());

        // Default profession fallback until character profession mapping is added.
        const auto skills = service_.GetSkillsByProfession(Profession::Warrior);

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
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AlgorithmGrpcServiceImpl::ValidateInput(
    grpc::ServerContext* context,
    const algorithm_proto::ValidationRequest* request,
    algorithm_proto::ValidationResult* response) {
    (void)context;

    try {
        if (request->validation_type() == "health_check") {
            response->set_is_valid(true);
            return grpc::Status::OK;
        }

        bool all_valid = true;
        auto* normalized = response->mutable_normalized_fields();

        for (const auto& field_entry : request->fields()) {
            const auto& key = field_entry.first;
            const auto& value = field_entry.second;

            if (value.empty()) {
                response->add_errors("Field '" + key + "' must not be empty");
                all_valid = false;
            } else {
                (*normalized)[key] = value;
            }
        }

        response->set_is_valid(all_valid);
        return grpc::Status::OK;
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

} // namespace algorithm
