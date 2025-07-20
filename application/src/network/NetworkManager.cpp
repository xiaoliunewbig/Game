/*
 * 文件名: NetworkManager.cpp
 * 说明: 网络管理器的具体实现。
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 本文件实现网络管理器的所有功能，包括服务连接管理、
 * 连接状态监控和错误处理。
 */
#include "network/NetworkManager.h"
#include "network/AlgorithmServiceClient.h"
#include "network/StrategyServiceClient.h"
#include <QTimer>
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_algorithmClient(nullptr)
    , m_strategyClient(nullptr)
    , m_connectionState(ConnectionState::Disconnected)
    , m_reconnectTimer(nullptr)
    , m_reconnectAttempts(0)
    , m_maxReconnectAttempts(5)
    , m_reconnectInterval(5000)
    , m_algorithmServiceUrl("localhost:50051")
    , m_strategyServiceUrl("localhost:50052")
{
    // 初始化重连定时器
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &NetworkManager::attemptReconnect);
    
    qDebug() << "NetworkManager: 网络管理器初始化完成";
}

NetworkManager::~NetworkManager()
{
    disconnect();
    qDebug() << "NetworkManager: 网络管理器销毁";
}

bool NetworkManager::initialize()
{
    try {
        // 创建算法服务客户端
        m_algorithmClient = std::make_unique<AlgorithmServiceClient>(this);
        if (!m_algorithmClient->initialize(m_algorithmServiceUrl)) {
            qCritical() << "NetworkManager: 算法服务客户端初始化失败";
            return false;
        }
        
        // 创建策略服务客户端
        m_strategyClient = std::make_unique<StrategyServiceClient>(this);
        if (!m_strategyClient->initialize(m_strategyServiceUrl)) {
            qCritical() << "NetworkManager: 策略服务客户端初始化失败";
            return false;
        }
        
        // 连接客户端信号
        setupClientConnections();
        
        qDebug() << "NetworkManager: 初始化成功";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "NetworkManager: 初始化异常:" << e.what();
        return false;
    }
}

bool NetworkManager::connectToServices()
{
    if (m_connectionState == ConnectionState::Connecting) {
        qWarning() << "NetworkManager: 正在连接中";
        return false;
    }
    
    qDebug() << "NetworkManager: 开始连接服务";
    
    setConnectionState(ConnectionState::Connecting);
    m_reconnectAttempts = 0;
    
    // 异步连接算法服务
    if (m_algorithmClient) {
        m_algorithmClient->connectToService();
    }
    
    // 异步连接策略服务
    if (m_strategyClient) {
        m_strategyClient->connectToService();
    }
    
    return true;
}

void NetworkManager::disconnect()
{
    if (m_connectionState == ConnectionState::Disconnected) {
        return;
    }
    
    qDebug() << "NetworkManager: 断开服务连接";
    
    // 停止重连定时器
    m_reconnectTimer->stop();
    
    // 断开客户端连接
    if (m_algorithmClient) {
        m_algorithmClient->disconnect();
    }
    
    if (m_strategyClient) {
        m_strategyClient->disconnect();
    }
    
    setConnectionState(ConnectionState::Disconnected);
}

void NetworkManager::setServiceUrls(const QString &algorithmUrl, const QString &strategyUrl)
{
    if (m_connectionState == ConnectionState::Connected) {
        qWarning() << "NetworkManager: 无法在连接状态下修改服务地址";
        return;
    }
    
    m_algorithmServiceUrl = algorithmUrl;
    m_strategyServiceUrl = strategyUrl;
    
    qDebug() << "NetworkManager: 设置服务地址 - Algorithm:" << algorithmUrl << "Strategy:" << strategyUrl;
}

void NetworkManager::setReconnectSettings(int maxAttempts, int intervalMs)
{
    m_maxReconnectAttempts = qMax(0, maxAttempts);
    m_reconnectInterval = qMax(1000, intervalMs);
    
    qDebug() << "NetworkManager: 设置重连参数 - 最大尝试次数:" << m_maxReconnectAttempts 
             << "间隔:" << m_reconnectInterval << "ms";
}

ConnectionState NetworkManager::getConnectionState() const
{
    return m_connectionState;
}

QString NetworkManager::getConnectionStateString() const
{
    switch (m_connectionState) {
        case ConnectionState::Disconnected:
            return "已断开";
        case ConnectionState::Connecting:
            return "连接中";
        case ConnectionState::Connected:
            return "已连接";
        case ConnectionState::Reconnecting:
            return "重连中";
        case ConnectionState::Error:
            return "连接错误";
        default:
            return "未知状态";
    }
}

AlgorithmServiceClient* NetworkManager::getAlgorithmClient() const
{
    return m_algorithmClient.get();
}

StrategyServiceClient* NetworkManager::getStrategyClient() const
{
    return m_strategyClient.get();
}

QJsonObject NetworkManager::getConnectionInfo() const
{
    QJsonObject info;
    
    info["state"] = getConnectionStateString();
    info["algorithmServiceUrl"] = m_algorithmServiceUrl;
    info["strategyServiceUrl"] = m_strategyServiceUrl;
    info["reconnectAttempts"] = m_reconnectAttempts;
    info["maxReconnectAttempts"] = m_maxReconnectAttempts;
    info["reconnectInterval"] = m_reconnectInterval;
    
    // 客户端连接状态
    QJsonObject clientStates;
    if (m_algorithmClient) {
        clientStates["algorithm"] = m_algorithmClient->isConnected();
    }
    if (m_strategyClient) {
        clientStates["strategy"] = m_strategyClient->isConnected();
    }
    info["clientStates"] = clientStates;
    
    return info;
}

bool NetworkManager::testConnection()
{
    if (m_connectionState != ConnectionState::Connected) {
        return false;
    }
    
    bool algorithmOk = m_algorithmClient ? m_algorithmClient->testConnection() : false;
    bool strategyOk = m_strategyClient ? m_strategyClient->testConnection() : false;
    
    return algorithmOk && strategyOk;
}

void NetworkManager::setupClientConnections()
{
    // 连接算法服务客户端信号
    if (m_algorithmClient) {
        connect(m_algorithmClient.get(), &AlgorithmServiceClient::connected,
                this, &NetworkManager::onAlgorithmServiceConnected);
        connect(m_algorithmClient.get(), &AlgorithmServiceClient::disconnected,
                this, &NetworkManager::onAlgorithmServiceDisconnected);
        connect(m_algorithmClient.get(), &AlgorithmServiceClient::errorOccurred,
                this, &NetworkManager::onAlgorithmServiceError);
    }
    
    // 连接策略服务客户端信号
    if (m_strategyClient) {
        connect(m_strategyClient.get(), &StrategyServiceClient::connected,
                this, &NetworkManager::onStrategyServiceConnected);
        connect(m_strategyClient.get(), &StrategyServiceClient::disconnected,
                this, &NetworkManager::onStrategyServiceDisconnected);
        connect(m_strategyClient.get(), &StrategyServiceClient::errorOccurred,
                this, &NetworkManager::onStrategyServiceError);
    }
}

void NetworkManager::setConnectionState(ConnectionState state)
{
    if (m_connectionState != state) {
        ConnectionState oldState = m_connectionState;
        m_connectionState = state;
        
        emit connectionStateChanged(state);
        
        qDebug() << "NetworkManager: 连接状态变更" 
                 << getConnectionStateString(oldState) << "->" << getConnectionStateString();
        
        // 根据状态变化执行相应操作
        if (state == ConnectionState::Connected) {
            m_reconnectAttempts = 0;
            emit allServicesConnected();
        } else if (state == ConnectionState::Error && oldState == ConnectionState::Connected) {
            emit connectionLost();
            startReconnect();
        }
    }
}

QString NetworkManager::getConnectionStateString(ConnectionState state) const
{
    switch (state) {
        case ConnectionState::Disconnected:
            return "已断开";
        case ConnectionState::Connecting:
            return "连接中";
        case ConnectionState::Connected:
            return "已连接";
        case ConnectionState::Reconnecting:
            return "重连中";
        case ConnectionState::Error:
            return "连接错误";
        default:
            return "未知状态";
    }
}

void NetworkManager::checkOverallConnectionState()
{
    bool algorithmConnected = m_algorithmClient ? m_algorithmClient->isConnected() : false;
    bool strategyConnected = m_strategyClient ? m_strategyClient->isConnected() : false;
    
    if (algorithmConnected && strategyConnected) {
        setConnectionState(ConnectionState::Connected);
    } else if (!algorithmConnected && !strategyConnected) {
        if (m_connectionState == ConnectionState::Connecting) {
            setConnectionState(ConnectionState::Error);
        } else if (m_connectionState == ConnectionState::Connected) {
            setConnectionState(ConnectionState::Error);
        }
    }
    // 部分连接状态保持当前状态，等待其他服务连接
}

void NetworkManager::startReconnect()
{
    if (m_maxReconnectAttempts <= 0 || m_reconnectAttempts >= m_maxReconnectAttempts) {
        qWarning() << "NetworkManager: 达到最大重连次数，停止重连";
        setConnectionState(ConnectionState::Error);
        return;
    }
    
    setConnectionState(ConnectionState::Reconnecting);
    
    qDebug() << "NetworkManager: 开始重连，尝试次数:" << (m_reconnectAttempts + 1) 
             << "/" << m_maxReconnectAttempts;
    
    m_reconnectTimer->start(m_reconnectInterval);
}

void NetworkManager::attemptReconnect()
{
    m_reconnectAttempts++;
    
    qDebug() << "NetworkManager: 执行重连尝试" << m_reconnectAttempts;
    
    // 重新连接服务
    if (m_algorithmClient && !m_algorithmClient->isConnected()) {
        m_algorithmClient->connectToService();
    }
    
    if (m_strategyClient && !m_strategyClient->isConnected()) {
        m_strategyClient->connectToService();
    }
    
    // 设置超时检查
    QTimer::singleShot(5000, this, [this]() {
        if (m_connectionState == ConnectionState::Reconnecting) {
            // 重连超时，继续下一次尝试
            startReconnect();
        }
    });
}

void NetworkManager::onAlgorithmServiceConnected()
{
    qDebug() << "NetworkManager: 算法服务连接成功";
    checkOverallConnectionState();
}

void NetworkManager::onAlgorithmServiceDisconnected()
{
    qDebug() << "NetworkManager: 算法服务连接断开";
    checkOverallConnectionState();
}

void NetworkManager::onAlgorithmServiceError(const QString &error)
{
    qWarning() << "NetworkManager: 算法服务错误:" << error;
    emit serviceError("Algorithm", error);
    checkOverallConnectionState();
}

void NetworkManager::onStrategyServiceConnected()
{
    qDebug() << "NetworkManager: 策略服务连接成功";
    checkOverallConnectionState();
}

void NetworkManager::onStrategyServiceDisconnected()
{
    qDebug() << "NetworkManager: 策略服务连接断开";
    checkOverallConnectionState();
}

void NetworkManager::onStrategyServiceError(const QString &error)
{
    qWarning() << "NetworkManager: 策略服务错误:" << error;
    emit serviceError("Strategy", error);
    checkOverallConnectionState();
}
