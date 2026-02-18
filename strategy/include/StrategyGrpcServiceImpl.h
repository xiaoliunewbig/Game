/*
 * 文件名: StrategyGrpcServiceImpl.h
 * 说明: 策略层 gRPC 服务实现，桥接 proto 接口与内部 StrategyService
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#ifndef STRATEGY_GRPCSERVICEIMPL_H
#define STRATEGY_GRPCSERVICEIMPL_H

#include <grpcpp/grpcpp.h>
#include "StrategyService.pb.h"
#include "StrategyService.grpc.pb.h"
#include "Algorithm_interact/StrategyService.h"

namespace strategy {

/**
 * @brief gRPC 服务实现，委托给内部 StrategyService
 */
class StrategyGrpcServiceImpl final
    : public strategy_proto::StrategyService::Service {
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
