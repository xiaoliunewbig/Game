/*
 * 文件名: GameEngine.h
 * 说明: 游戏引擎核心类，负责游戏逻辑的统一管理和协调。
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 本类是游戏的核心引擎，负责协调各个子系统的工作，
 * 管理游戏状态，处理游戏循环，以及与后端服务的通信。
 * 作为应用层的核心控制器。
 */
#pragma once

#include <QObject>
#include <QTimer>
#include <QJsonObject>
#include <memory>

// 前向声明
class SceneManager;
class ResourceManager;
class NetworkManager;
class AudioManager;
class GameState;
class Player;
class BattleSystem;
class InventorySystem;

/**
 * @brief 游戏状态枚举
 * 
 * 定义游戏的主要状态。
 */
enum class GameEngineState {
    Uninitialized,  ///< 未初始化
    MainMenu,       ///< 主菜单
    Loading,        ///< 加载中
    Playing,        ///< 游戏进行中
    Battle,         ///< 战斗中
    Paused,         ///< 暂停
    GameOver        ///< 游戏结束
};

/**
 * @brief 游戏引擎核心类
 * 
 * 该类是游戏的核心控制器，负责：
 * - 游戏状态管理和状态机控制
 * - 游戏循环的管理和更新调度
 * - 各个子系统的初始化和协调
 * - 与后端服务的通信协调
 * - 游戏数据的保存和加载
 * - 错误处理和异常恢复
 * 
 * 通过Q_PROPERTY暴露状态给QML，实现游戏状态的UI显示。
 */
class GameEngine : public QObject
{
    Q_OBJECT
    
    // QML可访问的属性
    Q_PROPERTY(GameEngineState currentState READ currentState NOTIFY stateChanged)
    Q_PROPERTY(QString stateDescription READ stateDescription NOTIFY stateChanged)
    Q_PROPERTY(bool isInitialized READ isInitialized NOTIFY initializationChanged)
    Q_PROPERTY(float deltaTime READ deltaTime NOTIFY deltaTimeChanged)
    Q_PROPERTY(int frameRate READ frameRate NOTIFY frameRateChanged)

public:
    /**
     * @brief 构造函数
     * 
     * 初始化游戏引擎，创建基础组件。
     * 
     * @param parent 父对象指针，用于Qt对象树管理
     */
    explicit GameEngine(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     * 
     * 清理游戏引擎，释放所有资源。
     */
    ~GameEngine();

    /**
     * @brief 初始化游戏引擎
     * 
     * 初始化所有子系统和游戏组件：
     * - 创建和初始化各个管理器
     * - 建立系统间的连接
     * - 加载游戏配置
     * - 启动游戏循环
     * 
     * @param sceneManager 场景管理器指针
     * @param resourceManager 资源管理器指针
     * @param networkManager 网络管理器指针
     * @param audioManager 音频管理器指针
     * @return bool 初始化是否成功
     * @retval true 初始化成功，游戏引擎可用
     * @retval false 初始化失败，游戏无法启动
     */
    bool initialize(SceneManager *sceneManager,
                   ResourceManager *resourceManager,
                   NetworkManager *networkManager,
                   AudioManager *audioManager);
    
    /**
     * @brief 启动游戏引擎
     * 
     * 启动游戏循环，开始游戏运行。
     */
    void start();
    
    /**
     * @brief 停止游戏引擎
     * 
     * 停止游戏循环，保存游戏状态。
     */
    void stop();

    // 状态查询属性访问器
    
    /**
     * @brief 获取当前游戏状态
     * 
     * @return GameEngineState 当前游戏状态
     */
    GameEngineState currentState() const { return m_currentState; }
    
    /**
     * @brief 获取状态描述
     * 
     * @return QString 当前状态的文字描述
     */
    QString stateDescription() const;
    
    /**
     * @brief 获取初始化状态
     * 
     * @return bool 是否已初始化
     */
    bool isInitialized() const { return m_isInitialized; }
    
    /**
     * @brief 获取帧时间间隔
     * 
     * @return float 上一帧的时间间隔（秒）
     */
    float deltaTime() const { return m_deltaTime; }
    
    /**
     * @brief 获取帧率
     * 
     * @return int 当前帧率（FPS）
     */
    int frameRate() const { return m_frameRate; }

    // 游戏控制接口
    
    /**
     * @brief 开始新游戏
     * 
     * 创建新的游戏存档，初始化玩家数据。
     * 
     * @param playerName 玩家角色名称
     * @param profession 玩家职业
     * @return bool 是否成功开始新游戏
     */
    bool startNewGame(const QString &playerName, const QString &profession);
    
    /**
     * @brief 加载游戏存档
     * 
     * 从指定存档槽位加载游戏数据。
     * 
     * @param saveSlot 存档槽位索引
     * @return bool 是否成功加载游戏
     */
    bool loadGame(int saveSlot);
    
    /**
     * @brief 保存游戏
     * 
     * 将当前游戏状态保存到指定槽位。
     * 
     * @param saveSlot 存档槽位索引
     * @return bool 是否成功保存游戏
     */
    bool saveGame(int saveSlot);
    
    /**
     * @brief 暂停游戏
     * 
     * 暂停游戏循环和逻辑更新。
     */
    void pauseGame();
    
    /**
     * @brief 恢复游戏
     * 
     * 恢复游戏循环和逻辑更新。
     */
    void resumeGame();

    // QML可调用的游戏操作接口
    
    /**
     * @brief 获取存档槽位信息
     * 
     * 获取指定存档槽位的详细信息。
     * 
     * @param slotIndex 存档槽位索引
     * @return QJsonObject 存档信息的JSON表示
     */
    Q_INVOKABLE QJsonObject getSaveSlotInfo(int slotIndex) const;
    
    /**
     * @brief 删除存档
     * 
     * 删除指定槽位的存档数据。
     * 
     * @param slotIndex 存档槽位索引
     * @return bool 是否成功删除
     */
    Q_INVOKABLE bool deleteSave(int slotIndex);
    
    /**
     * @brief 获取游戏统计信息
     * 
     * 获取当前游戏的统计数据。
     * 
     * @return QJsonObject 游戏统计信息
     */
    Q_INVOKABLE QJsonObject getGameStats() const;
    
    /**
     * @brief 重置游戏设置
     * 
     * 将游戏设置重置为默认值。
     */
    Q_INVOKABLE void resetSettings();
    
    /**
     * @brief 退出到主菜单
     * 
     * 保存当前进度并返回主菜单。
     */
    Q_INVOKABLE void exitToMainMenu();
    
    /**
     * @brief 强制垃圾回收
     * 
     * 触发内存清理和资源回收。
     */
    Q_INVOKABLE void forceGarbageCollection();

signals:
    /**
     * @brief 游戏状态改变信号
     */
    void stateChanged();
    
    /**
     * @brief 初始化状态改变信号
     */
    void initializationChanged();
    
    /**
     * @brief 帧时间改变信号
     */
    void deltaTimeChanged();
    
    /**
     * @brief 帧率改变信号
     */
    void frameRateChanged();
    
    /**
     * @brief 新游戏开始信号
     * 
     * @param playerName 玩家名称
     * @param profession 玩家职业
     */
    void newGameStarted(const QString &playerName, const QString &profession);
    
    /**
     * @brief 游戏加载完成信号
     * 
     * @param saveSlot 加载的存档槽位
     * @param success 是否加载成功
     */
    void gameLoaded(int saveSlot, bool success);
    
    /**
     * @brief 游戏保存完成信号
     * 
     * @param saveSlot 保存的存档槽位
     * @param success 是否保存成功
     */
    void gameSaved(int saveSlot, bool success);
    
    /**
     * @brief 游戏暂停信号
     */
    void gamePaused();
    
    /**
     * @brief 游戏恢复信号
     */
    void gameResumed();
    
    /**
     * @brief 错误发生信号
     * 
     * @param error 错误描述信息
     */
    void errorOccurred(const QString &error);

private slots:
    /**
     * @brief 游戏循环更新
     * 
     * 主游戏循环，处理游戏逻辑更新。
     */
    void update();
    
    /**
     * @brief 网络连接状态改变处理
     * 
     * @param connected 是否已连接
     */
    void onNetworkConnectionChanged(bool connected);
    
    /**
     * @brief 场景切换完成处理
     * 
     * @param sceneName 新场景名称
     */
    void onSceneTransitionCompleted(const QString &sceneName);

private:
    /**
     * @brief 改变游戏状态
     * 
     * 安全地改变游戏状态，触发相应的处理逻辑。
     * 
     * @param newState 新的游戏状态
     */
    void changeState(GameEngineState newState);
    
    /**
     * @brief 更新帧率统计
     * 
     * 计算和更新当前帧率。
     */
    void updateFrameRate();
    
    /**
     * @brief 处理游戏逻辑更新
     * 
     * 更新游戏中的各种逻辑系统。
     * 
     * @param deltaTime 帧时间间隔
     */
    void updateGameLogic(float deltaTime);
    
    /**
     * @brief 加载游戏配置
     * 
     * 从配置文件加载游戏设置。
     * 
     * @return bool 是否加载成功
     */
    bool loadGameConfig();
    
    /**
     * @brief 保存游戏配置
     * 
     * 将当前设置保存到配置文件。
     * 
     * @return bool 是否保存成功
     */
    bool saveGameConfig();

private:
    /**
     * @brief 游戏循环定时器
     * 
     * 控制游戏主循环的定时器。
     */
    QTimer *m_gameTimer;
    
    /**
     * @brief 场景管理器引用
     * 
     * 用于管理游戏场景的场景管理器。
     */
    SceneManager *m_sceneManager;
    
    /**
     * @brief 资源管理器引用
     * 
     * 用于管理游戏资源的资源管理器。
     */
    ResourceManager *m_resourceManager;
    
    /**
     * @brief 网络管理器引用
     * 
     * 用于网络通信的网络管理器。
     */
    NetworkManager *m_networkManager;
    
    /**
     * @brief 音频管理器引用
     * 
     * 用于音频播放的音频管理器。
     */
    AudioManager *m_audioManager;
    
    /**
     * @brief 游戏状态对象
     * 
     * 存储当前游戏的状态数据。
     */
    std::unique_ptr<GameState> m_gameState;
    
    /**
     * @brief 玩家对象
     * 
     * 当前玩家的数据和状态。
     */
    std::unique_ptr<Player> m_player;
    
    /**
     * @brief 战斗系统
     * 
     * 处理战斗逻辑的系统。
     */
    std::unique_ptr<BattleSystem> m_battleSystem;
    
    /**
     * @brief 背包系统
     * 
     * 处理物品和背包逻辑的系统。
     */
    std::unique_ptr<InventorySystem> m_inventorySystem;
    
    /**
     * @brief 当前游戏状态
     * 
     * 游戏引擎的当前状态。
     */
    GameEngineState m_currentState;
    
    /**
     * @brief 初始化状态
     * 
     * 游戏引擎是否已初始化。
     */
    bool m_isInitialized;
    
    /**
     * @brief 帧时间间隔
     * 
     * 上一帧的时间间隔（秒）。
     */
    float m_deltaTime;
    
    /**
     * @brief 当前帧率
     * 
     * 当前的帧率（FPS）。
     */
    int m_frameRate;
    
    /**
     * @brief 上一帧时间
     * 
     * 上一帧的时间戳，用于计算帧时间间隔。
     */
    qint64 m_lastFrameTime;
    
    /**
     * @brief 帧计数器
     * 
     * 用于计算帧率的帧计数器。
     */
    int m_frameCounter;
    
    /**
     * @brief 帧率计算时间
     * 
     * 上次计算帧率的时间戳。
     */
    qint64 m_frameRateTime;
    
    /**
     * @brief 游戏配置
     * 
     * 存储游戏的配置设置。
     */
    QJsonObject m_gameConfig;
    
    /**
     * @brief 目标帧率
     * 
     * 游戏循环的目标帧率。
     */
    static const int TARGET_FPS = 60;
};
