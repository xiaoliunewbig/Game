/*
 * 文件名: GameApplication.cpp
 * 说明: 游戏应用程序主类的具体实现。
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 本文件实现GameApplication类的所有方法，负责应用程序的完整生命周期管理，
 * 包括初始化、启动、运行和关闭流程。协调各个子系统的工作，
 * 提供稳定可靠的应用程序框架。
 */
#include "GameApplication.h"
#include "core/GameEngine.h"
#include "network/NetworkManager.h"
#include "audio/AudioManager.h"
#include "core/ResourceManager.h"
#include "game/GameState.h"

#include <QQmlContext>
#include <QQuickStyle>
#include <QApplication> 
#include <QDir>

/**
 * @brief 构造函数实现
 * 
 * 初始化游戏应用程序，设置初始状态。
 * 各个子系统将在initialize()方法中创建。
 * 
 * @param parent 父对象指针
 */
GameApplication::GameApplication(QObject *parent)
    : QObject(parent)
{
    // 子系统将在initialize()中创建
    // 这里只进行基础的对象初始化
}

/**
 * @brief 析构函数实现
 * 
 * 使用默认析构函数，智能指针会自动清理子系统。
 * 确保所有资源正确释放。
 */
GameApplication::~GameApplication() = default;

/**
 * @brief 应用程序初始化实现
 * 
 * 执行完整的初始化流程，创建并配置所有子系统。
 * 
 * @return bool 初始化结果
 */
bool GameApplication::initialize()
{
    try {
        // 1. 创建子系统实例（关键修复）
        m_resourceManager = std::make_unique<ResourceManager>();
        m_sceneManager = std::make_unique<SceneManager>();
        m_networkManager = std::make_unique<NetworkManager>();
        m_audioManager = std::make_unique<AudioManager>();
        m_gameEngine = std::make_unique<GameEngine>();
        
        //2.  初始化资源管理器
        if (!m_resourceManager->initialize()) {
            qCritical() << "GameApplication: 资源管理器初始化失败";
            return false;
        }
        
        //3. 初始化场景管理器
        if (!m_sceneManager->initialize(m_resourceManager)) {
            qCritical() << "GameApplication: 场景管理器初始化失败";
            return false;
        }
        
        //4. 初始化游戏引擎
        if (!m_gameEngine->initialize(m_sceneManager, m_resourceManager, 
                                     m_networkManager, m_audioManager)) {
            qCritical() << "GameApplication: 游戏引擎初始化失败";
            return false;
        }
        
        // 5. 注册QML类型和建立连接
        registerQmlTypes();
        setupConnections();
        
        m_isInitialized = true;
        //emit initializationChanged();
        qDebug() << "GameApplication: 应用程序初始化成功";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "GameApplication: 初始化异常:" << e.what();
        return false;
    }
}

/**
 * @brief 启动应用程序实现
 * 
 * 加载主QML文件，显示应用程序界面。
 */
void GameApplication::start()
{
    if (!m_isInitialized) {
        qCritical() << "GameApplication: 未初始化，无法启动";
        return;
    }

    // 设置UI样式
    QQuickStyle::setStyle("Material");

    // 向QML暴露核心管理器（关键补充）
    m_engine.rootContext()->setContextProperty("gameEngine", m_gameEngine.get());
    m_engine.rootContext()->setContextProperty("inventorySystem", m_gameEngine->getInventorySystem());  
    
    // 加载主QML文件
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    m_engine.load(url);
    
    // 检查QML是否加载成功
    if (m_engine.rootObjects().isEmpty()) {
        qCritical() << "GameApplication: QML文件加载失败";
    }
}

/**
 * @brief 关闭应用程序实现
 * 
 * 执行优雅关闭流程，保存状态并清理资源。
 */
void GameApplication::shutdown()
{
    // 1. 停止游戏引擎
    if (m_gameEngine) {
        m_gameEngine->stop();
    }
    
    // 2. 关闭音频系统
    if (m_audioManager) {
        m_audioManager->shutdown();
    }
    
    // 3. 断开网络连接
    if (m_networkManager) {
        m_networkManager->disconnect();
    }
    
    // 4. 清理资源
    if (m_resourceManager) {
        m_resourceManager->cleanup();
    }
}

/**
 * @brief QML对象创建完成处理实现
 * 
 * 处理QML引擎的对象创建事件，进行错误检查。
 * 
 * @param obj 创建的QML对象
 * @param objUrl QML文件URL
 */
void GameApplication::onEngineObjectCreated(QObject* object, const QUrl& url)
{
    Q_UNUSED(object)
    Q_UNUSED(url)
    
    if (!object) {
        qCritical() << "GameApplication: QML引擎对象创建失败";
        QApplication::exit(-1);
    }
}

/**
 * @brief 注册QML类型实现
 * 
 * 向QML引擎注册自定义C++类型。
 */
void GameApplication::registerQmlTypes()
{
    // 注册游戏状态类型
    qmlRegisterType<GameState>("GameUI", 1, 0, "GameState");
    
    // 注册其他自定义类型
    // qmlRegisterType<OtherClass>("GameUI", 1, 0, "OtherClass");
}

/**
 * @brief 建立信号槽连接实现
 * 
 * 建立各个子系统之间的通信连接。
 */
void GameApplication::setupConnections()
{
    // 连接QML引擎信号
    connect(&m_engine, &QQmlApplicationEngine::objectCreated,
            this, &GameApplication::onEngineObjectCreated);
    
    // 连接游戏引擎信号
    if (m_gameEngine) {
        connect(m_gameEngine.get(), &GameEngine::errorOccurred,
                this, [this](const QString& error) {
                    // 处理游戏引擎错误
                    // 可以显示错误对话框或记录日志
                });
    }
    
    // 连接网络管理器信号
    if (m_networkManager) {
        connect(m_networkManager.get(), &NetworkManager::connectionLost,
                this, [this]() {
                    // 处理网络连接丢失
                    // 可以显示重连对话框
                });
    }
}
