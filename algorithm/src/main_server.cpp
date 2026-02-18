/*
 * 文件名: main_server.cpp
 * 说明: 算法层 gRPC 服务入口
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#include "AlgorithmGrpcServiceImpl.h"
#include <grpcpp/grpcpp.h>
#include <iostream>
#include <csignal>
#include <memory>

static std::unique_ptr<grpc::Server> g_server;

static void signalHandler(int signum) {
    std::cout << "\n算法服务: 收到信号 " << signum << "，正在关闭..." << std::endl;
    if (g_server) {
        g_server->Shutdown();
    }
}

int main() {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::string server_address("0.0.0.0:50051");
    algorithm::AlgorithmGrpcServiceImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    g_server = builder.BuildAndStart();
    if (!g_server) {
        std::cerr << "算法服务: 启动失败" << std::endl;
        return 1;
    }

    std::cout << "算法服务: 监听端口 " << server_address << std::endl;
    g_server->Wait();
    std::cout << "算法服务: 已关闭" << std::endl;

    return 0;
}
