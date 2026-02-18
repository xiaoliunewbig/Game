/*
 * 文件名: AlgorithmGrpcServiceImpl.h
 * 说明: 算法层 gRPC 服务实现，桥接 proto 接口与内部 AlgorithmService
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#ifndef ALGORITHM_GRPCSERVICEIMPL_H
#define ALGORITHM_GRPCSERVICEIMPL_H

#include <grpcpp/grpcpp.h>
#include "AlgorithmService.pb.h"
#include "AlgorithmService.grpc.pb.h"
#include "AlgorithmService.h"

namespace algorithm {

/**
 * @brief gRPC 服务实现，委托给内部 AlgorithmService
 */
class AlgorithmGrpcServiceImpl final
    : public algorithm_proto::AlgorithmService::Service {
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
