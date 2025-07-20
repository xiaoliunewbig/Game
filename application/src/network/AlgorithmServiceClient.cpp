/*
 * 文件名: AlgorithmServiceClient.cpp
 * 说明: 算法服务客户端的具体实现。
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 本文件实现算法服务客户端的所有功能，包括连接管理、
 * 请求发送和响应处理。
 */
#include "network/AlgorithmServiceClient.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QDebug>

AlgorithmServiceClient::AlgorithmServiceClient(QObject *parent)
    : QObject(parent)
    , m_networkManager(nullptr)
    , m_connected(false)
    , m_serviceUrl("http://localhost:50051")
    , m_timeout(10000)
    , m_retryCount(3)
{
    // 初始化网络管理器
    m_networkManager = new QNetworkAccessManager(this);
    
    qDebug() << "AlgorithmServiceClient: 算法服务客户端创建完成";
}

AlgorithmServiceClient::~AlgorithmServiceClient()
{
    disconnect();
    qDebug() << "AlgorithmServiceClient: 算法服务客户端销毁";
}

bool AlgorithmServiceClient::initialize(const QString &serviceUrl)
{
    m_serviceUrl = serviceUrl;
    
    // 确保URL格式正确
    if (!m_serviceUrl.startsWith("http://") && !m_serviceUrl.startsWith("https://")) {
        m_serviceUrl = "http://" + m_serviceUrl;
    }
    
    qDebug() << "AlgorithmServiceClient: 初始化完成，服务地址:" << m_serviceUrl;
    return true;
}

void AlgorithmServiceClient::connectToService()
{
    if (m_connected) {
        qWarning() << "AlgorithmServiceClient: 已经连接到服务";
        return;
    }
    
    qDebug() << "AlgorithmServiceClient: 开始连接算法服务";
    
    // 发送健康检查请求
    testConnection();
}

void AlgorithmServiceClient::disconnect()
{
    if (!m_connected) {
        return;
    }
    
    m_connected = false;
    emit disconnected();
    
    qDebug() << "AlgorithmServiceClient: 断开算法服务连接";
}

bool AlgorithmServiceClient::isConnected() const
{
    return m_connected;
}

bool AlgorithmServiceClient::testConnection()
{
    QJsonObject request;
    request["type"] = "health_check";
    request["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    sendRequest("/health", request, [this](const QJsonObject &response) {
        if (response["status"].toString() == "ok") {
            if (!m_connected) {
                m_connected = true;
                emit connected();
                qDebug() << "AlgorithmServiceClient: 连接成功";
            }
        } else {
            handleConnectionError("健康检查失败");
        }
    });
    
    return true;
}

void AlgorithmServiceClient::calculateDamage(const QJsonObject &request, 
                                           std::function<void(const QJsonObject&)> callback)
{
    if (!m_connected) {
        qWarning() << "AlgorithmServiceClient: 服务未连接，无法计算伤害";
        return;
    }
    
    QJsonObject damageRequest = request;
    damageRequest["type"] = "calculate_damage";
    damageRequest["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    sendRequest("/calculate_damage", damageRequest, callback);
}

void AlgorithmServiceClient::makeAIDecision(const QJsonObject &request,
                                          std::function<void(const QJsonObject&)> callback)
{
    if (!m_connected) {
        qWarning() << "AlgorithmServiceClient: 服务未连接，无法进行AI决策";
        return;
    }
    
    QJsonObject aiRequest = request;
    aiRequest["type"] = "ai_decision";
    aiRequest["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    sendRequest("/ai_decision", aiRequest, callback);
}

void AlgorithmServiceClient::getSkillTree(const QJsonObject &request,
                                        std::function<void(const QJsonObject&)> callback)
{
    if (!m_connected) {
        qWarning() << "AlgorithmServiceClient: 服务未连接，无法获取技能树";
        return;
    }
    
    QJsonObject skillRequest = request;
    skillRequest["type"] = "get_skill_tree";
    skillRequest["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    sendRequest("/skill_tree", skillRequest, callback);
}

void AlgorithmServiceClient::validateData(const QJsonObject &request,
                                        std::function<void(const QJsonObject&)> callback)
{
    if (!m_connected) {
        qWarning() << "AlgorithmServiceClient: 服务未连接，无法验证数据";
        return;
    }
    
    QJsonObject validateRequest = request;
    validateRequest["type"] = "validate_data";
    validateRequest["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    sendRequest("/validate", validateRequest, callback);
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

void AlgorithmServiceClient::sendRequest(const QString &endpoint, 
                                       const QJsonObject &request,
                                       std::function<void(const QJsonObject&)> callback,
                                       int retryAttempt)
{
    QString url = m_serviceUrl + endpoint;
    
    QNetworkRequest netRequest;
    netRequest.setUrl(QUrl(url));
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    netRequest.setRawHeader("User-Agent", "GameClient/1.0");
    
    // 设置超时
    netRequest.setAttribute(QNetworkRequest::RedirectPolicyAttribute, 
                           QNetworkRequest::NoLessSafeRedirectPolicy);
    
    QJsonDocument doc(request);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    
    QNetworkReply *reply = m_networkManager->post(netRequest, data);
    
    // 设置超时定时器
    QTimer *timeoutTimer = new QTimer(this);
    timeoutTimer->setSingleShot(true);
    timeoutTimer->start(m_timeout);
    
    connect(timeoutTimer, &QTimer::timeout, [reply, timeoutTimer]() {
        reply->abort();
        timeoutTimer->deleteLater();
    });
    
    connect(reply, &QNetworkReply::finished, [this, reply, callback, endpoint, request, retryAttempt, timeoutTimer]() {
        timeoutTimer->stop();
        timeoutTimer->deleteLater();
        
        if (reply->error() == QNetworkReply::NoError) {
            // 请求成功
            QByteArray responseData = reply->readAll();
            QJsonParseError parseError;
            QJsonDocument responseDoc = QJsonDocument::fromJson(responseData, &parseError);
            
            if (parseError.error == QJsonParseError::NoError) {
                QJsonObject responseObj = responseDoc.object();
                
                if (callback) {
                    callback(responseObj);
                }
                
                qDebug() << "AlgorithmServiceClient: 请求成功" << endpoint;
            } else {
                qWarning() << "AlgorithmServiceClient: 响应解析失败" << parseError.errorString();
                handleRequestError(endpoint, request, callback, retryAttempt, "响应解析失败");
            }
        } else {
            // 请求失败
            QString errorString = reply->errorString();
            qWarning() << "AlgorithmServiceClient: 请求失败" << endpoint << errorString;
            
            if (reply->error() == QNetworkReply::TimeoutError) {
                handleRequestError(endpoint, request, callback, retryAttempt, "请求超时");
            } else {
                handleRequestError(endpoint, request, callback, retryAttempt, errorString);
            }
        }
        
        reply->deleteLater();
    });
    
    qDebug() << "AlgorithmServiceClient: 发送请求" << endpoint << "尝试次数:" << (retryAttempt + 1);
}

void AlgorithmServiceClient::handleRequestError(const QString &endpoint,
                                               const QJsonObject &request,
                                               std::function<void(const QJsonObject&)> callback,
                                               int retryAttempt,
                                               const QString &error)
{
    if (retryAttempt < m_retryCount) {
        // 重试请求
        int delay = (retryAttempt + 1) * 1000; // 递增延迟
        
        qDebug() << "AlgorithmServiceClient: 重试请求" << endpoint 
                 << "延迟" << delay << "ms";
        
        QTimer::singleShot(delay, [this, endpoint, request, callback, retryAttempt]() {
            sendRequest(endpoint, request, callback, retryAttempt + 1);
        });
    } else {
        // 重试次数用完，报告错误
        qCritical() << "AlgorithmServiceClient: 请求最终失败" << endpoint << error;
        
        emit errorOccurred(QString("请求失败: %1 - %2").arg(endpoint, error));
        
        // 如果是连接相关错误，标记为断开连接
        if (error.contains("连接") || error.contains("超时") || error.contains("网络")) {
            handleConnectionError(error);
        }
        
        // 返回错误响应
        if (callback) {
            QJsonObject errorResponse;
            errorResponse["error"] = true;
            errorResponse["message"] = error;
            errorResponse["endpoint"] = endpoint;
            callback(errorResponse);
        }
    }
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