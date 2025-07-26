/*
 * 文件名: GameApplication.h
 * 说明: 游戏应用程序主类，负责整个应用的生命周期管理。
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 本类作为游戏应用的入口点和协调中心，负责初始化各个子系统，
 * 管理QML引擎，协调游戏引擎、网络管理器、音频管理器等核心组件。
 * 提供统一的应用生命周期管理和错误处理机制。
 */
#pragma once

#include <QObject>
#include <QQmlApplicationEngine>
#include <memory>

// 前向声明核心组件
class GameEngine;
class NetworkManager;
class AudioManager;
class ResourceManager;

/**
 * @brief 游戏应用程序主类
 * 
 * 该类是整个游戏应用的核心控制器，负责：
 * - 应用程序的初始化和启动流程
 * - 各个子系统的创建和生命周期管理
 * - QML引擎的配置和管理
 * - 应用程序级别的事件处理
 * - 优雅的关闭和资源清理
 * 
 * 采用组合模式管理各个子系统，确保模块间的松耦合。
 */
class GameApplication : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * 
     * 初始化游戏应用程序对象，设置初始状态。
     * 不会立即创建子系统，需要调用initialize()方法。
     * 
     * @param parent 父对象指针，用于Qt对象树管理
     */
    explicit GameApplication(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     * 
     * 清理所有资源，确保各个子系统正确关闭。
     * 智能指针会自动管理子系统的生命周期。
     */
    ~GameApplication();

    /**
     * @brief 初始化应用程序
     * 
     * 执行应用程序的完整初始化流程，包括：
     * - 创建和初始化各个子系统
     * - 注册QML类型和组件
     * - 配置QML引擎
     * - 建立信号槽连接
     * 
     * @return bool 初始化是否成功
     * @retval true 初始化成功，应用可以启动
     * @retval false 初始化失败，应用无法正常运行
     * 
     * @note 该方法必须在start()之前调用
     * @see start() 启动应用程序
     */
    bool initialize();
    
    /**
     * @brief 启动应用程序
     * 
     * 启动游戏应用，显示主界面，开始游戏循环。
     * 加载主QML文件，显示应用程序窗口。
     * 
     * @pre initialize()必须已成功调用
     * @note 该方法会阻塞直到应用程序退出
     */
    void start();
    
    /**
     * @brief 关闭应用程序
     * 
     * 优雅地关闭应用程序，执行清理工作：
     * - 保存游戏状态和用户设置
     * - 关闭网络连接
     * - 停止音频播放
     * - 释放资源
     */
    void shutdown();

private slots:
    /**
     * @brief QML引擎对象创建完成处理
     * 
     * 当QML引擎创建对象时的回调函数，用于：
     * - 验证QML对象是否正确创建
     * - 处理QML加载错误
     * - 建立C++与QML之间的连接
     * 
     * @param obj 创建的QML对象指针
     * @param objUrl QML文件的URL
     */
    void onEngineObjectCreated(QObject *object, const QUrl &url);

private:
    /**
     * @brief 注册QML类型
     * 
     * 向QML引擎注册自定义的C++类型，使其可以在QML中使用。
     * 注册的类型包括：
     * - GameEngine：游戏引擎接口
     * - GameState：游戏状态数据
     * - 各种游戏组件和工具类
     */
    void registerQmlTypes();
    
    /**
     * @brief 建立信号槽连接
     * 
     * 建立各个子系统之间的信号槽连接，实现组件间通信：
     * - 游戏引擎与UI的数据绑定
     * - 网络事件的处理
     * - 音频事件的响应
     * - 错误处理和状态同步
     */
    void setupConnections();

private:
    /**
     * @brief QML应用引擎
     * 
     * 负责加载和管理QML界面，提供C++与QML的交互桥梁。
     * 管理QML组件的生命周期和属性绑定。
     */
    QQmlApplicationEngine m_engine;
    
    /**
     * @brief 游戏引擎实例
     * 
     * 游戏的核心逻辑引擎，负责：
     * - 游戏状态管理
     * - 游戏循环控制
     * - 场景管理
     * - 战斗系统协调
     */
    std::unique_ptr<GameEngine> m_gameEngine;
    
    /**
     * @brief 网络管理器实例
     * 
     * 负责与策略层和算法层的网络通信：
     * - gRPC客户端管理
     * - 网络请求调度
     * - 连接状态监控
     * - 网络错误处理
     */
    std::unique_ptr<NetworkManager> m_networkManager;
    
    /**
     * @brief 音频管理器实例
     * 
     * 负责游戏音频的播放和管理：
     * - 背景音乐播放
     * - 音效播放
     * - 音量控制
     * - 音频资源管理
     */
    std::unique_ptr<AudioManager> m_audioManager;
    
    /**
     * @brief 资源管理器实例
     * 
     * 负责游戏资源的加载和管理：
     * - 纹理资源加载
     * - 音频文件管理
     * - 配置文件读取
     * - 资源缓存策略
     */
    std::unique_ptr<ResourceManager> m_resourceManager;


    /**
     * @brief 场景管理器实例
     * 负责游戏场景的加载、切换和管理
     */
    std::unique_ptr<SceneManager> m_sceneManager;  // 添加场景管理器
    
    /**
     * @brief 初始化状态标志
     * 标记应用程序是否已完成初始化
     */
    bool m_isInitialized = false;  // 添加初始化状态标志
};
