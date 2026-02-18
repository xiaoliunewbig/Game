/*
 * 文件名: SceneManager.cpp
 * 说明: 场景管理器实现 - 负责游戏场景的加载、切换和生命周期管理
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 * 版本: v1.0.0
 */

#include "core/SceneManager.h"
#include "core/Scene.h"
#include "core/MainMenuScene.h"
#include "core/GameplayScene.h"
#include "core/BattleScene.h"
#include "core/LoadingScene.h"
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

    // 注册所有支持的场景名称（使用空场景占位，实际场景在加载时创建）
    static const QStringList sceneNames = {
        "MainMenu", "Loading", "Gameplay", "Battle",
        "Inventory", "CharacterInfo", "SkillTree",
        "WorldMap", "Settings", "SaveLoad",
        "GameOver", "Cutscene"
    };

    for (const QString &name : sceneNames) {
        if (!m_scenes.contains(name)) {
            m_scenes[name] = nullptr;  // 占位，延迟创建
        }
    }

    qDebug() << "SceneManager: 场景类型注册完成，共" << sceneNames.size() << "种场景";
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
        // 退出并卸载当前场景
        if (m_currentScene) {
            qDebug() << "SceneManager: 退出当前场景";
            m_currentScene->onExit();
            m_currentScene = nullptr;
        }

        // 创建新场景
        auto newScene = createScene(sceneName);
        if (newScene) {
            newScene->load(m_resourceManager);
            QJsonObject emptyData;
            newScene->onEnter(emptyData);
            m_currentScene = newScene;
            m_scenes[sceneName] = newScene;
        }

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

bool SceneManager::switchToScene(const QString &sceneName, TransitionType transitionType, const QJsonObject &sceneData)
{
    Q_UNUSED(transitionType)
    Q_UNUSED(sceneData)

    if (m_isTransitioning) {
        qWarning() << "SceneManager: 场景切换进行中，忽略切换请求:" << sceneName;
        return false;
    }

    loadScene(sceneName);
    return true;
}

bool SceneManager::resetToScene(const QString &sceneName, TransitionType transitionType, const QJsonObject &sceneData)
{
    Q_UNUSED(transitionType)
    Q_UNUSED(sceneData)

    // 清空堆栈
    m_sceneStack.clear();
    emit sceneStackChanged();

    loadScene(sceneName);
    return true;
}

void SceneManager::unloadScene(const QString &sceneName)
{
    qDebug() << "SceneManager: 卸载场景:" << sceneName;

    if (m_scenes.contains(sceneName)) {
        auto scene = m_scenes[sceneName];
        if (scene) {
            scene->unload();
        }
        m_scenes.remove(sceneName);
        emit sceneUnloaded(sceneName);
    }
}

bool SceneManager::isSceneLoaded(const QString &sceneName) const
{
    return m_scenes.contains(sceneName) || m_currentSceneName == sceneName;
}

QJsonObject SceneManager::getSceneInfo(const QString &sceneName) const
{
    QJsonObject info;
    info["name"] = sceneName;
    info["isLoaded"] = isSceneLoaded(sceneName);
    info["isCurrent"] = (m_currentSceneName == sceneName);
    return info;
}

void SceneManager::preloadSceneResources(const QString &sceneName)
{
    qDebug() << "SceneManager: 预加载场景资源:" << sceneName;
    auto scene = createScene(sceneName);
    if (scene) {
        scene->load(m_resourceManager);
        m_scenes[sceneName] = scene;
    }
}

void SceneManager::pauseCurrentScene()
{
    qDebug() << "SceneManager: 暂停当前场景:" << m_currentSceneName;
    if (m_currentScene) {
        m_currentScene->onExit();
    }
}

void SceneManager::resumeCurrentScene()
{
    qDebug() << "SceneManager: 恢复当前场景:" << m_currentSceneName;
    if (m_currentScene) {
        QJsonObject emptyData;
        m_currentScene->onEnter(emptyData);
    }
}

void SceneManager::onTransitionProgressUpdated(float progress)
{
    m_transitionProgress = progress;
    emit transitionProgressChanged();
}

void SceneManager::onTransitionCompleted()
{
    m_isTransitioning = false;
    m_transitionProgress = 1.0f;
    emit transitionStateChanged();
    emit transitionProgressChanged();
}

void SceneManager::onSceneResourcesLoaded(const QString &sceneName, bool success)
{
    emit sceneLoaded(sceneName, success);
}

std::shared_ptr<Scene> SceneManager::createScene(const QString &sceneName)
{
    if (sceneName == "MainMenu") {
        return std::make_shared<MainMenuScene>();
    } else if (sceneName == "Gameplay") {
        return std::make_shared<GameplayScene>();
    } else if (sceneName == "Battle") {
        return std::make_shared<BattleScene>();
    } else if (sceneName == "Loading") {
        return std::make_shared<LoadingScene>();
    }

    qWarning() << "SceneManager: 未知场景类型:" << sceneName;
    return nullptr;
}

void SceneManager::startTransition(std::shared_ptr<Scene> fromScene,
                                   std::shared_ptr<Scene> toScene,
                                   TransitionType transitionType)
{
    Q_UNUSED(fromScene)
    Q_UNUSED(toScene)
    Q_UNUSED(transitionType)

    m_isTransitioning = true;
    m_transitionProgress = 0.0f;
    emit transitionStateChanged();
}

void SceneManager::completeSceneSwitch(std::shared_ptr<Scene> newScene, const QJsonObject &sceneData)
{
    Q_UNUSED(sceneData)
    m_currentScene = newScene;
    m_isTransitioning = false;
    m_transitionProgress = 1.0f;
    emit transitionStateChanged();
}

void SceneManager::cleanupUnusedScenes()
{
    qDebug() << "SceneManager: 清理未使用的场景";

    QStringList toRemove;
    for (auto it = m_scenes.begin(); it != m_scenes.end(); ++it) {
        if (it.key() != m_currentSceneName && !m_sceneStack.contains(it.key())) {
            toRemove.append(it.key());
        }
    }

    for (const QString &name : toRemove) {
        auto scene = m_scenes[name];
        if (scene) {
            scene->unload();
        }
        m_scenes.remove(name);
        qDebug() << "SceneManager: 已清理场景:" << name;
    }
}

void SceneManager::cleanup()
{
    qDebug() << "SceneManager: 开始清理资源";

    // 卸载当前场景
    if (m_currentScene) {
        qDebug() << "SceneManager: 卸载当前场景";
        m_currentScene->onExit();
        m_currentScene->unload();
        m_currentScene = nullptr;
    }

    // 清理场景缓存
    for (auto it = m_scenes.begin(); it != m_scenes.end(); ++it) {
        if (it.value()) {
            it.value()->unload();
        }
    }
    m_scenes.clear();

    // 清理场景堆栈
    m_sceneStack.clear();

    // 重置状态
    m_isTransitioning = false;
    m_transitionProgress = 0.0f;
    m_currentSceneName.clear();

    qDebug() << "SceneManager: 资源清理完成";
}
