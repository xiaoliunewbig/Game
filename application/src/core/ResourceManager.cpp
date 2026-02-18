/*
 * 文件名: ResourceManager.cpp
 * 说明: 资源管理器实现文件
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 * 版本: v1.0.0
 */

#include "core/ResourceManager.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QFileInfo>
#include <QPixmap>
#include <QFont>
#include <QFontDatabase>

ResourceManager::ResourceManager(QObject *parent)
    : QObject(parent)
    , m_loadedResourceCount(0)
    , m_totalResourceSize(0)
    , m_maxCacheSize(100 * 1024 * 1024) // 100MB默认缓存大小
{
    qDebug() << "ResourceManager: 构造函数调用";
    
    // 设置默认资源路径
    m_resourcePaths << ":/resources"
                   << QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/resources"
                   << QCoreApplication::applicationDirPath() + "/resources";
}

ResourceManager::~ResourceManager()
{
    cleanup();
    qDebug() << "ResourceManager: 析构完成";
}

bool ResourceManager::initialize()
{
    qDebug() << "ResourceManager: 开始初始化";
    
    try {
        // 设置资源路径
        setupResourcePaths();
        
        // 验证资源路径
        if (!validateResourcePaths()) {
            qWarning() << "ResourceManager: 资源路径验证失败";
            return false;
        }
        
        // 预加载核心资源
        preloadCoreResources();
        
        qDebug() << "ResourceManager: 初始化成功";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "ResourceManager: 初始化异常:" << e.what();
        return false;
    }
}

void ResourceManager::setupResourcePaths()
{
    qDebug() << "ResourceManager: 设置资源路径";
    
    // 添加应用程序目录下的资源路径
    QString appDir = QCoreApplication::applicationDirPath();
    m_resourcePaths << appDir + "/assets"
                   << appDir + "/data"
                   << appDir + "/textures"
                   << appDir + "/sounds"
                   << appDir + "/fonts";
    
    // 添加用户数据目录
    QString userDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_resourcePaths << userDataDir + "/mods"
                   << userDataDir + "/custom";
    
    // 创建必要的目录
    for (const QString& path : m_resourcePaths) {
        QDir().mkpath(path);
    }
    
    qDebug() << "ResourceManager: 资源路径设置完成，共" << m_resourcePaths.size() << "个路径";
}

bool ResourceManager::validateResourcePaths()
{
    qDebug() << "ResourceManager: 验证资源路径";
    
    int validPaths = 0;
    for (const QString& path : m_resourcePaths) {
        QDir dir(path);
        if (dir.exists()) {
            validPaths++;
            qDebug() << "ResourceManager: 有效路径:" << path;
        } else {
            qWarning() << "ResourceManager: 无效路径:" << path;
        }
    }
    
    if (validPaths == 0) {
        qCritical() << "ResourceManager: 没有有效的资源路径";
        return false;
    }
    
    qDebug() << "ResourceManager: 路径验证完成，有效路径:" << validPaths;
    return true;
}

void ResourceManager::preloadCoreResources()
{
    qDebug() << "ResourceManager: 预加载核心资源";
    
    // 预加载常用纹理
    QStringList coreTextures = {
        "images/ui/button_normal.png",
        "images/ui/button_hover.png",
        "images/ui/button_pressed.png",
        "images/ui/background.png",
        "icons/inventory.png",
        "icons/settings.png"
    };
    
    for (const QString& texture : coreTextures) {
        loadTexture(texture);
    }
    
    // 预加载常用字体
    QStringList coreFonts = {
        "fonts/game_font.ttf",
        "fonts/ui_font.ttf"
    };
    
    for (const QString& font : coreFonts) {
        loadFont(font);
    }
    
    qDebug() << "ResourceManager: 核心资源预加载完成";
}

QString ResourceManager::findResourcePath(const QString& fileName) const
{
    for (const QString& basePath : m_resourcePaths) {
        QString fullPath = basePath + "/" + fileName;
        if (QFile::exists(fullPath)) {
            return fullPath;
        }
    }
    
    // 如果在文件系统中找不到，尝试Qt资源系统
    QString resourcePath = ":/" + fileName;
    if (QFile::exists(resourcePath)) {
        return resourcePath;
    }
    
    return QString(); // 未找到
}

QPixmap ResourceManager::loadTexture(const QString &fileName)
{
    // 检查缓存
    if (m_textureCache.contains(fileName)) {
        return m_textureCache[fileName];
    }
    
    // 查找文件路径
    QString fullPath = findResourcePath(fileName);
    if (fullPath.isEmpty()) {
        qWarning() << "ResourceManager: 纹理文件未找到:" << fileName;
        return QPixmap();
    }
    
    // 加载纹理
    QPixmap pixmap(fullPath);
    if (pixmap.isNull()) {
        qWarning() << "ResourceManager: 纹理加载失败:" << fullPath;
        return QPixmap();
    }
    
    // 添加到缓存
    m_textureCache[fileName] = pixmap;
    m_loadedResourceCount++;
    
    qDebug() << "ResourceManager: 纹理加载成功:" << fileName;
    return pixmap;
}

QFont ResourceManager::loadFont(const QString &fileName, int pointSize)
{
    QString cacheKey = QString("%1_%2").arg(fileName).arg(pointSize);
    
    // 检查缓存
    if (m_fontCache.contains(cacheKey)) {
        return m_fontCache[cacheKey];
    }
    
    // 查找文件路径
    QString fullPath = findResourcePath(fileName);
    if (fullPath.isEmpty()) {
        qWarning() << "ResourceManager: 字体文件未找到:" << fileName;
        return QFont();
    }
    
    // 加载字体
    int fontId = QFontDatabase::addApplicationFont(fullPath);
    if (fontId == -1) {
        qDebug() << "ResourceManager: 字体加载失败（使用系统默认字体）:" << fullPath;
        return QFont();
    }
    
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (fontFamilies.isEmpty()) {
        qDebug() << "ResourceManager: 字体族获取失败（使用系统默认字体）:" << fullPath;
        return QFont();
    }
    
    QFont font(fontFamilies.first(), pointSize);
    
    // 添加到缓存
    m_fontCache[cacheKey] = font;
    m_loadedResourceCount++;
    
    qDebug() << "ResourceManager: 字体加载成功:" << fileName;
    return font;
}

void ResourceManager::preloadResources(const QStringList &resourceList)
{
    qDebug() << "ResourceManager: 预加载资源列表，共" << resourceList.size() << "个资源";
    
    for (const QString& resource : resourceList) {
        if (resource.endsWith(".png") || resource.endsWith(".jpg") || 
            resource.endsWith(".jpeg") || resource.endsWith(".bmp")) {
            loadTexture(resource);
        } else if (resource.endsWith(".ttf") || resource.endsWith(".otf")) {
            loadFont(resource);
        } else if (resource.endsWith(".wav") || resource.endsWith(".mp3") || 
                  resource.endsWith(".ogg")) {
            // TODO: 实现音频加载
            qDebug() << "ResourceManager: 音频加载暂未实现:" << resource;
        }
    }
    
    qDebug() << "ResourceManager: 资源预加载完成";
}

void ResourceManager::cleanup()
{
    qDebug() << "ResourceManager: 开始清理资源";
    
    int textureCount = m_textureCache.size();
    int soundCount = 0; // m_soundCache.size();
    int fontCount = m_fontCache.size();
    
    m_textureCache.clear();
    // m_soundCache.clear();
    m_fontCache.clear();
    
    m_loadedResourceCount = 0;
    m_totalResourceSize = 0;
    
    qDebug() << "ResourceManager: 资源清理完成 - 纹理:" << textureCount 
             << "音频:" << soundCount << "字体:" << fontCount;
}

bool ResourceManager::isResourceLoaded(const QString &fileName) const
{
    return m_textureCache.contains(fileName) || 
           m_fontCache.contains(fileName);
}

int ResourceManager::getLoadedResourceCount() const
{
    return m_loadedResourceCount;
}

qint64 ResourceManager::getTotalResourceSize() const
{
    return m_totalResourceSize;
}

void ResourceManager::setMaxCacheSize(qint64 maxSize)
{
    m_maxCacheSize = maxSize;
    qDebug() << "ResourceManager: 设置最大缓存大小:" << maxSize << "字节";
}

qint64 ResourceManager::getMaxCacheSize() const
{
    return m_maxCacheSize;
}

QVariant ResourceManager::loadResource(const QString &resourcePath, ResourceType type)
{
    // 检查缓存
    if (m_resources.contains(resourcePath)) {
        auto &info = m_resources[resourcePath];
        info->refCount++;
        return info->data;
    }

    QVariant data;
    switch (type) {
    case ResourceType::Texture:
        data = loadTextureResource(resourcePath);
        break;
    case ResourceType::Audio:
        data = loadAudioResource(resourcePath);
        break;
    case ResourceType::Font:
        data = loadFontResource(resourcePath);
        break;
    case ResourceType::Config:
        data = loadConfigResource(resourcePath);
        break;
    default:
        qWarning() << "ResourceManager: 不支持的资源类型:" << static_cast<int>(type);
        return QVariant();
    }

    if (data.isValid()) {
        auto info = std::make_shared<ResourceInfo>();
        info->path = resourcePath;
        info->type = type;
        info->size = 0;
        info->loadTime = QDateTime::currentDateTime();
        info->refCount = 1;
        info->isLoaded = true;
        info->data = data;
        m_resources[resourcePath] = info;
        emit resourceLoaded(resourcePath, true);
        emit resourceCountChanged();
    }

    return data;
}

QVariant ResourceManager::loadTextureResource(const QString &path)
{
    QString fullPath = findResourcePath(path);
    if (fullPath.isEmpty()) {
        return QVariant();
    }
    QPixmap pixmap(fullPath);
    return pixmap.isNull() ? QVariant() : QVariant::fromValue(pixmap);
}

QVariant ResourceManager::loadAudioResource(const QString &path)
{
    QString fullPath = findResourcePath(path);
    if (fullPath.isEmpty()) {
        qWarning() << "ResourceManager: 音频资源未找到:" << path;
        return QVariant();
    }

    // 验证文件可读
    QFile file(fullPath);
    if (!file.exists()) {
        qWarning() << "ResourceManager: 音频文件不存在:" << fullPath;
        return QVariant();
    }

    // 音频资源存储路径，由 AudioManager 实际加载播放
    qDebug() << "ResourceManager: 音频资源已定位:" << fullPath;
    return QVariant(fullPath);
}

QVariant ResourceManager::loadFontResource(const QString &path)
{
    QString fullPath = findResourcePath(path);
    if (fullPath.isEmpty()) {
        return QVariant();
    }
    int fontId = QFontDatabase::addApplicationFont(fullPath);
    if (fontId == -1) {
        return QVariant();
    }
    QStringList families = QFontDatabase::applicationFontFamilies(fontId);
    return families.isEmpty() ? QVariant() : QVariant::fromValue(QFont(families.first()));
}

QVariant ResourceManager::loadConfigResource(const QString &path)
{
    QString fullPath = findResourcePath(path);
    if (fullPath.isEmpty()) {
        return QVariant();
    }
    QFile file(fullPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QVariant();
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    return doc.isNull() ? QVariant() : QVariant::fromValue(doc.object());
}

QFuture<QVariant> ResourceManager::loadResourceAsync(const QString &resourcePath, ResourceType type)
{
    Q_UNUSED(resourcePath)
    Q_UNUSED(type)
    // 异步加载预留接口，当前返回空 future
    return QFuture<QVariant>();
}

void ResourceManager::unloadResource(const QString &resourcePath)
{
    auto it = m_resources.find(resourcePath);
    if (it != m_resources.end()) {
        (*it)->refCount--;
        if ((*it)->refCount <= 0) {
            m_resources.erase(it);
            emit resourceUnloaded(resourcePath);
            emit resourceCountChanged();
        }
    }
}

QVariant ResourceManager::getResource(const QString &resourcePath) const
{
    auto it = m_resources.find(resourcePath);
    if (it != m_resources.end()) {
        return (*it)->data;
    }
    return QVariant();
}

QJsonObject ResourceManager::loadConfig(const QString &configPath)
{
    QString fullPath = findResourcePath(configPath);
    if (fullPath.isEmpty()) {
        qWarning() << "ResourceManager: 配置文件未找到:" << configPath;
        return QJsonObject();
    }

    QFile file(fullPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "ResourceManager: 配置文件打开失败:" << fullPath;
        return QJsonObject();
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "ResourceManager: 配置文件格式错误:" << fullPath;
        return QJsonObject();
    }

    return doc.object();
}

bool ResourceManager::saveConfig(const QString &configPath, const QJsonObject &config)
{
    QFile file(configPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "ResourceManager: 配置文件写入失败:" << configPath;
        return false;
    }

    QJsonDocument doc(config);
    file.write(doc.toJson(QJsonDocument::Indented));
    return true;
}

void ResourceManager::cleanupUnusedResources()
{
    qDebug() << "ResourceManager: 清理未使用的资源";

    QStringList toRemove;
    for (auto it = m_resources.begin(); it != m_resources.end(); ++it) {
        if ((*it)->refCount <= 0) {
            toRemove.append(it.key());
        }
    }

    for (const QString &key : toRemove) {
        m_resources.remove(key);
    }

    if (!toRemove.isEmpty()) {
        emit resourceCountChanged();
        qDebug() << "ResourceManager: 清理了" << toRemove.size() << "个未使用的资源";
    }
}

QJsonObject ResourceManager::getResourceInfo(const QString &resourcePath) const
{
    QJsonObject info;
    auto it = m_resources.find(resourcePath);
    if (it != m_resources.end()) {
        info["path"] = (*it)->path;
        info["type"] = static_cast<int>((*it)->type);
        info["size"] = (*it)->size;
        info["refCount"] = (*it)->refCount;
        info["isLoaded"] = (*it)->isLoaded;
    }
    return info;
}

void ResourceManager::onAsyncLoadFinished()
{
    // 异步加载完成回调
    qDebug() << "ResourceManager: 异步加载完成";
}

void ResourceManager::onPreloadFinished()
{
    m_isLoading = false;
    m_loadingProgress = 1.0f;
    emit loadingStateChanged();
    emit loadingProgressChanged();
    qDebug() << "ResourceManager: 预加载完成";
}

void ResourceManager::updateMemoryUsage()
{
    m_totalMemoryUsage = m_totalResourceSize;
    emit memoryUsageChanged();
}

void ResourceManager::checkMemoryLimit()
{
    if (m_totalMemoryUsage > m_maxMemoryUsage) {
        emit memoryWarning(m_totalMemoryUsage, m_maxMemoryUsage);
        cleanupUnusedResources();
    }
}
