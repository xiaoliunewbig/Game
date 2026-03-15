/*
 * File: AlgorithmGrpcServiceImpl.h
 * Description: Algorithm layer gRPC service bridge.
 */
#ifndef ALGORITHM_GRPCSERVICEIMPL_H
#define ALGORITHM_GRPCSERVICEIMPL_H

#include "AlgorithmService.grpc.pb.h"
#include "AlgorithmService.h"

#include <grpcpp/grpcpp.h>

namespace algorithm {

/**
 * @brief gRPC service implementation that delegates to AlgorithmService.
 */
class AlgorithmGrpcServiceImpl final : public algorithm_proto::AlgorithmService::Service {
public:
    AlgorithmGrpcServiceImpl();
    ~AlgorithmGrpcServiceImpl() override = default;

    grpc::Status CalculateDamage(
        grpc::ServerContext* context,
        const algorithm_proto::CalculationRequest* request,
        algorithm_proto::DamageResult* response) override;

    grpc::Status AIActionDecision(
        grpc::ServerContext* context,
        const algorithm_proto::AIDecisionRequest* request,
        algorithm_proto::ActionResponse* response) override;

    grpc::Status GetSkillTree(
        grpc::ServerContext* context,
        const algorithm_proto::SkillTreeRequest* request,
        algorithm_proto::SkillTree* response) override;

    grpc::Status ValidateInput(
        grpc::ServerContext* context,
        const algorithm_proto::ValidationRequest* request,
        algorithm_proto::ValidationResult* response) override;

private:
    AlgorithmService service_;
};

} // namespace algorithm

#endif // ALGORITHM_GRPCSERVICEIMPL_H
