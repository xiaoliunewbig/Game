/*
 * 文件名: ResourceManager.h
 * 说明: 资源管理器类，负责游戏资源的加载、缓存和管理。
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 *
 * 本类管理游戏中的所有资源，包括纹理、音频、配置文件、
 * 字体等。提供资源的异步加载、智能缓存、内存管理等功能，
 * 确保游戏资源的高效使用和管理。
 */
#pragma once

#include <QObject>
#include <QPixmap>
#include <QFont>
#include <QJsonObject>
#include <QJsonDocument>
#include <QHash>
#include <QMutex>
#include <QFuture>
#include <QFutureWatcher>
#include <memory>

/**
 * @brief 资源类型枚举
 * 
 * 定义不同类型的资源，用于分类管理和缓存策略。
 */
enum class ResourceType {
    Texture,        ///< 纹理图片
    Audio,          ///< 音频文件
    Font,           ///< 字体文件
    Config,         ///< 配置文件
    Data,           ///< 数据文件
    Shader,         ///< 着色器文件
    Animation       ///< 动画文件
};

/**
 * @brief 资源信息结构
 * 
 * 存储资源的元信息和状态。
 */
struct ResourceInfo {
    QString path;           ///< 资源文件路径
    ResourceType type;      ///< 资源类型
    qint64 size;           ///< 资源大小（字节）
    QDateTime loadTime;     ///< 加载时间
    int refCount;          ///< 引用计数
    bool isLoaded;         ///< 是否已加载
    QVariant data;         ///< 资源数据
};

/**
 * @brief 资源管理器类
 * 
 * 该类负责游戏资源的完整生命周期管理，包括：
 * - 资源的异步加载和预加载
 * - 智能缓存和内存管理
 * - 资源引用计数和自动释放
 * - 资源热重载和更新
 * - 资源压缩和解压缩
 * 
 * 通过Q_PROPERTY暴露状态给QML，实现资源加载进度的显示。
 */
class ResourceManager : public QObject
{
    Q_OBJECT
    
    // QML可访问的属性
    Q_PROPERTY(int loadedResourceCount READ loadedResourceCount NOTIFY resourceCountChanged)
    Q_PROPERTY(qint64 totalMemoryUsage READ totalMemoryUsage NOTIFY memoryUsageChanged)
    Q_PROPERTY(float loadingProgress READ loadingProgress NOTIFY loadingProgressChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY loadingStateChanged)

public:
    /**
     * @brief 构造函数
     * 
     * 初始化资源管理器，设置默认缓存策略。
     * 
     * @param parent 父对象指针，用于Qt对象树管理
     */
    explicit ResourceManager(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     * 
     * 清理所有资源，释放内存。
     */
    ~ResourceManager();

    /**
     * @brief 初始化资源管理器
     * 
     * 初始化资源系统，加载资源配置：
     * - 读取资源配置文件
     * - 设置缓存策略
     * - 创建资源加载线程池
     * - 预加载核心资源
     * 
     * @return bool 初始化是否成功
     * @retval true 初始化成功，资源系统可用
     * @retval false 初始化失败，资源功能不可用
     */
    bool initialize();
    
    /**
     * @brief 清理资源管理器
     * 
     * 清理所有资源，停止加载任务。
     */
    void cleanup();

    // 状态查询属性访问器
    
    /**
     * @brief 获取已加载资源数量
     * 
     * @return int 当前已加载的资源数量
     */
    int loadedResourceCount() const { return m_resources.size(); }
    
    /**
     * @brief 获取总内存使用量
     * 
     * @return qint64 资源占用的总内存（字节）
     */
    qint64 totalMemoryUsage() const { return m_totalMemoryUsage; }
    
    /**
     * @brief 获取加载进度
     * 
     * @return float 当前加载进度（0.0-1.0）
     */
    float loadingProgress() const { return m_loadingProgress; }
    
    /**
     * @brief 获取加载状态
     * 
     * @return bool 是否正在加载资源
     */
    bool isLoading() const { return m_isLoading; }

    // 资源加载接口
    
    /**
     * @brief 同步加载资源
     * 
     * 同步加载指定资源，阻塞直到加载完成。
     * 
     * @param resourcePath 资源文件路径
     * @param type 资源类型
     * @return QVariant 加载的资源数据，失败时返回无效QVariant
     */
    QVariant loadResource(const QString &resourcePath, ResourceType type);
    
    /**
     * @brief 异步加载资源
     * 
     * 异步加载指定资源，不阻塞调用线程。
     * 
     * @param resourcePath 资源文件路径
     * @param type 资源类型
     * @return QFuture<QVariant> 异步加载的Future对象
     */
    QFuture<QVariant> loadResourceAsync(const QString &resourcePath, ResourceType type);
    
    /**
     * @brief 预加载资源列表
     * 
     * 批量预加载资源列表，用于场景切换前的资源准备。
     * 
     * @param resourcePaths 资源路径列表
     */
    void preloadResources(const QStringList &resourcePaths);
    
    /**
     * @brief 卸载资源
     * 
     * 卸载指定资源，减少引用计数，必要时释放内存。
     * 
     * @param resourcePath 资源文件路径
     */
    void unloadResource(const QString &resourcePath);
    
    /**
     * @brief 获取资源
     * 
     * 获取已加载的资源数据，如果未加载则返回无效数据。
     * 
     * @param resourcePath 资源文件路径
     * @return QVariant 资源数据
     */
    QVariant getResource(const QString &resourcePath) const;
    
    /**
     * @brief 检查资源是否已加载
     * 
     * @param resourcePath 资源文件路径
     * @return bool 资源是否已加载
     */
    bool isResourceLoaded(const QString &resourcePath) const;

    // QML可调用的资源操作接口
    
    /**
     * @brief 加载纹理
     * 
     * 加载图片纹理资源。
     * 
     * @param imagePath 图片文件路径
     * @return QPixmap 加载的图片数据
     */
    Q_INVOKABLE QPixmap loadTexture(const QString &imagePath);
    
    /**
     * @brief 加载字体
     * 
     * 加载字体资源。
     * 
     * @param fontPath 字体文件路径
     * @param pointSize 字体大小
     * @return QFont 加载的字体对象
     */
    Q_INVOKABLE QFont loadFont(const QString &fontPath, int pointSize = 12);
    
    /**
     * @brief 加载配置文件
     * 
     * 加载JSON配置文件。
     * 
     * @param configPath 配置文件路径
     * @return QJsonObject 配置数据
     */
    Q_INVOKABLE QJsonObject loadConfig(const QString &configPath);
    
    /**
     * @brief 保存配置文件
     * 
     * 保存配置数据到文件。
     * 
     * @param configPath 配置文件路径
     * @param config 配置数据
     * @return bool 保存是否成功
     */
    Q_INVOKABLE bool saveConfig(const QString &configPath, const QJsonObject &config);
    
    /**
     * @brief 清理未使用的资源
     * 
     * 清理引用计数为0的资源，释放内存。
     */
    Q_INVOKABLE void cleanupUnusedResources();
    
    /**
     * @brief 获取资源信息
     * 
     * 获取指定资源的详细信息。
     * 
     * @param resourcePath 资源文件路径
     * @return QJsonObject 资源信息的JSON表示
     */
    Q_INVOKABLE QJsonObject getResourceInfo(const QString &resourcePath) const;

signals:
    /**
     * @brief 资源数量改变信号
     */
    void resourceCountChanged();
    
    /**
     * @brief 内存使用量改变信号
     */
    void memoryUsageChanged();
    
    /**
     * @brief 加载进度改变信号
     */
    void loadingProgressChanged();
    
    /**
     * @brief 加载状态改变信号
     */
    void loadingStateChanged();
    
    /**
     * @brief 资源加载完成信号
     * 
     * @param resourcePath 加载完成的资源路径
     * @param success 是否加载成功
     */
    void resourceLoaded(const QString &resourcePath, bool success);
    
    /**
     * @brief 资源卸载信号
     * 
     * @param resourcePath 卸载的资源路径
     */
    void resourceUnloaded(const QString &resourcePath);
    
    /**
     * @brief 内存不足警告信号
     * 
     * @param currentUsage 当前内存使用量
     * @param maxUsage 最大内存限制
     */
    void memoryWarning(qint64 currentUsage, qint64 maxUsage);

private slots:
    /**
     * @brief 异步加载完成处理
     */
    void onAsyncLoadFinished();
    
    /**
     * @brief 预加载完成处理
     */
    void onPreloadFinished();

private:
    /**
     * @brief 加载纹理资源
     * 
     * @param path 纹理文件路径
     * @return QVariant 纹理数据
     */
    QVariant loadTextureResource(const QString &path);
    
    /**
     * @brief 加载音频资源
     * 
     * @param path 音频文件路径
     * @return QVariant 音频数据
     */
    QVariant loadAudioResource(const QString &path);
    
    /**
     * @brief 加载字体资源
     * 
     * @param path 字体文件路径
     * @return QVariant 字体数据
     */
    QVariant loadFontResource(const QString &path);
    
    /**
     * @brief 加载配置资源
     * 
     * @param path 配置文件路径
     * @return QVariant 配置数据
     */
    QVariant loadConfigResource(const QString &path);
    
    /**
     * @brief 更新内存使用量
     */
    void updateMemoryUsage();
    
    /**
     * @brief 检查内存限制
     */
    void checkMemoryLimit();

        /**
     * @brief 获取已加载资源数量
     * @return 已加载资源数量
     */
    int getLoadedResourceCount() const;
    
    /**
     * @brief 获取总资源大小
     * @return 总资源大小（字节）
     */
    qint64 getTotalResourceSize() const;
    
    /**
     * @brief 设置最大缓存大小
     * @param maxSize 最大缓存大小（字节）
     */
    void setMaxCacheSize(qint64 maxSize);
    
    /**
     * @brief 获取最大缓存大小
     * @return 最大缓存大小（字节）
     */
    qint64 getMaxCacheSize() const;

private:
    /**
     * @brief 资源缓存
     * 
     * 存储所有已加载的资源信息和数据。
     */
    QHash<QString, std::shared_ptr<ResourceInfo>> m_resources;
    
    /**
     * @brief 异步加载监视器
     * 
     * 监视异步加载任务的完成状态。
     */
    QHash<QString, std::unique_ptr<QFutureWatcher<QVariant>>> m_loadWatchers;
    
    /**
     * @brief 资源访问互斥锁
     * 
     * 保护资源缓存的线程安全访问。
     */
    mutable QMutex m_resourceMutex;
    
    /**
     * @brief 总内存使用量
     * 
     * 当前资源占用的总内存大小。
     */
    qint64 m_totalMemoryUsage;
    
    /**
     * @brief 最大内存限制
     * 
     * 资源缓存的最大内存限制。
     */
    qint64 m_maxMemoryUsage;
    
    /**
     * @brief 加载进度
     * 
     * 当前资源加载的进度（0.0-1.0）。
     */
    float m_loadingProgress;
    
    /**
     * @brief 加载状态
     * 
     * 是否正在进行资源加载。
     */
    bool m_isLoading;
    
    /**
     * @brief 待加载资源数量
     * 
     * 当前待加载的资源数量。
     */
    int m_pendingLoads;
    
    /**
     * @brief 已完成加载数量
     * 
     * 已完成加载的资源数量。
     */
    int m_completedLoads;
    
    // 资源缓存
    QMap<QString, QPixmap> m_textureCache;      ///< 纹理缓存
    QMap<QString, QFont> m_fontCache;           ///< 字体缓存
    
    // 资源路径
    QStringList m_resourcePaths;                ///< 资源搜索路径列表
    
    // 统计信息
    int m_loadedResourceCount;                  ///< 已加载资源数量
    qint64 m_totalResourceSize;                 ///< 总资源大小
    qint64 m_maxCacheSize;                      ///< 最大缓存大小
    
    // 私有方法声明
    void setupResourcePaths();
    bool validateResourcePaths();
    void preloadCoreResources();
    QString findResourcePath(const QString& fileName) const;
};
