/*
 * 文件名: AlgorithmServiceClient.cpp
 * 说明: 算法服务gRPC客户端实现
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#include "network/AlgorithmServiceClient.h"
#include <QDebug>
#include <QJsonArray>

AlgorithmServiceClient::AlgorithmServiceClient(QObject *parent)
    : QObject(parent)
    , m_connected(false)
    , m_serviceUrl("localhost:50051")
    , m_timeout(10000)
    , m_retryCount(3)
{
    qDebug() << "AlgorithmServiceClient: gRPC客户端创建完成";
}

AlgorithmServiceClient::~AlgorithmServiceClient()
{
    disconnect();
    qDebug() << "AlgorithmServiceClient: gRPC客户端销毁";
}

bool AlgorithmServiceClient::initialize(const QString &serviceUrl)
{
    m_serviceUrl = serviceUrl;

    // Strip http:// prefix if present — gRPC uses raw host:port
    if (m_serviceUrl.startsWith("http://")) {
        m_serviceUrl = m_serviceUrl.mid(7);
    } else if (m_serviceUrl.startsWith("https://")) {
        m_serviceUrl = m_serviceUrl.mid(8);
    }

    channel_ = grpc::CreateChannel(m_serviceUrl.toStdString(),
                                   grpc::InsecureChannelCredentials());
    stub_ = algorithm_proto::AlgorithmService::NewStub(channel_);

    qDebug() << "AlgorithmServiceClient: 初始化完成，服务地址:" << m_serviceUrl;
    return true;
}

void AlgorithmServiceClient::connectToService()
{
    if (m_connected) {
        qWarning() << "AlgorithmServiceClient: 已经连接到服务";
        return;
    }

    if (!stub_) {
        initialize(m_serviceUrl);
    }

    testConnection();
}

void AlgorithmServiceClient::disconnect()
{
    if (!m_connected) {
        return;
    }

    m_connected = false;
    emit disconnected();
    qDebug() << "AlgorithmServiceClient: 断开gRPC连接";
}

bool AlgorithmServiceClient::isConnected() const
{
    return m_connected;
}

bool AlgorithmServiceClient::testConnection()
{
    if (!stub_) {
        handleConnectionError("gRPC stub未初始化");
        return false;
    }

    // Use a lightweight RPC call as health check
    algorithm_proto::ValidationRequest request;
    request.set_validation_type("health_check");
    algorithm_proto::ValidationResult response;

    grpc::ClientContext context;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::milliseconds(m_timeout);
    context.set_deadline(deadline);

    grpc::Status status = stub_->ValidateInput(&context, request, &response);

    if (status.ok()) {
        if (!m_connected) {
            m_connected = true;
            emit connected();
            qDebug() << "AlgorithmServiceClient: gRPC连接成功";
        }
        return true;
    } else {
        handleConnectionError(QString::fromStdString(status.error_message()));
        return false;
    }
}

void AlgorithmServiceClient::calculateDamage(const QJsonObject &request,
                                           std::function<void(const QJsonObject&)> callback)
{
    if (!stub_) {
        qWarning() << "AlgorithmServiceClient: stub未初始化";
        return;
    }

    algorithm_proto::CalculationRequest grpcRequest;
    grpcRequest.set_attacker_id(request["attackerId"].toInt());
    grpcRequest.set_defender_id(request["defenderId"].toInt());
    grpcRequest.set_skill_id(request["skillId"].toInt());

    algorithm_proto::DamageResult grpcResponse;
    grpc::ClientContext context;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::milliseconds(m_timeout);
    context.set_deadline(deadline);

    grpc::Status status = stub_->CalculateDamage(&context, grpcRequest, &grpcResponse);

    if (status.ok()) {
        QJsonObject response;
        response["damage"] = grpcResponse.damage();
        response["effect"] = QString::fromStdString(grpcResponse.effect());
        response["isCritical"] = grpcResponse.is_critical();
        response["elementMultiplier"] = static_cast<double>(grpcResponse.element_multiplier());
        if (callback) callback(response);
    } else {
        qWarning() << "AlgorithmServiceClient: calculateDamage失败:" << QString::fromStdString(status.error_message());
        if (callback) {
            QJsonObject errorResponse;
            errorResponse["error"] = true;
            errorResponse["message"] = QString::fromStdString(status.error_message());
            callback(errorResponse);
        }
    }
}

void AlgorithmServiceClient::makeAIDecision(const QJsonObject &request,
                                          std::function<void(const QJsonObject&)> callback)
{
    if (!stub_) {
        qWarning() << "AlgorithmServiceClient: stub未初始化";
        return;
    }

    algorithm_proto::AIDecisionRequest grpcRequest;
    grpcRequest.set_npc_id(request["npcId"].toInt());

    QJsonArray contextArray = request["context"].toArray();
    for (const auto &val : contextArray) {
        grpcRequest.add_context(val.toInt());
    }

    algorithm_proto::ActionResponse grpcResponse;
    grpc::ClientContext context;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::milliseconds(m_timeout);
    context.set_deadline(deadline);

    grpc::Status status = stub_->AIActionDecision(&context, grpcRequest, &grpcResponse);

    if (status.ok()) {
        QJsonObject response;
        response["actionId"] = grpcResponse.action_id();
        response["description"] = QString::fromStdString(grpcResponse.description());
        response["confidence"] = static_cast<double>(grpcResponse.confidence());
        if (callback) callback(response);
    } else {
        qWarning() << "AlgorithmServiceClient: makeAIDecision失败:" << QString::fromStdString(status.error_message());
        if (callback) {
            QJsonObject errorResponse;
            errorResponse["error"] = true;
            errorResponse["message"] = QString::fromStdString(status.error_message());
            callback(errorResponse);
        }
    }
}

void AlgorithmServiceClient::getSkillTree(const QJsonObject &request,
                                        std::function<void(const QJsonObject&)> callback)
{
    if (!stub_) {
        qWarning() << "AlgorithmServiceClient: stub未初始化";
        return;
    }

    algorithm_proto::SkillTreeRequest grpcRequest;
    grpcRequest.set_character_id(request["playerId"].toInt());

    algorithm_proto::SkillTree grpcResponse;
    grpc::ClientContext context;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::milliseconds(m_timeout);
    context.set_deadline(deadline);

    grpc::Status status = stub_->GetSkillTree(&context, grpcRequest, &grpcResponse);

    if (status.ok()) {
        QJsonObject response;
        response["characterId"] = grpcResponse.character_id();
        QJsonArray skillsArray;
        for (int i = 0; i < grpcResponse.skills_size(); ++i) {
            const auto& skill = grpcResponse.skills(i);
            QJsonObject skillObj;
            skillObj["skillId"] = skill.skill_id();
            skillObj["name"] = QString::fromStdString(skill.name());
            skillObj["description"] = QString::fromStdString(skill.description());
            skillObj["damageMultiplier"] = static_cast<double>(skill.damage_multiplier());
            skillObj["manaCost"] = skill.mana_cost();
            skillObj["cooldown"] = skill.cooldown();
            skillObj["canLearn"] = skill.can_learn();
            skillsArray.append(skillObj);
        }
        response["skills"] = skillsArray;
        if (callback) callback(response);
    } else {
        qWarning() << "AlgorithmServiceClient: getSkillTree失败:" << QString::fromStdString(status.error_message());
        if (callback) {
            QJsonObject errorResponse;
            errorResponse["error"] = true;
            errorResponse["message"] = QString::fromStdString(status.error_message());
            callback(errorResponse);
        }
    }
}

void AlgorithmServiceClient::validateData(const QJsonObject &request,
                                        std::function<void(const QJsonObject&)> callback)
{
    if (!stub_) {
        qWarning() << "AlgorithmServiceClient: stub未初始化";
        return;
    }

    algorithm_proto::ValidationRequest grpcRequest;
    grpcRequest.set_validation_type(request["validationType"].toString().toStdString());

    QJsonObject fields = request["fields"].toObject();
    auto* protoFields = grpcRequest.mutable_fields();
    for (auto it = fields.begin(); it != fields.end(); ++it) {
        (*protoFields)[it.key().toStdString()] = it.value().toString().toStdString();
    }

    algorithm_proto::ValidationResult grpcResponse;
    grpc::ClientContext context;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::milliseconds(m_timeout);
    context.set_deadline(deadline);

    grpc::Status status = stub_->ValidateInput(&context, grpcRequest, &grpcResponse);

    if (status.ok()) {
        QJsonObject response;
        response["isValid"] = grpcResponse.is_valid();
        QJsonArray errors;
        for (int i = 0; i < grpcResponse.errors_size(); ++i) {
            errors.append(QString::fromStdString(grpcResponse.errors(i)));
        }
        response["errors"] = errors;
        if (callback) callback(response);
    } else {
        qWarning() << "AlgorithmServiceClient: validateData失败:" << QString::fromStdString(status.error_message());
        if (callback) {
            QJsonObject errorResponse;
            errorResponse["error"] = true;
            errorResponse["message"] = QString::fromStdString(status.error_message());
            callback(errorResponse);
        }
    }
}

void AlgorithmServiceClient::setTimeout(int timeoutMs)
{
    m_timeout = qMax(1000, timeoutMs);
    qDebug() << "AlgorithmServiceClient: 设置超时时间为" << m_timeout << "ms";
}

void AlgorithmServiceClient::setRetryCount(int count)
{
    m_retryCount = qMax(0, count);
    qDebug() << "AlgorithmServiceClient: 设置重试次数为" << m_retryCount;
}

void AlgorithmServiceClient::handleConnectionError(const QString &error)
{
    if (m_connected) {
        m_connected = false;
        emit disconnected();
        qWarning() << "AlgorithmServiceClient: 连接丢失" << error;
    }
    emit errorOccurred(error);
}

QJsonObject AlgorithmServiceClient::createDamageRequest(int attackerId, int defenderId,
                                                       int skillId, int attackerLevel,
                                                       int attackerAttack, int defenderDefense)
{
    QJsonObject request;
    request["attackerId"] = attackerId;
    request["defenderId"] = defenderId;
    request["skillId"] = skillId;
    request["attackerLevel"] = attackerLevel;
    request["attackerAttack"] = attackerAttack;
    request["defenderDefense"] = defenderDefense;
    return request;
}

QJsonObject AlgorithmServiceClient::createAIDecisionRequest(int npcId, const QJsonArray &context)
{
    QJsonObject request;
    request["npcId"] = npcId;
    request["context"] = context;
    return request;
}

QJsonObject AlgorithmServiceClient::createSkillTreeRequest(int playerId, const QString &profession)
{
    QJsonObject request;
    request["playerId"] = playerId;
    request["profession"] = profession;
    return request;
}
