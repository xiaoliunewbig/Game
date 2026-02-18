/*
 * 文件名: AlgorithmServiceClient.h
 * 说明: 算法服务gRPC客户端类，负责与Algorithm Service通信。
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <functional>
#include <memory>

#include <grpcpp/grpcpp.h>
#include "AlgorithmService.pb.h"
#include "AlgorithmService.grpc.pb.h"

/**
 * @brief 算法服务gRPC客户端类
 *
 * 使用gRPC与Algorithm Service通信，保持与旧HTTP版本相同的公共接口。
 */
class AlgorithmServiceClient : public QObject
{
    Q_OBJECT

public:
    explicit AlgorithmServiceClient(QObject *parent = nullptr);
    ~AlgorithmServiceClient();

    bool initialize(const QString &serviceUrl);

    void connectToService();
    void disconnect();
    bool isConnected() const;
    bool testConnection();

    void calculateDamage(const QJsonObject &request,
                        std::function<void(const QJsonObject&)> callback);

    void makeAIDecision(const QJsonObject &request,
                       std::function<void(const QJsonObject&)> callback);

    void getSkillTree(const QJsonObject &request,
                     std::function<void(const QJsonObject&)> callback);

    void validateData(const QJsonObject &request,
                     std::function<void(const QJsonObject&)> callback);

    void setTimeout(int timeoutMs);
    void setRetryCount(int count);

    static QJsonObject createDamageRequest(int attackerId, int defenderId,
                                          int skillId, int attackerLevel,
                                          int attackerAttack, int defenderDefense);

    static QJsonObject createAIDecisionRequest(int npcId, const QJsonArray &context);

    static QJsonObject createSkillTreeRequest(int playerId, const QString &profession);

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &error);

private:
    void handleConnectionError(const QString &error);

    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<algorithm_proto::AlgorithmService::Stub> stub_;

    bool m_connected;
    QString m_serviceUrl;
    int m_timeout;
    int m_retryCount;
};
