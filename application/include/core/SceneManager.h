/*
 * 文件名: SceneManager.h
 * 说明: 场景管理器类，负责游戏场景的加载、切换和管理。
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 本类管理游戏中的所有场景，包括场景的加载、卸载、切换，
 * 场景对象的管理，以及场景间的数据传递。提供流畅的场景
 * 切换体验和高效的场景资源管理。
 */
#pragma once

#include <QObject>
#include <QJsonObject>
#include <QHash>
#include <QStack>
#include <memory>

// 前向声明
class Scene;
class ResourceManager;

/**
 * @brief 场景类型枚举
 * 
 * 定义不同类型的游戏场景。
 */
enum class SceneType {
    MainMenu,       ///< 主菜单场景
    Gameplay,       ///< 游戏进行场景
    Battle,         ///< 战斗场景
    Inventory,      ///< 背包界面场景
    Settings,       ///< 设置界面场景
    Loading,        ///< 加载场景
    Cutscene        ///< 过场动画场景
};

/**
 * @brief 场景切换类型枚举
 * 
 * 定义场景切换的方式和效果。
 */
enum class TransitionType {
    Instant,        ///< 立即切换
    Fade,           ///< 淡入淡出
    Slide,          ///< 滑动切换
    Zoom,           ///< 缩放切换
    Custom          ///< 自定义切换效果
};

/**
 * @brief 场景管理器类
 * 
 * 该类负责游戏场景系统的完整管理，包括：
 * - 场景的创建、加载和销毁
 * - 场景间的切换和转场效果
 * - 场景堆栈管理（支持场景嵌套）
 * - 场景数据的传递和共享
 * - 场景资源的预加载和缓存
 * 
 * 通过Q_PROPERTY暴露状态给QML，实现场景状态的UI显示。
 */
class SceneManager : public QObject
{
    Q_OBJECT
    
    // QML可访问的属性
    Q_PROPERTY(QString currentScene READ currentScene NOTIFY currentSceneChanged)
    Q_PROPERTY(bool isTransitioning READ isTransitioning NOTIFY transitionStateChanged)
    Q_PROPERTY(float transitionProgress READ transitionProgress NOTIFY transitionProgressChanged)
    Q_PROPERTY(int sceneStackDepth READ sceneStackDepth NOTIFY sceneStackChanged)

public:
    /**
     * @brief 构造函数
     * 
     * 初始化场景管理器，创建场景堆栈。
     * 
     * @param parent 父对象指针，用于Qt对象树管理
     */
    explicit SceneManager(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     * 
     * 清理所有场景，释放资源。
     */
    ~SceneManager();

    /**
     * @brief 初始化场景管理器
     * 
     * 初始化场景系统，加载场景配置：
     * - 读取场景配置文件
     * - 注册场景类型
     * - 设置资源管理器引用
     * - 预加载核心场景
     * 
     * @param resourceManager 资源管理器指针
     * @return bool 初始化是否成功
     * @retval true 初始化成功，场景系统可用
     * @retval false 初始化失败，场景功能不可用
     */
    bool initialize(ResourceManager *resourceManager);

    // 状态查询属性访问器
    
    /**
     * @brief 获取当前场景名称
     * 
     * @return QString 当前活跃场景的名称
     */
    QString currentScene() const { return m_currentSceneName; }
    
    /**
     * @brief 获取转场状态
     * 
     * @return bool 是否正在进行场景切换
     */
    bool isTransitioning() const { return m_isTransitioning; }
    
    /**
     * @brief 获取转场进度
     * 
     * @return float 场景切换进度（0.0-1.0）
     */
    float transitionProgress() const { return m_transitionProgress; }
    
    /**
     * @brief 获取场景堆栈深度
     * 
     * @return int 当前场景堆栈的深度
     */
    int sceneStackDepth() const { return m_sceneStack.size(); }

    // 场景操作接口
    
    /**
     * @brief 切换到指定场景
     * 
     * 切换到新场景，替换当前场景。
     * 
     * @param sceneName 目标场景名称
     * @param transitionType 切换效果类型
     * @param sceneData 传递给新场景的数据
     * @return bool 切换是否成功启动
     */
    bool switchToScene(const QString &sceneName, 
                      TransitionType transitionType = TransitionType::Fade,
                      const QJsonObject &sceneData = QJsonObject());
    
    /**
     * @brief 推入新场景
     * 
     * 将新场景推入堆栈，保留当前场景。
     * 
     * @param sceneName 新场景名称
     * @param transitionType 切换效果类型
     * @param sceneData 传递给新场景的数据
     * @return bool 推入是否成功启动
     */
    bool pushScene(const QString &sceneName,
                   TransitionType transitionType = TransitionType::Slide,
                   const QJsonObject &sceneData = QJsonObject());
    
    /**
     * @brief 弹出当前场景
     * 
     * 弹出当前场景，返回到上一个场景。
     * 
     * @param transitionType 切换效果类型
     * @param returnData 返回给上一场景的数据
     * @return bool 弹出是否成功启动
     */
    bool popScene(TransitionType transitionType = TransitionType::Slide,
                  const QJsonObject &returnData = QJsonObject());
    
    /**
     * @brief 清空场景堆栈并切换
     * 
     * 清空所有场景堆栈，切换到指定场景。
     * 
     * @param sceneName 目标场景名称
     * @param transitionType 切换效果类型
     * @param sceneData 传递给新场景的数据
     * @return bool 切换是否成功启动
     */
    bool resetToScene(const QString &sceneName,
                      TransitionType transitionType = TransitionType::Fade,
                      const QJsonObject &sceneData = QJsonObject());

    // QML可调用的场景操作接口
    
    /**
     * @brief 加载场景
     * 
     * 预加载指定场景，但不切换到该场景。
     * 
     * @param sceneName 场景名称
     */
    Q_INVOKABLE void loadScene(const QString &sceneName);
    
    /**
     * @brief 卸载场景
     * 
     * 卸载指定场景，释放其资源。
     * 
     * @param sceneName 场景名称
     */
    Q_INVOKABLE void unloadScene(const QString &sceneName);
    
    /**
     * @brief 检查场景是否已加载
     * 
     * @param sceneName 场景名称
     * @return bool 场景是否已加载
     */
    Q_INVOKABLE bool isSceneLoaded(const QString &sceneName) const;
    
    /**
     * @brief 获取场景信息
     * 
     * 获取指定场景的详细信息。
     * 
     * @param sceneName 场景名称
     * @return QJsonObject 场景信息的JSON表示
     */
    Q_INVOKABLE QJsonObject getSceneInfo(const QString &sceneName) const;
    
    /**
     * @brief 预加载场景资源
     * 
     * 预加载场景所需的资源。
     * 
     * @param sceneName 场景名称
     */
    Q_INVOKABLE void preloadSceneResources(const QString &sceneName);
    
    /**
     * @brief 暂停当前场景
     * 
     * 暂停当前场景的更新和渲染。
     */
    Q_INVOKABLE void pauseCurrentScene();
    
    /**
     * @brief 恢复当前场景
     * 
     * 恢复当前场景的更新和渲染。
     */
    Q_INVOKABLE void resumeCurrentScene();

signals:
    /**
     * @brief 当前场景改变信号
     */
    void currentSceneChanged();
    
    /**
     * @brief 转场状态改变信号
     */
    void transitionStateChanged();
    
    /**
     * @brief 转场进度改变信号
     */
    void transitionProgressChanged();
    
    /**
     * @brief 场景堆栈改变信号
     */
    void sceneStackChanged();
    
    /**
     * @brief 场景切换开始信号
     * 
     * @param fromScene 源场景名称
     * @param toScene 目标场景名称
     */
    void sceneTransitionStarted(const QString &fromScene, const QString &toScene);
    
    /**
     * @brief 场景切换完成信号
     * 
     * @param sceneName 新场景名称
     */
    void sceneTransitionCompleted(const QString &sceneName);
    
    /**
     * @brief 场景加载完成信号
     * 
     * @param sceneName 加载完成的场景名称
     * @param success 是否加载成功
     */
    void sceneLoaded(const QString &sceneName, bool success);
    
    /**
     * @brief 场景卸载信号
     * 
     * @param sceneName 卸载的场景名称
     */
    void sceneUnloaded(const QString &sceneName);

private slots:
    /**
     * @brief 转场动画更新处理
     * 
     * @param progress 转场进度（0.0-1.0）
     */
    void onTransitionProgressUpdated(float progress);
    
    /**
     * @brief 转场动画完成处理
     */
    void onTransitionCompleted();
    
    /**
     * @brief 场景资源加载完成处理
     * 
     * @param sceneName 场景名称
     * @param success 是否成功
     */
    void onSceneResourcesLoaded(const QString &sceneName, bool success);

private:
    /**
     * @brief 创建场景实例
     * 
     * 根据场景名称创建对应的场景对象。
     * 
     * @param sceneName 场景名称
     * @return std::shared_ptr<Scene> 场景对象指针
     */
    std::shared_ptr<Scene> createScene(const QString &sceneName);
    
    /**
     * @brief 开始转场动画
     * 
     * @param fromScene 源场景
     * @param toScene 目标场景
     * @param transitionType 转场类型
     */
    void startTransition(std::shared_ptr<Scene> fromScene,
                        std::shared_ptr<Scene> toScene,
                        TransitionType transitionType);
    
    /**
     * @brief 完成场景切换
     * 
     * @param newScene 新场景
     * @param sceneData 场景数据
     */
    void completeSceneSwitch(std::shared_ptr<Scene> newScene, const QJsonObject &sceneData);
    
    /**
     * @brief 清理未使用的场景
     */
    void cleanupUnusedScenes();

private:
    /**
     * @brief 场景缓存
     * 
     * 存储所有已加载的场景对象。
     */
    QHash<QString, std::shared_ptr<Scene>> m_scenes;
    
    /**
     * @brief 场景堆栈
     * 
     * 存储场景切换的历史堆栈。
     */
    QStack<std::shared_ptr<Scene>> m_sceneStack;
    
    /**
     * @brief 当前活跃场景
     * 
     * 当前正在显示和更新的场景。
     */
    std::shared_ptr<Scene> m_currentScene;
    
    /**
     * @brief 资源管理器引用
     * 
     * 用于加载场景资源的资源管理器。
     */
    ResourceManager *m_resourceManager;
    
    /**
     * @brief 当前场景名称
     * 
     * 当前活跃场景的名称。
     */
    QString m_currentSceneName;
    
    /**
     * @brief 转场状态
     * 
     * 是否正在进行场景切换。
     */
    bool m_isTransitioning;
    
    /**
     * @brief 转场进度
     * 
     * 当前场景切换的进度（0.0-1.0）。
     */
    float m_transitionProgress;
    
    /**
     * @brief 场景配置
     * 
     * 存储场景的配置信息。
     */
    QJsonObject m_sceneConfig;
};