/*
 * 文件名: StrategyServiceClient.h
 * 说明: 策略服务客户端类，负责与Strategy Service通信。
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 本类封装与策略服务的所有通信逻辑，包括游戏规则查询、
 * 世界状态更新、事件触发等功能。
 */
#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <functional>

class QNetworkAccessManager;

/**
 * @brief 策略服务客户端类
 * 
 * 该类负责与Strategy Service进行通信，提供以下功能：
 * - 游戏规则查询
 * - 世界状态更新
 * - 事件触发和处理
 * - 玩家数据管理
 * - 连接管理和错误处理
 */
class StrategyServiceClient : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * 
     * @param parent 父对象指针
     */
    explicit StrategyServiceClient(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~StrategyServiceClient();

    /**
     * @brief 初始化客户端
     * 
     * @param serviceUrl 服务地址
     * @return bool 是否初始化成功
     */
    bool initialize(const QString &serviceUrl);

    // 连接管理接口
    
    /**
     * @brief 连接到服务
     */
    void connectToService();
    
    /**
     * @brief 断开连接
     */
    void disconnect();
    
    /**
     * @brief 检查是否已连接
     * 
     * @return bool 是否已连接
     */
    bool isConnected() const;
    
    /**
     * @brief 测试连接
     * 
     * @return bool 是否连接正常
     */
    bool testConnection();

    // 策略服务接口
    
    /**
     * @brief 获取游戏规则
     * 
     * @param request 规则查询请求
     * @param callback 结果回调函数
     */
    void getGameRules(const QJsonObject &request,
                     std::function<void(const QJsonObject&)> callback);
    
    /**
     * @brief 更新世界状态
     * 
     * @param request 世界状态更新请求
     * @param callback 结果回调函数
     */
    void updateWorldState(const QJsonObject &request,
                         std::function<void(const QJsonObject&)> callback);
    
    /**
     * @brief 触发事件
     * 
     * @param request 事件触发请求
     * @param callback 结果回调函数
     */
    void triggerEvent(const QJsonObject &request,
                     std::function<void(const QJsonObject&)> callback);
    
    /**
     * @brief 获取玩家数据
     * 
     * @param request 玩家数据请求
     * @param callback 结果回调函数
     */
    void getPlayerData(const QJsonObject &request,
                      std::function<void(const QJsonObject&)> callback);
    
    /**
     * @brief 保存玩家数据
     * 
     * @param request 玩家数据保存请求
     * @param callback 结果回调函数
     */
    void savePlayerData(const QJsonObject &request,
                       std::function<void(const QJsonObject&)> callback);

    // 配置接口
    
    /**
     * @brief 设置请求超时时间
     * 
     * @param timeoutMs 超时时间（毫秒）
     */
    void setTimeout(int timeoutMs);
    
    /**
     * @brief 设置重试次数
     * 
     * @param count 重试次数
     */
    void setRetryCount(int count);

    // 便捷方法
    
    /**
     * @brief 创建规则查询请求
     * 
     * @param ruleId 规则ID
     * @param context 上下文数据
     * @return QJsonObject 请求对象
     */
    static QJsonObject createRuleRequest(const QString &ruleId, const QJsonObject &context);
    
    /**
     * @brief 创建世界状态更新请求
     * 
     * @param stateData 状态数据
     * @return QJsonObject 请求对象
     */
    static QJsonObject createWorldStateRequest(const QJsonObject &stateData);
    
    /**
     * @brief 创建事件触发请求
     * 
     * @param eventId 事件ID
     * @param params 事件参数
     * @return QJsonObject 请求对象
     */
    static QJsonObject createEventRequest(const QString &eventId, const QJsonObject &params);

signals:
    /**
     * @brief 连接成功信号
     */
    void connected();
    
    /**
     * @brief 连接断开信号
     */
    void disconnected();
    
    /**
     * @brief 错误发生信号
     * 
     * @param error 错误信息
     */
    void errorOccurred(const QString &error);

private:
    /**
     * @brief 发送HTTP请求
     * 
     * @param endpoint 端点路径
     * @param request 请求数据
     * @param callback 回调函数
     * @param retryAttempt 重试次数
     */
    void sendRequest(const QString &endpoint, 
                    const QJsonObject &request,
                    std::function<void(const QJsonObject&)> callback,
                    int retryAttempt = 0);
    
    /**
     * @brief 处理请求错误
     * 
     * @param endpoint 端点路径
     * @param request 原始请求
     * @param callback 回调函数
     * @param retryAttempt 当前重试次数
     * @param error 错误信息
     */
    void handleRequestError(const QString &endpoint,
                           const QJsonObject &request,
                           std::function<void(const QJsonObject&)> callback,
                           int retryAttempt,
                           const QString &error);
    
    /**
     * @brief 处理连接错误
     * 
     * @param error 错误信息
     */
    void handleConnectionError(const QString &error);

private:
    /**
     * @brief 网络访问管理器
     */
    QNetworkAccessManager *m_networkManager;
    
    /**
     * @brief 连接状态
     */
    bool m_connected;
    
    /**
     * @brief 服务地址
     */
    QString m_serviceUrl;
    
    /**
     * @brief 请求超时时间（毫秒）
     */
    int m_timeout;
    
    /**
     * @brief 重试次数
     */
    int m_retryCount;
};
