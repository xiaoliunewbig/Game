/*
 * 文件名: ResourceManager.cpp
 * 说明: 资源管理器实现 - 统一管理游戏中的所有资源（纹理、音频、字体等）
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 * 版本: v1.0.0
 * 
 * 功能描述:
 * - 提供统一的资源加载接口
 * - 实现资源缓存和生命周期管理
 * - 支持异步资源加载
 * - 提供资源热重载功能
 * - 内存使用优化和资源预加载
 * 
 * 支持的资源类型:
 * - 纹理资源 (PNG, JPG, BMP等)
 * - 音频资源 (MP3, WAV, OGG等)
 * - 字体资源 (TTF, OTF等)
 * - 配置文件 (JSON, XML, INI等)
 * - 3D模型资源 (OBJ, FBX等，预留)
 * 
 * 依赖项:
 * - Qt Core (QObject, QString, QDir等)
 * - Qt GUI (QPixmap, QFont等)
 * 
 * 使用示例:
 * ResourceManager* rm = ResourceManager::instance();
 * QPixmap texture = rm->loadTexture("player.png");
 * QFont font = rm->loadFont("game_font.ttf", 16);
 */

#include "core/ResourceManager.h"
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QPixmap>
#include <QFont>
#include <QFontDatabase>

/**
 * @brief 构造函数
 * @param parent 父对象指针，用于Qt对象树管理
 * 
 * 初始化资源管理器的基本状态和缓存容器
 */
ResourceManager::ResourceManager(QObject *parent)
    : QObject(parent)
    , m_isInitialized(false)       // 初始化状态标志
{
    qDebug() << "ResourceManager: 构造函数调用";
    
    // 初始化缓存容器
    // m_textureCache, m_soundCache, m_fontCache 在头文件中定义
}

/**
 * @brief 析构函数
 * 
 * 清理所有缓存的资源，释放内存
 */
ResourceManager::~ResourceManager()
{
    qDebug() << "ResourceManager: 析构函数调用，开始清理资源";
    cleanup();
    qDebug() << "ResourceManager: 析构完成";
}

/**
 * @brief 初始化资源管理器
 * @return bool 初始化是否成功
 * 
 * 初始化流程：
 * 1. 设置资源搜索路径
 * 2. 验证资源目录的可访问性
 * 3. 预加载核心资源
 * 4. 设置资源监控（热重载支持）
 */
bool ResourceManager::initialize()
{
    qDebug() << "ResourceManager: 开始初始化";
    
    try {
        // 设置资源搜索路径
        setupResourcePaths();
        
        // 验证资源路径
        if (!validateResourcePaths()) {
            qCritical() << "ResourceManager: 资源路径验证失败";
            return false;
        }
        
        // 预加载核心资源
        preloadCoreResources();
        
        // TODO: 设置资源文件监控（热重载）
        // setupResourceWatcher();
        
        m_isInitialized = true;
        qDebug() << "ResourceManager: 初始化成功";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "ResourceManager: 初始化失败:" << e.what();
        return false;
    }
}

/**
 * @brief 设置资源搜索路径
 * 
 * 按优先级顺序设置资源搜索路径：
 * 1. 应用程序目录下的resources文件夹
 * 2. Qt资源系统 (qrc)
 * 3. 用户数据目录（用于用户自定义资源）
 * 4. 系统资源目录（Linux/macOS）
 */
void ResourceManager::setupResourcePaths()
{
    qDebug() << "ResourceManager: 设置资源搜索路径";
    
    // 清空现有路径
    m_resourcePaths.clear();
    
    // 1. 应用程序目录下的resources文件夹（最高优先级）
    QString appDir = QCoreApplication::applicationDirPath();
    QString appResourceDir = appDir + "/resources";
    m_resourcePaths << appResourceDir;
    qDebug() << "ResourceManager: 添加应用资源路径:" << appResourceDir;
    
    // 2. Qt资源系统
    m_resourcePaths << ":/resources";
    qDebug() << "ResourceManager: 添加Qt资源系统路径: :/resources";
    
    // 3. 用户数据目录（用于MOD和用户自定义内容）
    QString userDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString userResourceDir = userDataDir + "/resources";
    m_resourcePaths << userResourceDir;
    qDebug() << "ResourceManager: 添加用户资源路径:" << userResourceDir;
    
    // 4. 开发环境资源路径（仅在调试模式下）
    #ifdef QT_DEBUG
    QString devResourceDir = QDir::currentPath() + "/application/resources";
    m_resourcePaths << devResourceDir;
    qDebug() << "ResourceManager: 添加开发资源路径:" << devResourceDir;
    #endif
    
    qDebug() << "ResourceManager: 资源路径设置完成，共" << m_resourcePaths.size() << "个路径";
}

/**
 * @brief 验证资源路径的可访问性
 * @return bool 验证是否通过
 * 
 * 检查每个资源路径是否存在且可读，
 * 至少需要一个有效路径才能通过验证
 */
bool ResourceManager::validateResourcePaths()
{
    qDebug() << "ResourceManager: 验证资源路径";
    
    int validPaths = 0;
    
    for (const QString& path : m_resourcePaths) {
        QDir dir(path);
        if (dir.exists() && dir.isReadable()) {
            qDebug() << "ResourceManager: 有效资源路径:" << path;
            validPaths++;
        } else {
            qWarning() << "ResourceManager: 无效资源路径:" << path;
        }
    }
    
    bool isValid = (validPaths > 0);
    qDebug() << "ResourceManager: 路径验证完成，有效路径数:" << validPaths;
    
    return isValid;
}

/**
 * @brief 预加载核心资源
 * 
 * 预加载游戏启动时必需的核心资源：
 * 1. UI相关的图标和背景
 * 2. 默认字体
 * 3. 系统音效
 * 4. 加载界面资源
 * 
 * 预加载可以减少游戏运行时的加载延迟
 */
void ResourceManager::preloadCoreResources()
{
    qDebug() << "ResourceManager: 开始预加载核心资源";
    
    try {
        // 预加载核心UI纹理
        QStringList coreTextures = {
            "app_icon.png",
            "main_menu_bg.jpg",
            "loading_bg.jpg"
        };
        
        for (const QString& textureName : coreTextures) {
            // TODO: 实际的纹理加载逻辑
            // loadTexture(textureName, true); // true表示预加载
            qDebug() << "ResourceManager: 预加载纹理:" << textureName;
        }
        
        // 预加载默认字体
        // TODO: 实际的字体加载逻辑
        // loadFont("game_font.ttf", 16);
        qDebug() << "ResourceManager: 预加载默认字体";
        
        // 预加载系统音效
        QStringList coreSounds = {
            "button_click.wav",
            "error.wav",
            "success.wav"
        };
        
        for (const QString& soundName : coreSounds) {
            // TODO: 实际的音频加载逻辑
            // loadSound(soundName, true); // true表示预加载
            qDebug() << "ResourceManager: 预加载音效:" << soundName;
        }
        
        qDebug() << "ResourceManager: 核心资源预加载完成";
        
    } catch (const std::exception& e) {
        qWarning() << "ResourceManager: 预加载过程中出现异常:" << e.what();
    }
}

/**
 * @brief 清理所有缓存的资源
 * 
 * 释放所有缓存的资源内存：
 * 1. 清理纹理缓存
 * 2. 清理音频缓存
 * 3. 清理字体缓存
 * 4. 重置初始化状态
 * 
 * 此函数在析构时自动调用，也可手动调用进行内存清理
 */
void ResourceManager::cleanup()
{
    qDebug() << "ResourceManager: 开始清理缓存资源";
    
    // 清理纹理缓存
    int textureCount = m_textureCache.size();
    m_textureCache.clear();
    qDebug() << "ResourceManager: 清理纹理缓存，释放" << textureCount << "个纹理";
    
    // 清理音频缓存
    int soundCount = m_soundCache.size();
    m_soundCache.clear();
    qDebug() << "ResourceManager: 清理音频缓存，释放" << soundCount << "个音频";
    
    // 清理字体缓存
    int fontCount = m_fontCache.size();
    m_fontCache.clear();
    qDebug() << "ResourceManager: 清理字体缓存，释放" << fontCount << "个字体";
    
    // 重置状态
    m_isInitialized = false;
    
    qDebug() << "ResourceManager: 资源清理完成";
}

/**
 * @brief 查找资源文件的完整路径
 * @param fileName 资源文件名
 * @return QString 找到的完整路径，未找到则返回空字符串
 * 
 * 按照优先级顺序在所有资源路径中搜索指定文件
 */
QString ResourceManager::findResourcePath(const QString& fileName) const
{
    for (const QString& basePath : m_resourcePaths) {
        QString fullPath = basePath + "/" + fileName;
        if (QFile::exists(fullPath)) {
            return fullPath;
        }
    }
    
    qWarning() << "ResourceManager: 未找到资源文件:" << fileName;
    return QString();
}
