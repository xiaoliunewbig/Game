/*
 * 文件名: StrategyServiceClient.cpp
 * 说明: 策略服务gRPC客户端实现
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#include "network/StrategyServiceClient.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>

StrategyServiceClient::StrategyServiceClient(QObject *parent)
    : QObject(parent)
    , m_connected(false)
    , m_serviceUrl("localhost:50052")
    , m_timeout(10000)
    , m_retryCount(3)
{
    qDebug() << "StrategyServiceClient: gRPC客户端创建完成";
}

StrategyServiceClient::~StrategyServiceClient()
{
    disconnect();
    qDebug() << "StrategyServiceClient: gRPC客户端销毁";
}

bool StrategyServiceClient::initialize(const QString &serviceUrl)
{
    m_serviceUrl = serviceUrl;

    if (m_serviceUrl.startsWith("http://")) {
        m_serviceUrl = m_serviceUrl.mid(7);
    } else if (m_serviceUrl.startsWith("https://")) {
        m_serviceUrl = m_serviceUrl.mid(8);
    }

    channel_ = grpc::CreateChannel(m_serviceUrl.toStdString(),
                                   grpc::InsecureChannelCredentials());
    stub_ = strategy_proto::StrategyService::NewStub(channel_);

    qDebug() << "StrategyServiceClient: 初始化完成，服务地址:" << m_serviceUrl;
    return true;
}

void StrategyServiceClient::connectToService()
{
    if (m_connected) {
        qWarning() << "StrategyServiceClient: 已经连接到服务";
        return;
    }

    if (!stub_) {
        initialize(m_serviceUrl);
    }

    testConnection();
}

void StrategyServiceClient::disconnect()
{
    if (!m_connected) {
        return;
    }

    m_connected = false;
    emit disconnected();
    qDebug() << "StrategyServiceClient: 断开gRPC连接";
}

bool StrategyServiceClient::isConnected() const
{
    return m_connected;
}

bool StrategyServiceClient::testConnection()
{
    if (!stub_) {
        handleConnectionError("gRPC stub未初始化");
        return false;
    }

    // Use QueryGameState as health check
    strategy_proto::QueryRequest request;
    request.set_query_type("health_check");
    strategy_proto::GameStateResponse response;

    grpc::ClientContext context;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::milliseconds(m_timeout);
    context.set_deadline(deadline);

    grpc::Status status = stub_->QueryGameState(&context, request, &response);

    if (status.ok()) {
        if (!m_connected) {
            m_connected = true;
            emit connected();
            qDebug() << "StrategyServiceClient: gRPC连接成功";
        }
        return true;
    } else {
        handleConnectionError(QString::fromStdString(status.error_message()));
        return false;
    }
}

void StrategyServiceClient::getGameRules(const QJsonObject &request,
                                       std::function<void(const QJsonObject&)> callback)
{
    if (!stub_) {
        qWarning() << "StrategyServiceClient: stub未初始化";
        return;
    }

    strategy_proto::RulesRequest grpcRequest;
    grpcRequest.set_category(request["category"].toString().toStdString());
    grpcRequest.set_active_only(request["activeOnly"].toBool(true));

    strategy_proto::GameRulesResponse grpcResponse;
    grpc::ClientContext context;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::milliseconds(m_timeout);
    context.set_deadline(deadline);

    grpc::Status status = stub_->GetGameRules(&context, grpcRequest, &grpcResponse);

    if (status.ok()) {
        QJsonObject response;
        response["category"] = QString::fromStdString(grpcResponse.category());
        response["version"] = grpcResponse.version();
        QJsonArray rulesArray;
        for (int i = 0; i < grpcResponse.rules_size(); ++i) {
            const auto& rule = grpcResponse.rules(i);
            QJsonObject ruleObj;
            ruleObj["ruleId"] = QString::fromStdString(rule.rule_id());
            ruleObj["description"] = QString::fromStdString(rule.description());
            ruleObj["category"] = QString::fromStdString(rule.category());
            ruleObj["priority"] = rule.priority();
            ruleObj["isActive"] = rule.is_active();
            rulesArray.append(ruleObj);
        }
        response["rules"] = rulesArray;
        if (callback) callback(response);
    } else {
        qWarning() << "StrategyServiceClient: getGameRules失败:" << QString::fromStdString(status.error_message());
        if (callback) {
            QJsonObject errorResponse;
            errorResponse["error"] = true;
            errorResponse["message"] = QString::fromStdString(status.error_message());
            callback(errorResponse);
        }
    }
}

void StrategyServiceClient::updateWorldState(const QJsonObject &request,
                                           std::function<void(const QJsonObject&)> callback)
{
    if (!stub_) {
        qWarning() << "StrategyServiceClient: stub未初始化";
        return;
    }

    strategy_proto::WorldStateUpdate grpcRequest;
    QJsonDocument doc(request["worldState"].toObject());
    grpcRequest.set_world_state_json(doc.toJson(QJsonDocument::Compact).toStdString());

    strategy_proto::StatusResponse grpcResponse;
    grpc::ClientContext context;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::milliseconds(m_timeout);
    context.set_deadline(deadline);

    grpc::Status status = stub_->UpdateWorldState(&context, grpcRequest, &grpcResponse);

    if (status.ok()) {
        QJsonObject response;
        response["success"] = grpcResponse.success();
        response["message"] = QString::fromStdString(grpcResponse.message());
        if (callback) callback(response);
    } else {
        qWarning() << "StrategyServiceClient: updateWorldState失败:" << QString::fromStdString(status.error_message());
        if (callback) {
            QJsonObject errorResponse;
            errorResponse["error"] = true;
            errorResponse["message"] = QString::fromStdString(status.error_message());
            callback(errorResponse);
        }
    }
}

void StrategyServiceClient::triggerEvent(const QJsonObject &request,
                                       std::function<void(const QJsonObject&)> callback)
{
    if (!stub_) {
        qWarning() << "StrategyServiceClient: stub未初始化";
        return;
    }

    strategy_proto::EventTrigger grpcRequest;
    grpcRequest.set_event_id(request["eventId"].toInt());

    QJsonArray paramsArray = request["params"].toArray();
    for (const auto &val : paramsArray) {
        grpcRequest.add_params(val.toInt());
    }

    strategy_proto::EventResult grpcResponse;
    grpc::ClientContext context;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::milliseconds(m_timeout);
    context.set_deadline(deadline);

    grpc::Status status = stub_->TriggerEvent(&context, grpcRequest, &grpcResponse);

    if (status.ok()) {
        QJsonObject response;
        response["triggered"] = grpcResponse.triggered();
        response["resultJson"] = QString::fromStdString(grpcResponse.result_json());
        QJsonArray affectedNpcs;
        for (int i = 0; i < grpcResponse.affected_npcs_size(); ++i) {
            affectedNpcs.append(grpcResponse.affected_npcs(i));
        }
        response["affectedNpcs"] = affectedNpcs;
        if (callback) callback(response);
    } else {
        qWarning() << "StrategyServiceClient: triggerEvent失败:" << QString::fromStdString(status.error_message());
        if (callback) {
            QJsonObject errorResponse;
            errorResponse["error"] = true;
            errorResponse["message"] = QString::fromStdString(status.error_message());
            callback(errorResponse);
        }
    }
}

void StrategyServiceClient::getPlayerData(const QJsonObject &request,
                                        std::function<void(const QJsonObject&)> callback)
{
    if (!stub_) {
        qWarning() << "StrategyServiceClient: stub未初始化";
        return;
    }

    strategy_proto::QueryRequest grpcRequest;
    grpcRequest.set_query_type("player");
    grpcRequest.set_entity_id(request["playerId"].toInt());

    strategy_proto::GameStateResponse grpcResponse;
    grpc::ClientContext context;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::milliseconds(m_timeout);
    context.set_deadline(deadline);

    grpc::Status status = stub_->QueryGameState(&context, grpcRequest, &grpcResponse);

    if (status.ok()) {
        QJsonObject response;
        response["isValid"] = grpcResponse.is_valid();
        response["stateJson"] = QString::fromStdString(grpcResponse.state_json());
        if (callback) callback(response);
    } else {
        qWarning() << "StrategyServiceClient: getPlayerData失败:" << QString::fromStdString(status.error_message());
        if (callback) {
            QJsonObject errorResponse;
            errorResponse["error"] = true;
            errorResponse["message"] = QString::fromStdString(status.error_message());
            callback(errorResponse);
        }
    }
}

void StrategyServiceClient::savePlayerData(const QJsonObject &request,
                                         std::function<void(const QJsonObject&)> callback)
{
    if (!stub_) {
        qWarning() << "StrategyServiceClient: stub未初始化";
        return;
    }

    strategy_proto::WorldStateUpdate grpcRequest;
    QJsonDocument doc(request);
    grpcRequest.set_world_state_json(doc.toJson(QJsonDocument::Compact).toStdString());

    strategy_proto::StatusResponse grpcResponse;
    grpc::ClientContext context;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::milliseconds(m_timeout);
    context.set_deadline(deadline);

    grpc::Status status = stub_->UpdateWorldState(&context, grpcRequest, &grpcResponse);

    if (status.ok()) {
        QJsonObject response;
        response["success"] = grpcResponse.success();
        response["message"] = QString::fromStdString(grpcResponse.message());
        if (callback) callback(response);
    } else {
        qWarning() << "StrategyServiceClient: savePlayerData失败:" << QString::fromStdString(status.error_message());
        if (callback) {
            QJsonObject errorResponse;
            errorResponse["error"] = true;
            errorResponse["message"] = QString::fromStdString(status.error_message());
            callback(errorResponse);
        }
    }
}

void StrategyServiceClient::setTimeout(int timeoutMs)
{
    m_timeout = qMax(1000, timeoutMs);
    qDebug() << "StrategyServiceClient: 设置超时时间为" << m_timeout << "ms";
}

void StrategyServiceClient::setRetryCount(int count)
{
    m_retryCount = qMax(0, count);
    qDebug() << "StrategyServiceClient: 设置重试次数为" << m_retryCount;
}

void StrategyServiceClient::handleConnectionError(const QString &error)
{
    if (m_connected) {
        m_connected = false;
        emit disconnected();
        qWarning() << "StrategyServiceClient: 连接丢失" << error;
    }
    emit errorOccurred(error);
}

QJsonObject StrategyServiceClient::createRuleRequest(const QString &ruleId, const QJsonObject &context)
{
    QJsonObject request;
    request["ruleId"] = ruleId;
    request["context"] = context;
    return request;
}

QJsonObject StrategyServiceClient::createWorldStateRequest(const QJsonObject &stateData)
{
    QJsonObject request;
    request["worldState"] = stateData;
    return request;
}

QJsonObject StrategyServiceClient::createEventRequest(const QString &eventId, const QJsonObject &params)
{
    QJsonObject request;
    request["eventId"] = eventId;
    request["params"] = params;
    return request;
}
