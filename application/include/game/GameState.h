/*
 * 文件名: GameState.h
 * 说明: 游戏状态管理器头文件 - 管理游戏的全局状态和数据
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 * 版本: v1.0.0
 * 
 * 功能描述:
 * - 管理游戏的全局状态（菜单、游戏中、暂停等）
 * - 存储和管理玩家数据（等级、经验、金币等）
 * - 提供游戏进度和世界状态的持久化
 * - 支持状态变更的信号通知
 * - 实现状态数据的线程安全访问
 * 
 * 设计模式:
 * - 单例模式：确保全局唯一的游戏状态
 * - 观察者模式：通过Qt信号槽通知状态变更
 * - 状态模式：管理不同的游戏状态转换
 * 
 * 依赖项:
 * - Qt Core (QObject, QVariant, QMap等)
 * - Qt QML (Q_PROPERTY宏，用于QML绑定)
 * 
 * 使用示例:
 * GameState* gameState = GameState::instance();
 * gameState->setState(GameState::State::InGame);
 * gameState->setPlayerLevel(5);
 * int level = gameState->getPlayerLevel();
 */

#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <QObject>
#include <QVariant>
#include <QMap>
#include <QMutex>
#include <QDateTime>

/**
 * @brief 游戏状态管理器
 * 
 * 这是一个核心类，负责管理游戏的全局状态和玩家数据。
 * 它提供了线程安全的状态访问和修改接口，支持状态持久化，
 * 并通过Qt信号槽机制通知状态变更。
 * 
 * 主要职责：
 * 1. 游戏状态管理（菜单、游戏中、暂停等）
 * 2. 玩家数据管理（等级、经验、属性等）
 * 3. 游戏进度跟踪（当前场景、任务状态等）
 * 4. 状态持久化（保存/加载游戏状态）
 * 5. 状态变更通知（信号发射）
 */
class GameState : public QObject
{
    Q_OBJECT
    
    // QML属性绑定，允许在QML中直接访问和绑定这些属性
    Q_PROPERTY(int playerLevel READ getPlayerLevel WRITE setPlayerLevel NOTIFY playerLevelChanged)
    Q_PROPERTY(int playerExperience READ getPlayerExperience WRITE setPlayerExperience NOTIFY playerExperienceChanged)
    Q_PROPERTY(int playerGold READ getPlayerGold WRITE setPlayerGold NOTIFY playerGoldChanged)
    Q_PROPERTY(QString currentScene READ getCurrentScene WRITE setCurrentScene NOTIFY currentSceneChanged)
    Q_PROPERTY(QString playerName READ getPlayerName WRITE setPlayerName NOTIFY playerNameChanged)
    Q_PROPERTY(bool isGamePaused READ isGamePaused NOTIFY gamePausedChanged)
    
public:
    /**
     * @brief 游戏状态枚举
     * 
     * 定义游戏可能处于的各种状态，用于控制游戏流程和UI显示
     */
    enum class State {
        Uninitialized,  ///< 未初始化状态
        MainMenu,       ///< 主菜单状态
        InGame,         ///< 游戏进行中
        Paused,         ///< 游戏暂停
        Loading,        ///< 加载中
        Battle,         ///< 战斗状态
        Inventory,      ///< 物品栏界面
        Settings,       ///< 设置界面
        GameOver,       ///< 游戏结束
        Victory         ///< 胜利状态
    };
    Q_ENUM(State)  // 使枚举在QML中可用
    
    /**
     * @brief 玩家职业枚举
     */
    enum class PlayerClass {
        None,           ///< 未选择职业
        Warrior,        ///< 战士
        Mage,           ///< 法师
        Archer          ///< 弓箭手
    };
    Q_ENUM(PlayerClass)
    
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit GameState(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~GameState();
    
    // ==================== 游戏状态控制 ====================
    
    /**
     * @brief 获取当前游戏状态
     * @return State 当前状态
     */
    State getCurrentState() const { return m_currentState; }
    
    /**
     * @brief 设置游戏状态
     * @param state 新的游戏状态
     * 
     * 状态转换会触发相应的信号，并执行状态切换逻辑
     */
    void setState(State state);
    
    /**
     * @brief 检查游戏是否暂停
     * @return bool 是否暂停
     */
    bool isGamePaused() const { return m_currentState == State::Paused; }
    
    // ==================== 玩家数据管理 ====================
    
    /**
     * @brief 获取玩家等级
     * @return int 玩家等级
     */
    int getPlayerLevel() const { return m_playerLevel; }
    
    /**
     * @brief 设置玩家等级
     * @param level 新的等级值
     * 
     * 等级变化会触发相关的游戏逻辑更新
     */
    void setPlayerLevel(int level);
    
    /**
     * @brief 获取玩家经验值
     * @return int 当前经验值
     */
    int getPlayerExperience() const { return m_playerExperience; }
    
    /**
     * @brief 设置玩家经验值
     * @param experience 新的经验值
     * 
     * 经验值变化可能触发升级逻辑
     */
    void setPlayerExperience(int experience);
    
    /**
     * @brief 增加玩家经验值
     * @param amount 增加的经验值
     * @return bool 是否触发了升级
     */
    bool addPlayerExperience(int amount);
    
    /**
     * @brief 获取玩家金币数量
     * @return int 金币数量
     */
    int getPlayerGold() const { return m_playerGold; }
    
    /**
     * @brief 设置玩家金币数量
     * @param gold 新的金币数量
     */
    void setPlayerGold(int gold);
    
    /**
     * @brief 获取玩家姓名
     * @return QString 玩家姓名
     */
    QString getPlayerName() const { return m_playerName; }
    
    /**
     * @brief 设置玩家姓名
     * @param name 新的玩家姓名
     */
    void setPlayerName(const QString &name);
    
    /**
     * @brief 获取玩家职业
     * @return PlayerClass 玩家职业
     */
    PlayerClass getPlayerClass() const { return m_playerClass; }
    
    /**
     * @brief 设置玩家职业
     * @param playerClass 新的职业
     */
    void setPlayerClass(PlayerClass playerClass);
    
    // ==================== 场景和进度管理 ====================
    
    /**
     * @brief 获取当前场景名称
     * @return QString 场景名称
     */
    QString getCurrentScene() const { return m_currentScene; }
    
    /**
     * @brief 设置当前场景
     * @param scene 场景名称
     */
    void setCurrentScene(const QString &scene);
    
    /**
     * @brief 获取游戏进度百分比
     * @return float 进度百分比 (0.0-1.0)
     */
    float getGameProgress() const { return m_gameProgress; }
    
    /**
     * @brief 设置游戏进度
     * @param progress 进度百分比 (0.0-1.0)
     */
    void setGameProgress(float progress);
    
    // ==================== 通用状态数据 ====================
    
    /**
     * @brief 获取状态数据
     * @param key 数据键名
     * @param defaultValue 默认值
     * @return QVariant 数据值
     * 
     * 提供灵活的键值对数据存储，用于存储各种游戏状态信息
     */
    QVariant getStateData(const QString &key, const QVariant &defaultValue = QVariant()) const;
    
    /**
     * @brief 设置状态数据
     * @param key 数据键名
     * @param value 数据值
     * 
     * 线程安全的数据设置方法
     */
    void setStateData(const QString &key, const QVariant &value);
    
    /**
     * @brief 移除状态数据
     * @param key 要移除的数据键名
     * @return bool 是否成功移除
     */
    bool removeStateData(const QString &key);
    
    /**
     * @brief 清空所有状态数据
     */
    void clearStateData();
    
    // ==================== 状态持久化 ====================
    
    /**
     * @brief 保存游戏状态到文件
     * @param filePath 保存文件路径，为空则使用默认路径
     * @return bool 保存是否成功
     * 
     * 将当前的游戏状态序列化并保存到文件
     */
    bool saveState(const QString &filePath = QString());
    
    /**
     * @brief 从文件加载游戏状态
     * @param filePath 加载文件路径，为空则使用默认路径
     * @return bool 加载是否成功
     * 
     * 从文件反序列化游戏状态数据
     */
    bool loadState(const QString &filePath = QString());
    
    /**
     * @brief 重置游戏状态到初始值
     * 
     * 将所有状态数据重置为默认值，用于新游戏开始
     */
    void resetToDefaults();
    
    // ==================== 实用方法 ====================
    
    /**
     * @brief 获取状态名称字符串
     * @param state 状态枚举值
     * @return QString 状态名称
     */
    static QString stateToString(State state);
    
    /**
     * @brief 获取默认保存文件路径
     * @return QString 默认保存路径
     */
    QString getDefaultSavePath() const;

signals:
    // ==================== 状态变更信号 ====================
    
    /**
     * @brief 游戏状态变更信号
     * @param newState 新状态
     * @param oldState 旧状态
     */
    void stateChanged(State newState, State oldState);
    
    /**
     * @brief 游戏暂停状态变更信号
     * @param paused 是否暂停
     */
    void gamePausedChanged(bool paused);
    
    // ==================== 玩家数据变更信号 ====================
    
    /**
     * @brief 玩家等级变更信号
     * @param level 新等级
     */
    void playerLevelChanged(int level);
    
    /**
     * @brief 玩家经验值变更信号
     * @param experience 新经验值
     */
    void playerExperienceChanged(int experience);
    
    /**
     * @brief 玩家升级信号
     * @param newLevel 新等级
     * @param oldLevel 旧等级
     */
    void playerLevelUp(int newLevel, int oldLevel);
    
    /**
     * @brief 玩家金币变更信号
     * @param gold 新金币数量
     */
    void playerGoldChanged(int gold);
    
    /**
     * @brief 玩家姓名变更信号
     * @param name 新姓名
     */
    void playerNameChanged(const QString &name);
    
    /**
     * @brief 玩家职业变更信号
     * @param playerClass 新职业
     */
    void playerClassChanged(PlayerClass playerClass);
    
    // ==================== 场景和进度信号 ====================
    
    /**
     * @brief 当前场景变更信号
     * @param scene 新场景名称
     */
    void currentSceneChanged(const QString &scene);
    
    /**
     * @brief 游戏进度变更信号
     * @param progress 新进度值
     */
    void gameProgressChanged(float progress);
    
    // ==================== 通用数据信号 ====================
    
    /**
     * @brief 状态数据变更信号
     * @param key 数据键名
     * @param value 新数据值
     */
    void stateDataChanged(const QString &key, const QVariant &value);

private:
    // ==================== 私有成员变量 ====================
    
    // 游戏状态
    State m_currentState;                   ///< 当前游戏状态
    State m_previousState;                  ///< 前一个游戏状态
    
    // 玩家基本数据
    int m_playerLevel;                      ///< 玩家等级
    int m_playerExperience;                 ///< 玩家经验值
    int m_playerGold;                       ///< 玩家金币
    QString m_playerName;                   ///< 玩家姓名
    PlayerClass m_playerClass;              ///< 玩家职业
    
    // 游戏进度数据
    QString m_currentScene;                 ///< 当前场景名称
    float m_gameProgress;                   ///< 游戏进度 (0.0-1.0)
    QDateTime m_lastSaveTime;               ///< 最后保存时间
    
    // 通用状态数据存储
    QMap<QString, QVariant> m_stateData;    ///< 键值对状态数据
    
    // 线程安全
    mutable QMutex m_mutex;                 ///< 互斥锁，保证线程安全
    
    // ==================== 私有方法 ====================
    
    /**
     * @brief 初始化默认值
     */
    void initializeDefaults();
    
    /**
     * @brief 验证状态转换的合法性
     * @param fromState 源状态
     * @param toState 目标状态
     * @return bool 转换是否合法
     */
    bool isValidStateTransition(State fromState, State toState) const;
    
    /**
     * @brief 计算升级所需经验值
     * @param level 目标等级
     * @return int 所需经验值
     */
    int calculateExperienceForLevel(int level) const;
};

#endif // GAMESTATE_H
