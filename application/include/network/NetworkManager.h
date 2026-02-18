/*
 * 文件名: NetworkManager.h
 * 说明: 网络管理器类，统一管理与后端服务的连接。
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 *
 * 本类负责管理与Algorithm Service和Strategy Service的连接，
 * 提供连接状态监控、自动重连和错误处理功能。
 */
#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QTimer>
#include <memory>

class AlgorithmServiceClient;
class StrategyServiceClient;

/**
 * @brief 连接状态枚举
 */
enum class ConnectionState {
    Disconnected,   ///< 已断开
    Connecting,     ///< 连接中
    Connected,      ///< 已连接
    Reconnecting,   ///< 重连中
    Error          ///< 连接错误
};

/**
 * @brief 网络管理器类
 * 
 * 该类统一管理游戏客户端与后端服务的网络连接，包括：
 * - 服务连接管理
 * - 连接状态监控
 * - 自动重连机制
 * - 错误处理和恢复
 * - 连接配置管理
 * 
 * 通过Q_PROPERTY暴露连接状态给QML，实现实时的连接状态显示。
 */
class NetworkManager : public QObject
{
    Q_OBJECT
    
    // QML可访问的属性
    Q_PROPERTY(ConnectionState connectionState READ getConnectionState NOTIFY connectionStateChanged)
    Q_PROPERTY(QString connectionStateString READ getConnectionStateString NOTIFY connectionStateChanged)
    Q_PROPERTY(QJsonObject connectionInfo READ getConnectionInfo NOTIFY connectionStateChanged)

public:
    /**
     * @brief 构造函数
     * 
     * @param parent 父对象指针
     */
    explicit NetworkManager(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~NetworkManager();

    /**
     * @brief 初始化网络管理器
     * 
     * @return bool 是否初始化成功
     */
    bool initialize();

    // 连接管理接口
    
    /**
     * @brief 连接到所有服务
     * 
     * @return bool 是否开始连接
     */
    Q_INVOKABLE bool connectToServices();
    
    /**
     * @brief 断开所有连接
     */
    Q_INVOKABLE void disconnect();
    
    /**
     * @brief 设置服务地址
     * 
     * @param algorithmUrl 算法服务地址
     * @param strategyUrl 策略服务地址
     */
    void setServiceUrls(const QString &algorithmUrl, const QString &strategyUrl);
    
    /**
     * @brief 设置重连参数
     * 
     * @param maxAttempts 最大重连次数
     * @param intervalMs 重连间隔（毫秒）
     */
    void setReconnectSettings(int maxAttempts, int intervalMs);

    // 状态查询接口
    
    /**
     * @brief 获取连接状态
     * 
     * @return ConnectionState 当前连接状态
     */
    ConnectionState getConnectionState() const;
    
    /**
     * @brief 获取连接状态字符串
     * 
     * @return QString 连接状态的中文描述
     */
    QString getConnectionStateString() const;
    
    /**
     * @brief 获取连接信息
     * 
     * @return QJsonObject 详细的连接信息
     */
    QJsonObject getConnectionInfo() const;
    
    /**
     * @brief 测试连接
     * 
     * @return bool 连接是否正常
     */
    Q_INVOKABLE bool testConnection();

    // 客户端访问接口
    
    /**
     * @brief 获取算法服务客户端
     * 
     * @return AlgorithmServiceClient* 算法服务客户端指针
     */
    AlgorithmServiceClient* getAlgorithmClient() const;
    
    /**
     * @brief 获取策略服务客户端
     * 
     * @return StrategyServiceClient* 策略服务客户端指针
     */
    StrategyServiceClient* getStrategyClient() const;

signals:
    /**
     * @brief 连接状态改变信号
     * 
     * @param state 新的连接状态
     */
    void connectionStateChanged(ConnectionState state);
    
    /**
     * @brief 所有服务连接成功信号
     */
    void allServicesConnected();
    
    /**
     * @brief 连接丢失信号
     */
    void connectionLost();
    
    /**
     * @brief 服务错误信号
     * 
     * @param serviceName 服务名称
     * @param error 错误信息
     */
    void serviceError(const QString &serviceName, const QString &error);
    
    /**
     * @brief 重连开始信号
     * 
     * @param attempt 重连尝试次数
     */
    void reconnectStarted(int attempt);
    
    /**
     * @brief 重连成功信号
     */
    void reconnectSucceeded();
    
    /**
     * @brief 重连失败信号
     * 
     * @param reason 失败原因
     */
    void reconnectFailed(const QString &reason);

private slots:
    /**
     * @brief 算法服务连接成功处理
     */
    void onAlgorithmServiceConnected();
    
    /**
     * @brief 算法服务连接断开处理
     */
    void onAlgorithmServiceDisconnected();
    
    /**
     * @brief 算法服务错误处理
     * 
     * @param error 错误信息
     */
    void onAlgorithmServiceError(const QString &error);
    
    /**
     * @brief 策略服务连接成功处理
     */
    void onStrategyServiceConnected();
    
    /**
     * @brief 策略服务连接断开处理
     */
    void onStrategyServiceDisconnected();
    
    /**
     * @brief 策略服务错误处理
     * 
     * @param error 错误信息
     */
    void onStrategyServiceError(const QString &error);
    
    /**
     * @brief 重连尝试处理
     */
    void attemptReconnect();

private:
    /**
     * @brief 设置客户端信号连接
     */
    void setupClientConnections();
    
    /**
     * @brief 设置连接状态
     * 
     * @param state 新的连接状态
     */
    void setConnectionState(ConnectionState state);
    
    /**
     * @brief 获取连接状态字符串
     * 
     * @param state 连接状态
     * @return QString 状态描述
     */
    QString getConnectionStateString(ConnectionState state) const;
    
    /**
     * @brief 检查整体连接状态
     */
    void checkOverallConnectionState();
    
    /**
     * @brief 开始重连
     */
    void startReconnect();

private:
    /**
     * @brief 算法服务客户端
     */
    std::unique_ptr<AlgorithmServiceClient> m_algorithmClient;
    
    /**
     * @brief 策略服务客户端
     */
    std::unique_ptr<StrategyServiceClient> m_strategyClient;
    
    /**
     * @brief 当前连接状态
     */
    ConnectionState m_connectionState;
    
    /**
     * @brief 重连定时器
     */
    QTimer *m_reconnectTimer;
    
    /**
     * @brief 重连尝试次数
     */
    int m_reconnectAttempts;
    
    /**
     * @brief 最大重连次数
     */
    int m_maxReconnectAttempts;
    
    /**
     * @brief 重连间隔（毫秒）
     */
    int m_reconnectInterval;
    
    /**
     * @brief 算法服务地址
     */
    QString m_algorithmServiceUrl;
    
    /**
     * @brief 策略服务地址
     */
    QString m_strategyServiceUrl;
};
