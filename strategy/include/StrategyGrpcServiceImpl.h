/*
 * File: StrategyGrpcServiceImpl.h
 * Description: Strategy layer gRPC service bridge.
 */
#ifndef STRATEGY_GRPCSERVICEIMPL_H
#define STRATEGY_GRPCSERVICEIMPL_H

#include "Algorithm_interact/StrategyService.h"
#include "StrategyService.grpc.pb.h"
#include "StrategyService.pb.h"

#include <grpcpp/grpcpp.h>

namespace strategy {

/**
 * @brief gRPC service implementation that delegates to StrategyService.
 */
class StrategyGrpcServiceImpl final : public strategy_proto::StrategyService::Service {
public:
    StrategyGrpcServiceImpl();
    ~StrategyGrpcServiceImpl() override = default;

    grpc::Status GetGameRules(
        grpc::ServerContext* context,
        const strategy_proto::RulesRequest* request,
        strategy_proto::GameRulesResponse* response) override;

    grpc::Status UpdateWorldState(
        grpc::ServerContext* context,
        const strategy_proto::WorldStateUpdate* request,
        strategy_proto::StatusResponse* response) override;

    grpc::Status TriggerEvent(
        grpc::ServerContext* context,
        const strategy_proto::EventTrigger* request,
        strategy_proto::EventResult* response) override;

    grpc::Status QueryGameState(
        grpc::ServerContext* context,
        const strategy_proto::QueryRequest* request,
        strategy_proto::GameStateResponse* response) override;

private:
    StrategyService service_;
};

} // namespace strategy

#endif // STRATEGY_GRPCSERVICEIMPL_H
