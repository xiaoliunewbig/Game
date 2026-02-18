/*
 * 文件名: BattleSystem.h
 * 说明: 战斗系统类，管理回合制战斗逻辑。
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 *
 * 本类管理游戏的战斗系统，包括回合管理、伤害计算、
 * 技能释放和战斗结果处理。
 */
#pragma once

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include <QTimer>
#include <QList>

class Player;
class AlgorithmServiceClient;

/**
 * @brief 战斗状态枚举
 */
enum class BattleState {
    Idle,           ///< 空闲状态
    Starting,       ///< 战斗开始
    PlayerTurn,     ///< 玩家回合
    EnemyTurn,      ///< 敌人回合
    Victory,        ///< 胜利
    Defeat          ///< 失败
};

/**
 * @brief 敌人数据结构
 */
struct Enemy {
    int id;                 ///< 敌人ID
    QString name;           ///< 敌人名称
    int health;             ///< 当前生命值
    int maxHealth;          ///< 最大生命值
    int attack;             ///< 攻击力
    int defense;            ///< 防御力
    int speed;              ///< 速度
    int level;              ///< 等级
};

/**
 * @brief 战斗系统类
 * 
 * 该类管理游戏的回合制战斗系统，包括：
 * - 战斗流程控制
 * - 回合顺序管理
 * - 伤害计算协调
 * - 技能和物品使用
 * - 自动战斗模式
 * - 战斗日志记录
 * 
 * 通过Q_PROPERTY暴露数据给QML，实现实时的战斗界面更新。
 */
class BattleSystem : public QObject
{
    Q_OBJECT
    
    // QML可访问的属性
    Q_PROPERTY(BattleState battleState READ getBattleState NOTIFY battleStateChanged)
    Q_PROPERTY(QJsonArray enemies READ getEnemies NOTIFY enemiesChanged)
    Q_PROPERTY(QStringList battleLog READ getBattleLog NOTIFY battleLogChanged)
    Q_PROPERTY(int currentTurn READ getCurrentTurn NOTIFY turnChanged)
    Q_PROPERTY(QJsonArray turnOrder READ getTurnOrder NOTIFY turnOrderChanged)
    Q_PROPERTY(float battleSpeed READ getBattleSpeed WRITE setBattleSpeed NOTIFY battleSpeedChanged)
    Q_PROPERTY(bool autoMode READ isAutoMode WRITE setAutoMode NOTIFY autoModeChanged)

public:
    /**
     * @brief 构造函数
     * 
     * @param parent 父对象指针
     */
    explicit BattleSystem(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~BattleSystem();

    /**
     * @brief 初始化战斗系统
     * 
     * @param player 玩家对象指针
     * @param algorithmClient 算法服务客户端指针
     * @return bool 是否初始化成功
     */
    bool initialize(Player *player, AlgorithmServiceClient *algorithmClient);

    // 战斗控制接口
    
    /**
     * @brief 开始战斗
     * 
     * @param battleConfig 战斗配置JSON
     * @return bool 是否成功开始战斗
     */
    bool startBattle(const QJsonObject &battleConfig);
    
    /**
     * @brief 结束战斗
     */
    void endBattle();

    // 玩家行动接口
    
    /**
     * @brief 玩家攻击
     * 
     * @param targetId 目标敌人ID
     * @param skillId 技能ID（0为普通攻击）
     * @return bool 是否执行成功
     */
    Q_INVOKABLE bool playerAttack(int targetId, int skillId = 0);
    
    /**
     * @brief 玩家使用物品
     * 
     * @param itemId 物品ID
     * @param targetId 目标ID（-1为自己）
     * @return bool 是否执行成功
     */
    Q_INVOKABLE bool playerUseItem(int itemId, int targetId = -1);
    
    /**
     * @brief 玩家防御
     * 
     * @return bool 是否执行成功
     */
    Q_INVOKABLE bool playerDefend();
    
    /**
     * @brief 跳过回合
     */
    Q_INVOKABLE void skipTurn();

    // 战斗设置
    
    /**
     * @brief 设置战斗速度
     * 
     * @param speed 战斗速度倍率（0.5-3.0）
     */
    void setBattleSpeed(float speed);
    
    /**
     * @brief 获取战斗速度
     * 
     * @return float 当前战斗速度
     */
    float getBattleSpeed() const;
    
    /**
     * @brief 设置自动战斗模式
     * 
     * @param enabled 是否启用自动模式
     */
    void setAutoMode(bool enabled);
    
    /**
     * @brief 获取自动战斗模式状态
     * 
     * @return bool 是否为自动模式
     */
    bool isAutoMode() const;

    // 状态查询接口
    
    /**
     * @brief 获取战斗状态
     * 
     * @return BattleState 当前战斗状态
     */
    BattleState getBattleState() const;
    
    /**
     * @brief 获取敌人列表
     * 
     * @return QJsonArray 敌人数据的JSON数组
     */
    QJsonArray getEnemies() const;
    
    /**
     * @brief 获取战斗日志
     * 
     * @return QStringList 战斗日志列表
     */
    QStringList getBattleLog() const;
    
    /**
     * @brief 获取当前回合数
     * 
     * @return int 当前回合数
     */
    int getCurrentTurn() const;
    
    /**
     * @brief 获取回合顺序
     * 
     * @return QJsonArray 回合顺序的JSON数组
     */
    QJsonArray getTurnOrder() const;

    /**
     * @brief 更新战斗逻辑
     * 
     * @param deltaTime 帧时间间隔
     */
    void update(float deltaTime);

signals:
    /**
     * @brief 战斗开始信号
     */
    void battleStarted();
    
    /**
     * @brief 战斗结束信号
     */
    void battleEnded();
    
    /**
     * @brief 战斗胜利信号
     */
    void battleWon();
    
    /**
     * @brief 战斗失败信号
     */
    void battleDefeated();
    
    /**
     * @brief 战斗状态改变信号
     */
    void battleStateChanged();
    
    /**
     * @brief 玩家回合开始信号
     */
    void playerTurnStarted();
    
    /**
     * @brief 玩家回合结束信号
     */
    void playerTurnEnded();
    
    /**
     * @brief 敌人回合开始信号
     */
    void enemyTurnStarted();
    
    /**
     * @brief 敌人回合结束信号
     */
    void enemyTurnEnded();
    
    /**
     * @brief 敌人列表改变信号
     */
    void enemiesChanged();
    
    /**
     * @brief 敌人被击败信号
     * 
     * @param enemyId 被击败的敌人ID
     */
    void enemyDefeated(int enemyId);
    
    /**
     * @brief 战斗日志改变信号
     */
    void battleLogChanged();
    
    /**
     * @brief 回合数改变信号
     * 
     * @param turn 新的回合数
     */
    void turnChanged(int turn);
    
    /**
     * @brief 回合顺序改变信号
     */
    void turnOrderChanged();
    
    /**
     * @brief 战斗速度改变信号
     * 
     * @param speed 新的战斗速度
     */
    void battleSpeedChanged(float speed);
    
    /**
     * @brief 自动模式改变信号
     * 
     * @param enabled 是否启用自动模式
     */
    void autoModeChanged(bool enabled);

private slots:
    /**
     * @brief 回合超时处理
     */
    void onTurnTimeout();
    
    /**
     * @brief 玩家死亡处理
     */
    void onPlayerDied();

private:
    /**
     * @brief 解析战斗配置
     * 
     * @param config 战斗配置JSON
     * @return bool 是否解析成功
     */
    bool parseBattleConfig(const QJsonObject &config);
    
    /**
     * @brief 计算回合顺序
     */
    void calculateTurnOrder();
    
    /**
     * @brief 开始玩家回合
     */
    void startPlayerTurn();
    
    /**
     * @brief 结束玩家回合
     */
    void endPlayerTurn();
    
    /**
     * @brief 开始敌人回合
     */
    void startEnemyTurn();
    
    /**
     * @brief 结束敌人回合
     */
    void endEnemyTurn();
    
    /**
     * @brief 执行玩家攻击
     * 
     * @param target 目标敌人
     * @param skillId 技能ID
     */
    void executePlayerAttack(Enemy &target, int skillId);
    
    /**
     * @brief 执行玩家物品使用
     * 
     * @param itemId 物品ID
     * @param targetId 目标ID
     */
    void executePlayerItemUse(int itemId, int targetId);
    
    /**
     * @brief 执行敌人行动
     */
    void executeEnemyActions();
    
    /**
     * @brief 执行单个敌人行动
     * 
     * @param enemy 敌人对象
     */
    void executeEnemyAction(Enemy &enemy);
    
    /**
     * @brief 执行自动行动
     */
    void executeAutoAction();
    
    /**
     * @brief 检查战斗是否结束
     * 
     * @return bool 是否结束
     */
    bool checkBattleEnd();
    
    /**
     * @brief 添加战斗日志
     * 
     * @param message 日志消息
     */
    void addBattleLog(const QString &message);

private:
    /**
     * @brief 玩家对象指针
     */
    Player *m_player;
    
    /**
     * @brief 算法服务客户端指针
     */
    AlgorithmServiceClient *m_algorithmClient;
    
    /**
     * @brief 当前战斗状态
     */
    BattleState m_battleState;
    
    /**
     * @brief 敌人列表
     */
    QList<Enemy> m_enemies;
    
    /**
     * @brief 回合顺序（实体ID列表，-1表示玩家）
     */
    QList<int> m_turnOrder;
    
    /**
     * @brief 当前回合数
     */
    int m_currentTurn;
    
    /**
     * @brief 战斗日志
     */
    QStringList m_battleLog;
    
    /**
     * @brief 回合定时器
     */
    QTimer *m_turnTimer;
    
    /**
     * @brief 回合持续时间（毫秒）
     */
    int m_turnDuration;
    
    /**
     * @brief 战斗速度倍率
     */
    float m_battleSpeed;
    
    /**
     * @brief 自动战斗模式
     */
    bool m_autoMode;
};