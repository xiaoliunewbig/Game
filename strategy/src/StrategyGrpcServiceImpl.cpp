/*
 * 文件名: StrategyGrpcServiceImpl.cpp
 * 说明: 策略层 gRPC 服务实现
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#include "StrategyGrpcServiceImpl.h"
#include <iostream>

namespace strategy {

StrategyGrpcServiceImpl::StrategyGrpcServiceImpl() {
    std::cout << "StrategyGrpcServiceImpl: 服务初始化" << std::endl;
}

grpc::Status StrategyGrpcServiceImpl::GetGameRules(
    grpc::ServerContext* context,
    const strategy_proto::RulesRequest* request,
    strategy_proto::GameRulesResponse* response)
{
    (void)context;

    auto rules = service_.GetGameRules(request->category());

    response->set_category(request->category());
    response->set_version(1);

    return grpc::Status::OK;
}

grpc::Status StrategyGrpcServiceImpl::UpdateWorldState(
    grpc::ServerContext* context,
    const strategy_proto::WorldStateUpdate* request,
    strategy_proto::StatusResponse* response)
{
    (void)context;

    WorldStateUpdate internal_update;
    internal_update.world_state_json = request->world_state_json();

    auto result = service_.UpdateWorldState(internal_update);

    response->set_success(result.success);
    response->set_message(result.message);

    return grpc::Status::OK;
}

grpc::Status StrategyGrpcServiceImpl::TriggerEvent(
    grpc::ServerContext* context,
    const strategy_proto::EventTrigger* request,
    strategy_proto::EventResult* response)
{
    (void)context;

    EventTriggerRequest internal_request;
    internal_request.event_id = request->event_id();
    for (int i = 0; i < request->params_size(); ++i) {
        internal_request.params.push_back(request->params(i));
    }

    auto result = service_.TriggerEvent(internal_request);

    response->set_triggered(result.triggered);
    response->set_result_json(result.result_json);
    for (int npc_id : result.affected_npcs) {
        response->add_affected_npcs(npc_id);
    }
    for (const auto& change : result.state_changes) {
        response->add_state_changes(change);
    }

    return grpc::Status::OK;
}

grpc::Status StrategyGrpcServiceImpl::QueryGameState(
    grpc::ServerContext* context,
    const strategy_proto::QueryRequest* request,
    strategy_proto::GameStateResponse* response)
{
    (void)context;

    auto state = service_.QueryGameState(request->query_type());

    response->set_is_valid(true);

    return grpc::Status::OK;
}

} // namespace strategy
