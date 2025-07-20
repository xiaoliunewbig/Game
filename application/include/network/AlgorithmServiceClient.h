/*
 * 文件名: AlgorithmServiceClient.h
 * 说明: 算法服务客户端类，负责与Algorithm Service通信。
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 本类封装与算法服务的所有通信逻辑，包括伤害计算、
 * AI决策、技能树查询等功能。
 */
#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <functional>

class QNetworkAccessManager;

/**
 * @brief 算法服务客户端类
 * 
 * 该类负责与Algorithm Service进行通信，提供以下功能：
 * - 伤害计算请求
 * - AI决策请求
 * - 技能树查询
 * - 数据校验
 * - 连接管理和错误处理
 * 
 * 使用HTTP/JSON协议与服务端通信，支持异步回调和重试机制。
 */
class AlgorithmServiceClient : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * 
     * @param parent 父对象指针
     */
    explicit AlgorithmServiceClient(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~AlgorithmServiceClient();

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

    // 算法服务接口
    
    /**
     * @brief 计算伤害
     * 
     * @param request 伤害计算请求
     * @param callback 结果回调函数
     */
    void calculateDamage(const QJsonObject &request, 
                        std::function<void(const QJsonObject&)> callback);
    
    /**
     * @brief AI决策
     * 
     * @param request AI决策请求
     * @param callback 结果回调函数
     */
    void makeAIDecision(const QJsonObject &request,
                       std::function<void(const QJsonObject&)> callback);
    
    /**
     * @brief 获取技能树
     * 
     * @param request 技能树请求
     * @param callback 结果回调函数
     */
    void getSkillTree(const QJsonObject &request,
                     std::function<void(const QJsonObject&)> callback);
    
    /**
     * @brief 数据校验
     * 
     * @param request 校验请求
     * @param callback 结果回调函数
     */
    void validateData(const QJsonObject &request,
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
     * @brief 创建伤害计算请求
     * 
     * @param attackerId 攻击者ID
     * @param defenderId 防御者ID
     * @param skillId 技能ID
     * @param attackerLevel 攻击者等级
     * @param attackerAttack 攻击者攻击力
     * @param defenderDefense 防御者防御力
     * @return QJsonObject 请求对象
     */
    static QJsonObject createDamageRequest(int attackerId, int defenderId, 
                                          int skillId, int attackerLevel,
                                          int attackerAttack, int defenderDefense);
    
    /**
     * @brief 创建AI决策请求
     * 
     * @param npcId NPC ID
     * @param context 上下文数据
     * @return QJsonObject 请求对象
     */
    static QJsonObject createAIDecisionRequest(int npcId, const QJsonArray &context);
    
    /**
     * @brief 创建技能树请求
     * 
     * @param playerId 玩家ID
     * @param profession 职业
     * @return QJsonObject 请求对象
     */
    static QJsonObject createSkillTreeRequest(int playerId, const QString &profession);

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