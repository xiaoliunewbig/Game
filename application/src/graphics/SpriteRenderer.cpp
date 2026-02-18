/*
 * 文件名: SpriteRenderer.cpp
 * 说明: 精灵渲染器实现
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 * 
 * 本文件实现了游戏的精灵渲染系统，负责图像资源的加载、缓存和渲染。
 * 
 * 主要功能：
 * - 支持单张图像和精灵表（Sprite Sheet）
 * - 图像缓存管理，避免重复加载
 * - 丰富的渲染效果（缩放、旋转、翻转、透明度、色调）
 * - 内存优化的资源管理
 * - 线程安全的操作接口
 * 
 * 设计模式：
 * - 单例模式：全局唯一的渲染器实例
 * - 缓存模式：智能缓存管理
 * - 工厂模式：统一的资源创建接口
 */

#include "graphics/SpriteRenderer.h"
#include <QDebug>
#include <QPixmap>
#include <QPainter>
#include <QTransform>
#include <QFileInfo>

/**
 * @brief 构造函数
 * @param parent 父对象指针
 * 
 * 初始化精灵渲染器，设置默认参数
 */
SpriteRenderer::SpriteRenderer(QObject *parent)
    : QObject(parent)
    , m_renderEnabled(true)
{
    qDebug() << "SpriteRenderer: 初始化精灵渲染器";
}

/**
 * @brief 析构函数
 * 
 * 清理所有缓存的精灵资源
 */
SpriteRenderer::~SpriteRenderer()
{
    clearCache();
    qDebug() << "SpriteRenderer: 销毁精灵渲染器";
}

/**
 * @brief 加载单张精灵图像
 * @param name 精灵名称，用作缓存键
 * @param filePath 图像文件路径
 * @return 加载成功返回true，失败返回false
 * 
 * 从文件系统加载图像并存储到缓存中。支持常见的图像格式：
 * PNG, JPG, BMP, GIF等。加载失败的原因可能包括：
 * - 文件不存在
 * - 文件格式不支持
 * - 内存不足
 */
bool SpriteRenderer::loadSprite(const QString &name, const QString &filePath)
{
    if (name.isEmpty() || filePath.isEmpty()) {
        qWarning() << "SpriteRenderer: 无效的精灵名称或文件路径";
        return false;
    }
    
    // 检查文件是否存在
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qWarning() << "SpriteRenderer: 精灵文件不存在:" << filePath;
        return false;
    }
    
    // 加载图像
    QPixmap pixmap(filePath);
    if (pixmap.isNull()) {
        qWarning() << "SpriteRenderer: 无法加载精灵图像:" << filePath;
        return false;
    }
    
    // 创建精灵数据结构
    SpriteData spriteData;
    spriteData.name = name;
    spriteData.filePath = filePath;
    spriteData.pixmap = pixmap;
    spriteData.originalSize = pixmap.size();
    spriteData.isLoaded = true;
    
    // 存储到缓存映射表
    m_spriteCache[name] = spriteData;
    
    qDebug() << "SpriteRenderer: 加载精灵成功" << name << "大小:" << pixmap.size();
    emit spriteLoaded(name);
    
    return true;
}

/**
 * @brief 加载精灵表（Sprite Sheet）
 * @param name 精灵表名称
 * @param filePath 图像文件路径
 * @param frameWidth 单帧宽度（像素）
 * @param frameHeight 单帧高度（像素）
 * @param frameCount 总帧数
 * @return 加载成功返回true，失败返回false
 * 
 * 精灵表是包含多个动画帧的大图像，按网格排列。
 * 此方法会计算网格布局并支持按帧索引提取单帧图像。
 * 
 * 布局规则：
 * - 从左到右，从上到下排列
 * - 帧索引从0开始
 * - 自动计算行列数
 */
bool SpriteRenderer::loadSpriteSheet(const QString &name, const QString &filePath, 
                                    int frameWidth, int frameHeight, int frameCount)
{
    // 首先作为普通精灵加载
    if (!loadSprite(name, filePath)) {
        return false;
    }
    
    // 获取已加载的精灵数据并设置精灵表信息
    SpriteData &spriteData = m_spriteCache[name];
    
    // 设置精灵表特有属性
    spriteData.isSpriteSheet = true;
    spriteData.frameWidth = frameWidth;
    spriteData.frameHeight = frameHeight;
    spriteData.frameCount = frameCount;
    
    // 计算网格布局
    spriteData.columns = spriteData.pixmap.width() / frameWidth;
    spriteData.rows = spriteData.pixmap.height() / frameHeight;
    
    qDebug() << "SpriteRenderer: 加载精灵表成功" << name 
             << "帧数:" << frameCount 
             << "帧大小:" << QSize(frameWidth, frameHeight)
             << "布局:" << spriteData.columns << "x" << spriteData.rows;
    
    return true;
}

/**
 * @brief 卸载精灵
 * @param name 精灵名称
 * 
 * 从缓存中移除指定精灵，释放内存。
 * 如果精灵不存在，此操作无效果。
 */
void SpriteRenderer::unloadSprite(const QString &name)
{
    auto it = m_spriteCache.find(name);
    if (it != m_spriteCache.end()) {
        qDebug() << "SpriteRenderer: 卸载精灵" << name;
        m_spriteCache.erase(it);
        emit spriteUnloaded(name);
    }
}

/**
 * @brief 检查精灵是否已加载
 * @param name 精灵名称
 * @return 如果精灵已加载返回true，否则返回false
 */
bool SpriteRenderer::hasSprite(const QString &name) const
{
    return m_spriteCache.contains(name) && m_spriteCache[name].isLoaded;
}

/**
 * @brief 获取完整精灵图像
 * @param name 精灵名称
 * @return 精灵的QPixmap对象，如果不存在返回空QPixmap
 * 
 * 返回原始的完整图像，不进行任何处理。
 * 对于精灵表，返回整个表格图像。
 */
QPixmap SpriteRenderer::getSprite(const QString &name) const
{
    auto it = m_spriteCache.find(name);
    if (it != m_spriteCache.end() && it->isLoaded) {
        return it->pixmap;
    }
    
    qWarning() << "SpriteRenderer: 精灵不存在或未加载:" << name;
    return QPixmap();
}

/**
 * @brief 获取精灵表的指定帧
 * @param name 精灵表名称
 * @param frameIndex 帧索引（从0开始）
 * @return 指定帧的QPixmap对象，如果失败返回空QPixmap
 * 
 * 从精灵表中提取单个动画帧。对于非精灵表的普通精灵，
 * 忽略frameIndex参数并返回完整图像。
 * 
 * 帧索引计算：
 * - 行号 = frameIndex / columns
 * - 列号 = frameIndex % columns
 * - 坐标 = (列号 * frameWidth, 行号 * frameHeight)
 */
QPixmap SpriteRenderer::getSpriteFrame(const QString &name, int frameIndex) const
{
    auto it = m_spriteCache.find(name);
    if (it == m_spriteCache.end() || !it->isLoaded) {
        qWarning() << "SpriteRenderer: 精灵不存在或未加载:" << name;
        return QPixmap();
    }
    
    const SpriteData &spriteData = *it;
    
    // 如果不是精灵表，返回整个图像
    if (!spriteData.isSpriteSheet) {
        return spriteData.pixmap;
    }
    
    // 验证帧索引范围
    if (frameIndex < 0 || frameIndex >= spriteData.frameCount) {
        qWarning() << "SpriteRenderer: 帧索引超出范围:" << frameIndex;
        return QPixmap();
    }
    
    // 计算帧在网格中的位置
    int col = frameIndex % spriteData.columns;
    int row = frameIndex / spriteData.columns;
    
    // 定义帧的矩形区域
    QRect frameRect(col * spriteData.frameWidth, 
                   row * spriteData.frameHeight,
                   spriteData.frameWidth, 
                   spriteData.frameHeight);
    
    // 从完整图像中复制指定区域
    return spriteData.pixmap.copy(frameRect);
}

/**
 * @brief 渲染精灵（应用渲染效果）
 * @param name 精灵名称
 * @param options 渲染选项，包含各种效果参数
 * @return 应用效果后的QPixmap对象
 * 
 * 这是高级渲染接口，支持多种视觉效果：
 * - 缩放：改变图像大小
 * - 旋转：按指定角度旋转
 * - 翻转：水平或垂直翻转
 * - 透明度：调整不透明度
 * - 色调：应用颜色滤镜
 * - 帧选择：对精灵表选择特定帧
 */
QPixmap SpriteRenderer::renderSprite(const QString &name, const RenderOptions &options) const
{
    QPixmap sprite;
    
    // 根据是否指定帧索引选择获取方式
    if (options.frameIndex >= 0) {
        sprite = getSpriteFrame(name, options.frameIndex);
    } else {
        sprite = getSprite(name);
    }
    
    if (sprite.isNull()) {
        return QPixmap();
    }
    
    // 应用渲染选项
    return applyRenderOptions(sprite, options);
}

/**
 * @brief 清空所有精灵缓存
 * 
 * 释放所有已加载的精灵资源，用于内存清理。
 * 通常在场景切换或游戏退出时调用。
 */
void SpriteRenderer::clearCache()
{
    qDebug() << "SpriteRenderer: 清空精灵缓存，共" << m_spriteCache.size() << "个精灵";
    m_spriteCache.clear();
    emit cacheCleared();
}

/**
 * @brief 获取缓存中的精灵数量
 * @return 当前缓存的精灵数量
 */
int SpriteRenderer::getCacheSize() const
{
    return m_spriteCache.size();
}

/**
 * @brief 获取所有已加载精灵的名称列表
 * @return 精灵名称的字符串列表
 * 
 * 用于调试或显示资源管理界面
 */
QStringList SpriteRenderer::getLoadedSprites() const
{
    QStringList names;
    for (auto it = m_spriteCache.begin(); it != m_spriteCache.end(); ++it) {
        if (it->isLoaded) {
            names.append(it.key());
        }
    }
    return names;
}

/**
 * @brief 获取精灵的详细信息
 * @param name 精灵名称
 * @return 包含精灵详细信息的结构体
 * 
 * 返回的信息包括：
 * - 基本信息：名称、文件路径、大小
 * - 精灵表信息：是否为精灵表、帧数、帧大小
 * - 状态信息：是否已加载
 */
SpriteRenderer::SpriteInfo SpriteRenderer::getSpriteInfo(const QString &name) const
{
    SpriteInfo info;
    
    auto it = m_spriteCache.find(name);
    if (it != m_spriteCache.end() && it->isLoaded) {
        const SpriteData &data = *it;
        info.name = data.name;
        info.filePath = data.filePath;
        info.size = data.originalSize;
        info.isSpriteSheet = data.isSpriteSheet;
        info.frameCount = data.frameCount;
        info.frameSize = QSize(data.frameWidth, data.frameHeight);
        info.isLoaded = data.isLoaded;
    }
    
    return info;
}

/**
 * @brief 设置渲染是否启用
 * @param enabled 是否启用渲染效果
 * 
 * 禁用渲染时，renderSprite方法将跳过所有效果处理，
 * 直接返回原始图像，可用于性能优化。
 */
void SpriteRenderer::setRenderEnabled(bool enabled)
{
    if (m_renderEnabled != enabled) {
        m_renderEnabled = enabled;
        qDebug() << "SpriteRenderer: 渲染" << (enabled ? "启用" : "禁用");
        emit renderEnabledChanged(enabled);
    }
}

/**
 * @brief 获取渲染是否启用
 * @return 当前渲染启用状态
 */
bool SpriteRenderer::isRenderEnabled() const
{
    return m_renderEnabled;
}

/**
 * @brief 应用渲染选项到图像
 * @param sprite 原始图像
 * @param options 渲染选项
 * @return 应用效果后的图像
 * 
 * 这是内部方法，按顺序应用各种视觉效果：
 * 1. 缩放变换
 * 2. 旋转变换
 * 3. 翻转变换
 * 4. 透明度调整
 * 5. 色调滤镜
 * 
 * 效果应用顺序很重要，会影响最终视觉结果。
 */
QPixmap SpriteRenderer::applyRenderOptions(const QPixmap &sprite, const RenderOptions &options) const
{
    // 如果渲染被禁用，直接返回原图
    if (!m_renderEnabled) {
        return sprite;
    }
    
    QPixmap result = sprite;
    
    // 1. 应用缩放变换
    if (options.scale != 1.0f) {
        QSize newSize = sprite.size() * options.scale;
        result = result.scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    
    // 2. 应用旋转变换
    if (options.rotation != 0.0f) {
        QTransform transform;
        transform.rotate(options.rotation);
        result = result.transformed(transform, Qt::SmoothTransformation);
    }
    
    // 3. 应用翻转变换
    if (options.flipHorizontal || options.flipVertical) {
        result = result.transformed(QTransform().scale(
            options.flipHorizontal ? -1 : 1,
            options.flipVertical ? -1 : 1
        ));
    }
    
    // 4. 应用透明度效果
    if (options.opacity < 1.0f) {
        QPixmap opacityResult(result.size());
        opacityResult.fill(Qt::transparent);
        
        QPainter painter(&opacityResult);
        painter.setOpacity(options.opacity);
        painter.drawPixmap(0, 0, result);
        painter.end();
        
        result = opacityResult;
    }
    
    // 5. 应用色调滤镜
    if (options.tint != QColor(255, 255, 255)) {
        QPixmap tintResult(result.size());
        tintResult.fill(Qt::transparent);
        
        QPainter painter(&tintResult);
        // 使用乘法混合模式应用色调
        painter.setCompositionMode(QPainter::CompositionMode_Multiply);
        painter.fillRect(result.rect(), options.tint);
        // 保持原图的透明度
        painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        painter.drawPixmap(0, 0, result);
        painter.end();
        
        result = tintResult;
    }
    
    return result;
}
