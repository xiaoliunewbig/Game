/*
 * 文件名: StrategyServiceClient.cpp
 * 说明: 策略服务客户端的具体实现。
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 本文件实现策略服务客户端的所有功能，包括连接管理、
 * 请求发送和响应处理。
 */
#include "network/StrategyServiceClient.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QTimer>
#include <QDebug>

StrategyServiceClient::StrategyServiceClient(QObject *parent)
    : QObject(parent)
    , m_networkManager(nullptr)
    , m_connected(false)
    , m_serviceUrl("http://localhost:50052")
    , m_timeout(10000)
    , m_retryCount(3)
{
    m_networkManager = new QNetworkAccessManager(this);
    qDebug() << "StrategyServiceClient: 策略服务客户端创建完成";
}

StrategyServiceClient::~StrategyServiceClient()
{
    disconnect();
    qDebug() << "StrategyServiceClient: 策略服务客户端销毁";
}

bool StrategyServiceClient::initialize(const QString &serviceUrl)
{
    m_serviceUrl = serviceUrl;
    
    if (!m_serviceUrl.startsWith("http://") && !m_serviceUrl.startsWith("https://")) {
        m_serviceUrl = "http://" + m_serviceUrl;
    }
    
    qDebug() << "StrategyServiceClient: 初始化完成，服务地址:" << m_serviceUrl;
    return true;
}

void StrategyServiceClient::connectToService()
{
    if (m_connected) {
        qWarning() << "StrategyServiceClient: 已经连接到服务";
        return;
    }
    
    qDebug() << "StrategyServiceClient: 开始连接策略服务";
    testConnection();
}

void StrategyServiceClient::disconnect()
{
    if (!m_connected) {
        return;
    }
    
    m_connected = false;
    emit disconnected();
    qDebug() << "StrategyServiceClient: 断开策略服务连接";
}

bool StrategyServiceClient::isConnected() const
{
    return m_connected;
}

bool StrategyServiceClient::testConnection()
{
    QJsonObject request;
    request["type"] = "health_check";
    request["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    sendRequest("/health", request, [this](const QJsonObject &response) {
        if (response["status"].toString() == "ok") {
            if (!m_connected) {
                m_connected = true;
                emit connected();
                qDebug() << "StrategyServiceClient: 连接成功";
            }
        } else {
            handleConnectionError("健康检查失败");
        }
    });
    
    return true;
}

void StrategyServiceClient::getGameRules(const QJsonObject &request,
                                       std::function<void(const QJsonObject&)> callback)
{
    if (!m_connected) {
        qWarning() << "StrategyServiceClient: 服务未连接，无法获取游戏规则";
        return;
    }
    
    QJsonObject ruleRequest = request;
    ruleRequest["type"] = "get_game_rules";
    ruleRequest["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    sendRequest("/game_rules", ruleRequest, callback);
}

void StrategyServiceClient::updateWorldState(const QJsonObject &request,
                                           std::function<void(const QJsonObject&)> callback)
{
    if (!m_connected) {
        qWarning() << "StrategyServiceClient: 服务未连接，无法更新世界状态";
        return;
    }
    
    QJsonObject stateRequest = request;
    stateRequest["type"] = "update_world_state";
    stateRequest["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    sendRequest("/world_state", stateRequest, callback);
}

void StrategyServiceClient::triggerEvent(const QJsonObject &request,
                                       std::function<void(const QJsonObject&)> callback)
{
    if (!m_connected) {
        qWarning() << "StrategyServiceClient: 服务未连接，无法触发事件";
        return;
    }
    
    QJsonObject eventRequest = request;
    eventRequest["type"] = "trigger_event";
    eventRequest["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    sendRequest("/trigger_event", eventRequest, callback);
}

void StrategyServiceClient::getPlayerData(const QJsonObject &request,
                                        std::function<void(const QJsonObject&)> callback)
{
    if (!m_connected) {
        qWarning() << "StrategyServiceClient: 服务未连接，无法获取玩家数据";
        return;
    }
    
    QJsonObject playerRequest = request;
    playerRequest["type"] = "get_player_data";
    playerRequest["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    sendRequest("/player_data", playerRequest, callback);
}

void StrategyServiceClient::savePlayerData(const QJsonObject &request,
                                         std::function<void(const QJsonObject&)> callback)
{
    if (!m_connected) {
        qWarning() << "StrategyServiceClient: 服务未连接，无法保存玩家数据";
        return;
    }
    
    QJsonObject saveRequest = request;
    saveRequest["type"] = "save_player_data";
    saveRequest["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    sendRequest("/save_player", saveRequest, callback);
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

void StrategyServiceClient::sendRequest(const QString &endpoint, 
                                      const QJsonObject &request,
                                      std::function<void(const QJsonObject&)> callback,
                                      int retryAttempt)
{
    QString url = m_serviceUrl + endpoint;
    
    QNetworkRequest netRequest;
    netRequest.setUrl(QUrl(url));
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    netRequest.setRawHeader("User-Agent", "GameClient/1.0");
    
    QJsonDocument doc(request);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    
    QNetworkReply *reply = m_networkManager->post(netRequest, data);
    
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
            QByteArray responseData = reply->readAll();
            QJsonParseError parseError;
            QJsonDocument responseDoc = QJsonDocument::fromJson(responseData, &parseError);
            
            if (parseError.error == QJsonParseError::NoError) {
                QJsonObject responseObj = responseDoc.object();
                
                if (callback) {
                    callback(responseObj);
                }
                
                qDebug() << "StrategyServiceClient: 请求成功" << endpoint;
            } else {
                qWarning() << "StrategyServiceClient: 响应解析失败" << parseError.errorString();
                handleRequestError(endpoint, request, callback, retryAttempt, "响应解析失败");
            }
        } else {
            QString errorString = reply->errorString();
            qWarning() << "StrategyServiceClient: 请求失败" << endpoint << errorString;
            
            if (reply->error() == QNetworkReply::TimeoutError) {
                handleRequestError(endpoint, request, callback, retryAttempt, "请求超时");
            } else {
                handleRequestError(endpoint, request, callback, retryAttempt, errorString);
            }
        }
        
        reply->deleteLater();
    });
    
    qDebug() << "StrategyServiceClient: 发送请求" << endpoint << "尝试次数:" << (retryAttempt + 1);
}

void StrategyServiceClient::handleRequestError(const QString &endpoint,
                                             const QJsonObject &request,
                                             std::function<void(const QJsonObject&)> callback,
                                             int retryAttempt,
                                             const QString &error)
{
    if (retryAttempt < m_retryCount) {
        int delay = (retryAttempt + 1) * 1000;
        
        qDebug() << "StrategyServiceClient: 重试请求" << endpoint 
                 << "延迟" << delay << "ms";
        
        QTimer::singleShot(delay, [this, endpoint, request, callback, retryAttempt]() {
            sendRequest(endpoint, request, callback, retryAttempt + 1);
        });
    } else {
        qCritical() << "StrategyServiceClient: 请求最终失败" << endpoint << error;
        
        emit errorOccurred(QString("请求失败: %1 - %2").arg(endpoint, error));
        
        if (error.contains("连接") || error.contains("超时") || error.contains("网络")) {
            handleConnectionError(error);
        }
        
        if (callback) {
            QJsonObject errorResponse;
            errorResponse["error"] = true;
            errorResponse["message"] = error;
            errorResponse["endpoint"] = endpoint;
            callback(errorResponse);
        }
    }
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