/*
 * File: StrategyGrpcServiceImpl.cpp
 * Description: Strategy layer gRPC service implementation.
 */

#include "StrategyGrpcServiceImpl.h"

#include <exception>
#include <iostream>

namespace strategy {

StrategyGrpcServiceImpl::StrategyGrpcServiceImpl() {
    std::cout << "StrategyGrpcServiceImpl: service initialized" << std::endl;
}

grpc::Status StrategyGrpcServiceImpl::GetGameRules(
    grpc::ServerContext* context,
    const strategy_proto::RulesRequest* request,
    strategy_proto::GameRulesResponse* response) {
    (void)context;

    try {
        const auto rules = service_.GetGameRules(request->category());
        response->set_category(rules.category);
        response->set_version(rules.version);

        for (const auto& rule : rules.rules) {
            if (request->active_only() && !rule.is_active) {
                continue;
            }

            auto* out_rule = response->add_rules();
            out_rule->set_rule_id(rule.rule_id);
            out_rule->set_description(rule.description);
            out_rule->set_category(rule.category);
            out_rule->set_priority(rule.priority);
            out_rule->set_is_active(rule.is_active);

            auto* out_conditions = out_rule->mutable_conditions();
            for (const auto& condition : rule.conditions) {
                (*out_conditions)[condition.first] = condition.second;
            }

            auto* out_effects = out_rule->mutable_effects();
            for (const auto& effect : rule.effects) {
                (*out_effects)[effect.first] = effect.second;
            }
        }

        return grpc::Status::OK;
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status StrategyGrpcServiceImpl::UpdateWorldState(
    grpc::ServerContext* context,
    const strategy_proto::WorldStateUpdate* request,
    strategy_proto::StatusResponse* response) {
    (void)context;

    try {
        WorldStateUpdate internal_update;
        internal_update.world_state_json = request->world_state_json();

        const auto result = service_.UpdateWorldState(internal_update);

        response->set_success(result.success);
        response->set_message(result.message);

        return grpc::Status::OK;
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status StrategyGrpcServiceImpl::TriggerEvent(
    grpc::ServerContext* context,
    const strategy_proto::EventTrigger* request,
    strategy_proto::EventResult* response) {
    (void)context;

    try {
        EventTriggerRequest internal_request;
        internal_request.event_id = request->event_id();
        for (int i = 0; i < request->params_size(); ++i) {
            internal_request.params.push_back(request->params(i));
        }

        const auto result = service_.TriggerEvent(internal_request);

        response->set_triggered(result.triggered);
        response->set_result_json(result.result_json);
        for (int npc_id : result.affected_npcs) {
            response->add_affected_npcs(npc_id);
        }
        for (const auto& change : result.state_changes) {
            response->add_state_changes(change);
        }

        return grpc::Status::OK;
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status StrategyGrpcServiceImpl::QueryGameState(
    grpc::ServerContext* context,
    const strategy_proto::QueryRequest* request,
    strategy_proto::GameStateResponse* response) {
    (void)context;

    try {
        if (request->query_type() == "health_check") {
            response->set_is_valid(true);
            response->set_state_json("{\"status\":\"ok\"}");
            return grpc::Status::OK;
        }

        const auto state = service_.QueryGameState(request->query_type());
        response->set_state_json(state.state_json);
        response->set_is_valid(state.is_valid);

        auto* world_state = response->mutable_world_state();
        world_state->set_last_update_time(state.world_state.last_update_time);

        auto* global_vars = world_state->mutable_global_variables();
        for (const auto& entry : state.world_state.global_variables) {
            (*global_vars)[entry.first] = entry.second;
        }

        auto* world_flags = world_state->mutable_world_flags();
        for (const auto& entry : state.world_state.world_flags) {
            (*world_flags)[entry.first] = entry.second;
        }

        for (const auto& event_name : state.world_state.active_events) {
            world_state->add_active_events(event_name);
        }

        for (const auto& relation_entry : state.world_state.npc_relations) {
            const auto& relation = relation_entry.second;
            auto* out_relation = world_state->add_npc_relations();
            out_relation->set_npc_id(relation.npc_id);
            out_relation->set_target_id(relation.target_id);
            out_relation->set_relation_type(relation.relation_type);
            out_relation->set_trust_level(relation.trust_level);
            for (const auto& history : relation.shared_history) {
                out_relation->add_shared_history(history);
            }
        }

        return grpc::Status::OK;
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

} // namespace strategy
