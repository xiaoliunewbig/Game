/*
 * 文件名: SpriteRenderer.h
 * 说明: 精灵渲染器头文件
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 * 
 * 本文件定义了游戏精灵渲染系统的核心接口，提供高效的图像处理能力。
 * 
 * 核心特性：
 * - 统一的图像资源管理
 * - 智能缓存机制
 * - 精灵表（Sprite Sheet）支持
 * - 丰富的渲染效果
 * - 内存优化设计
 * 
 * 使用场景：
 * - 游戏角色精灵渲染
 * - UI图标和按钮
 * - 背景和环境元素
 * - 动画帧序列
 * 
 * 性能考虑：
 * - 缓存避免重复加载
 * - 按需渲染减少内存占用
 * - 支持渲染开关进行性能调优
 */

#ifndef SPRITERENDERER_H
#define SPRITERENDERER_H

#include <QObject>
#include <QPixmap>
#include <QMap>
#include <QColor>
#include <QSize>

/**
 * @brief 精灵渲染器类
 * 
 * 负责游戏精灵的加载、缓存和渲染，是图形系统的核心组件。
 * 
 * 主要职责：
 * - 图像资源的统一管理
 * - 内存缓存优化
 * - 精灵表解析和帧提取
 * - 视觉效果处理
 * - 渲染性能控制
 * 
 * 设计原则：
 * - 单一职责：专注于图像处理
 * - 开闭原则：易于扩展新的渲染效果
 * - 依赖倒置：通过信号解耦与其他系统
 */
class SpriteRenderer : public QObject
{
    Q_OBJECT
    
    // Qt属性系统，支持QML绑定和运行时查询
    Q_PROPERTY(bool renderEnabled READ isRenderEnabled WRITE setRenderEnabled NOTIFY renderEnabledChanged)
    Q_PROPERTY(int cacheSize READ getCacheSize NOTIFY cacheSizeChanged)
    
public:
    /**
     * @brief 渲染选项结构
     * 
     * 定义了各种视觉效果的参数，支持组合使用。
     * 所有效果都有合理的默认值，可以选择性设置。
     */
    struct RenderOptions {
        /** @brief 缩放比例，1.0为原始大小 */
        float scale = 1.0f;
        
        /** @brief 旋转角度（度），顺时针为正 */
        float rotation = 0.0f;
        
        /** @brief 是否水平翻转 */
        bool flipHorizontal = false;
        
        /** @brief 是否垂直翻转 */
        bool flipVertical = false;
        
        /** @brief 透明度，范围0.0-1.0 */
        float opacity = 1.0f;
        
        /** @brief 色调滤镜，白色(255,255,255)为无效果 */
        QColor tint = QColor(255, 255, 255);
        
        /** @brief 帧索引，-1表示使用完整图像 */
        int frameIndex = -1;
    };
    
    /**
     * @brief 精灵信息结构
     * 
     * 包含精灵的完整元数据，用于调试和资源管理。
     */
    struct SpriteInfo {
        /** @brief 精灵名称（缓存键） */
        QString name;
        
        /** @brief 源文件路径 */
        QString filePath;
        
        /** @brief 原始图像大小 */
        QSize size;
        
        /** @brief 是否为精灵表 */
        bool isSpriteSheet = false;
        
        /** @brief 动画帧数（精灵表专用） */
        int frameCount = 1;
        
        /** @brief 单帧大小（精灵表专用） */
        QSize frameSize;
        
        /** @brief 是否已成功加载 */
        bool isLoaded = false;
    };
    
    /**
     * @brief 构造函数
     * @param parent 父对象指针，用于Qt对象树管理
     */
    explicit SpriteRenderer(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     * 
     * 自动清理所有缓存资源
     */
    ~SpriteRenderer();
    
    // ==================== 精灵加载接口 ====================
    
    /**
     * @brief 加载单张精灵图像
     * @param name 精灵名称，作为缓存键使用
     * @param filePath 图像文件的完整路径
     * @return 加载成功返回true，失败返回false
     * 
     * 支持的图像格式：PNG, JPG, BMP, GIF, TIFF等Qt支持的格式。
     * 建议使用PNG格式以获得最佳的透明度支持。
     * 
     * 注意事项：
     * - 名称必须唯一，重复名称会覆盖已有精灵
     * - 文件路径可以是相对路径或绝对路径
     * - 加载失败会输出警告信息到日志
     */
    bool loadSprite(const QString &name, const QString &filePath);
    
    /**
     * @brief 加载精灵表（Sprite Sheet）
     * @param name 精灵表名称
     * @param filePath 精灵表文件路径
     * @param frameWidth 单帧宽度（像素）
     * @param frameHeight 单帧高度（像素）
     * @param frameCount 总帧数
     * @return 加载成功返回true，失败返回false
     * 
     * 精灵表是包含多个动画帧的大图像，按网格排列。
     * 
     * 布局约定：
     * - 帧按从左到右、从上到下的顺序排列
     * - 帧索引从0开始编号
     * - 图像大小应该能被帧大小整除
     * 
     * 使用建议：
     * - 将相关动画帧放在同一个精灵表中
     * - 保持帧大小一致以简化处理
     * - 合理安排帧数量以平衡内存和性能
     */
    bool loadSpriteSheet(const QString &name, const QString &filePath, 
                        int frameWidth, int frameHeight, int frameCount);
    
    /**
     * @brief 卸载精灵
     * @param name 要卸载的精灵名称
     * 
     * 从缓存中移除指定精灵，释放相关内存。
     * 适用于场景切换时清理不再需要的资源。
     */
    void unloadSprite(const QString &name);
    
    // ==================== 精灵查询接口 ====================
    
    /**
     * @brief 检查精灵是否已加载
     * @param name 精灵名称
     * @return 已加载返回true，否则返回false
     */
    bool hasSprite(const QString &name) const;
    
    /**
     * @brief 获取完整精灵图像
     * @param name 精灵名称
     * @return 精灵的QPixmap对象，失败返回空QPixmap
     * 
     * 返回原始的、未经处理的图像。对于精灵表，
     * 返回包含所有帧的完整图像。
     */
    QPixmap getSprite(const QString &name) const;
    
    /**
     * @brief 获取精灵表的指定帧
     * @param name 精灵表名称
     * @param frameIndex 帧索引（从0开始）
     * @return 指定帧的QPixmap对象，失败返回空QPixmap
     * 
     * 从精灵表中提取单个动画帧。对于普通精灵，
     * 忽略frameIndex参数并返回完整图像。
     * 
     * 性能提示：
     * - 帧提取涉及图像复制，频繁调用可能影响性能
     * - 考虑缓存常用帧或使用renderSprite方法
     */
    QPixmap getSpriteFrame(const QString &name, int frameIndex) const;
    
    /**
     * @brief 渲染精灵（应用视觉效果）
     * @param name 精灵名称
     * @param options 渲染选项，默认为无效果
     * @return 应用效果后的QPixmap对象
     * 
     * 这是推荐的精灵获取方法，支持丰富的视觉效果。
     * 可以在一次调用中应用多种效果，提高渲染效率。
     * 
     * 效果组合：
     * - 所有效果可以同时使用
     * - 效果按特定顺序应用以确保正确结果
     * - 某些效果组合可能产生意外的视觉结果
     */
    QPixmap renderSprite(const QString &name, const RenderOptions &options = RenderOptions()) const;
    
    // ==================== 缓存管理接口 ====================
    
    /**
     * @brief 清空所有精灵缓存
     * 
     * 释放所有已加载的精灵资源，用于内存清理。
     * 通常在以下情况调用：
     * - 场景或关卡切换
     * - 内存不足时的紧急清理
     * - 游戏退出前的资源清理
     */
    void clearCache();
    
    /**
     * @brief 获取缓存中的精灵数量
     * @return 当前缓存的精灵总数
     * 
     * 用于监控内存使用情况和调试
     */
    int getCacheSize() const;
    
    /**
     * @brief 获取所有已加载精灵的名称列表
     * @return 精灵名称的字符串列表
     * 
     * 用于：
     * - 调试和日志记录
     * - 资源管理界面显示
     * - 批量操作的准备工作
     */
    QStringList getLoadedSprites() const;
    
    /**
     * @brief 获取精灵的详细信息
     * @param name 精灵名称
     * @return 包含精灵详细信息的结构体
     * 
     * 提供精灵的完整元数据，包括文件信息、
     * 尺寸信息、精灵表信息等。
     */
    SpriteInfo getSpriteInfo(const QString &name) const;
    
    // ==================== 渲染控制接口 ====================
    
    /**
     * @brief 设置渲染是否启用
     * @param enabled 是否启用渲染效果
     * 
     * 禁用渲染时，所有视觉效果都会被跳过，
     * 直接返回原始图像。用于性能优化。
     */
    void setRenderEnabled(bool enabled);
    
    /**
     * @brief 获取渲染是否启用
     * @return 当前渲染启用状态
     */
    bool isRenderEnabled() const;

signals:
    // ==================== 资源管理信号 ====================
    
    /**
     * @brief 精灵加载完成信号
     * @param name 已加载的精灵名称
     * 
     * 当精灵成功加载到缓存时发送
     */
    void spriteLoaded(const QString &name);
    
    /**
     * @brief 精灵卸载完成信号
     * @param name 已卸载的精灵名称
     * 
     * 当精灵从缓存中移除时发送
     */
    void spriteUnloaded(const QString &name);
    
    /**
     * @brief 缓存清空完成信号
     * 
     * 当所有精灵缓存被清空时发送
     */
    void cacheCleared();
    
    /**
     * @brief 缓存大小变化信号
     * @param size 新的缓存大小
     * 
     * 当缓存中的精灵数量发生变化时发送
     */
    void cacheSizeChanged(int size);
    
    /**
     * @brief 渲染启用状态变化信号
     * @param enabled 新的启用状态
     * 
     * 当渲染启用状态改变时发送
     */
    void renderEnabledChanged(bool enabled);

private:
    /**
     * @brief 精灵数据内部结构
     * 
     * 存储单个精灵的完整信息，包括图像数据和元数据。
     * 这是内部使用的数据结构，外部通过SpriteInfo访问。
     */
    struct SpriteData {
        QString name;                          ///< 精灵名称
        QString filePath;                      ///< 文件路径
        QPixmap pixmap;                        ///< 像素图
        QSize originalSize;                    ///< 原始大小
        bool isLoaded = false;                 ///< 是否已加载
        
        // 精灵表相关
        bool isSpriteSheet = false;            ///< 是否为精灵表
        int frameWidth = 0;                    ///< 帧宽度
        int frameHeight = 0;                   ///< 帧高度
        int frameCount = 1;                    ///< 帧数
        int columns = 1;                       ///< 列数
        int rows = 1;                          ///< 行数
    };
    
    QPixmap applyRenderOptions(const QPixmap &sprite, const RenderOptions &options) const;
    
private:
    QMap<QString, SpriteData> m_spriteCache;   ///< 精灵缓存
    bool m_renderEnabled;                      ///< 渲染是否启用
};

#endif // SPRITERENDERER_H
