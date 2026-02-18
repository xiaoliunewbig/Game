/*
 * 文件名: StrategyServiceClient.h
 * 说明: 策略服务gRPC客户端类，负责与Strategy Service通信。
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
#include "StrategyService.pb.h"
#include "StrategyService.grpc.pb.h"

/**
 * @brief 策略服务gRPC客户端类
 *
 * 使用gRPC与Strategy Service通信，保持与旧HTTP版本相同的公共接口。
 */
class StrategyServiceClient : public QObject
{
    Q_OBJECT

public:
    explicit StrategyServiceClient(QObject *parent = nullptr);
    ~StrategyServiceClient();

    bool initialize(const QString &serviceUrl);

    void connectToService();
    void disconnect();
    bool isConnected() const;
    bool testConnection();

    void getGameRules(const QJsonObject &request,
                     std::function<void(const QJsonObject&)> callback);

    void updateWorldState(const QJsonObject &request,
                         std::function<void(const QJsonObject&)> callback);

    void triggerEvent(const QJsonObject &request,
                     std::function<void(const QJsonObject&)> callback);

    void getPlayerData(const QJsonObject &request,
                      std::function<void(const QJsonObject&)> callback);

    void savePlayerData(const QJsonObject &request,
                       std::function<void(const QJsonObject&)> callback);

    void setTimeout(int timeoutMs);
    void setRetryCount(int count);

    static QJsonObject createRuleRequest(const QString &ruleId, const QJsonObject &context);
    static QJsonObject createWorldStateRequest(const QJsonObject &stateData);
    static QJsonObject createEventRequest(const QString &eventId, const QJsonObject &params);

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &error);

private:
    void handleConnectionError(const QString &error);

    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<strategy_proto::StrategyService::Stub> stub_;

    bool m_connected;
    QString m_serviceUrl;
    int m_timeout;
    int m_retryCount;
};
