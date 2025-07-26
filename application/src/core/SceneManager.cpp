/*
 * 文件名: SceneManager.cpp
 * 说明: 场景管理器实现 - 负责游戏场景的加载、切换和生命周期管理
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 * 版本: v1.0.0
 */

#include "core/SceneManager.h"
#include "core/EventSystem.h"
#include "core/ResourceManager.h"
#include <QDebug>

SceneManager::SceneManager(QObject *parent)
    : QObject(parent)
    , m_currentScene(nullptr)
    , m_isTransitioning(false)
    , m_transitionProgress(0.0f)
    , m_currentSceneName("")
    , m_resourceManager(nullptr)
{
    qDebug() << "SceneManager: 构造函数调用，初始化基本状态";
}

SceneManager::~SceneManager()
{
    qDebug() << "SceneManager: 析构函数调用，开始清理资源";
    cleanup();
    qDebug() << "SceneManager: 析构完成";
}

bool SceneManager::initialize(ResourceManager *resourceManager)
{
    qDebug() << "SceneManager: 开始初始化";
    
    if (!resourceManager) {
        qCritical() << "SceneManager: ResourceManager为空";
        return false;
    }
    
    m_resourceManager = resourceManager;
    
    try {
        // 注册所有内置场景类型
        registerSceneTypes();
        
        qDebug() << "SceneManager: 初始化成功";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "SceneManager: 初始化失败:" << e.what();
        return false;
    }
}

void SceneManager::registerSceneTypes()
{
    qDebug() << "SceneManager: 开始注册场景类型";
    
    // TODO: 实际的场景类型注册逻辑
    // 示例：
    // registerScene("MainMenu", []() { return new MainMenuScene(); });
    // registerScene("Gameplay", []() { return new GameplayScene(); });
    // registerScene("Battle", []() { return new BattleScene(); });
    
    qDebug() << "SceneManager: 场景类型注册完成";
}

void SceneManager::loadScene(const QString &sceneName)
{
    qDebug() << "SceneManager: 请求加载场景:" << sceneName;
    
    // 防止重复切换场景
    if (m_isTransitioning) {
        qWarning() << "SceneManager: 场景切换进行中，忽略加载请求:" << sceneName;
        return;
    }
    
    // 验证场景名称
    if (sceneName.isEmpty()) {
        qWarning() << "SceneManager: 场景名称为空，加载失败";
        return;
    }
    
    // 设置切换状态标志
    m_isTransitioning = true;
    
    try {
        // 卸载当前场景
        if (m_currentScene) {
            qDebug() << "SceneManager: 卸载当前场景";
            // m_currentScene->onExit();
            m_currentScene = nullptr;
        }
        
        // 加载新场景
        qDebug() << "SceneManager: 开始加载新场景:" << sceneName;
        m_currentSceneName = sceneName;
        
        // 重置切换状态
        m_isTransitioning = false;
        
        // 发送场景切换完成信号
        emit sceneTransitionCompleted(sceneName);
        qDebug() << "SceneManager: 场景加载完成:" << sceneName;
        
    } catch (const std::exception& e) {
        qCritical() << "SceneManager: 场景加载异常:" << e.what();
        m_isTransitioning = false;
    }
}

QString SceneManager::getCurrentSceneName() const
{
    return m_currentSceneName;
}

bool SceneManager::pushScene(const QString &sceneName, TransitionType transitionType, const QJsonObject &sceneData)
{
    Q_UNUSED(transitionType)
    Q_UNUSED(sceneData)
    
    // 将当前场景名称推入堆栈
    if (!m_currentSceneName.isEmpty()) {
        m_sceneStack.push(m_currentSceneName);
    }
    
    loadScene(sceneName);
    return true;
}

bool SceneManager::popScene(TransitionType transitionType, const QJsonObject &returnData)
{
    Q_UNUSED(transitionType)
    Q_UNUSED(returnData)
    
    if (m_sceneStack.isEmpty()) {
        qWarning() << "SceneManager: 场景堆栈为空，无法弹出";
        return false;
    }
    
    QString previousScene = m_sceneStack.pop();
    loadScene(previousScene);
    return true;
}

void SceneManager::cleanup()
{
    qDebug() << "SceneManager: 开始清理资源";
    
    // 卸载当前场景
    if (m_currentScene) {
        qDebug() << "SceneManager: 卸载当前场景";
        m_currentScene = nullptr;
    }
    
    // 清理场景堆栈
    m_sceneStack.clear();
    
    // 重置状态
    m_isTransitioning = false;
    m_currentSceneName.clear();
    
    qDebug() << "SceneManager: 资源清理完成";
}
