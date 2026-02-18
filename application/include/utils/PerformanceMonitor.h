/*
 * 文件名: PerformanceMonitor.h
 * 说明: 性能监控器头文件
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 * 
 * 本文件定义了游戏性能监控系统，用于实时监控和分析游戏性能。
 * 
 * 监控指标：
 * - 帧率（FPS）统计
 * - 内存使用情况
 * - CPU使用率
 * - 渲染性能
 * - 网络延迟
 * - 资源加载时间
 * 
 * 功能特性：
 * - 实时性能数据收集
 * - 性能历史记录
 * - 性能警告和阈值检测
 * - 性能报告生成
 * - 调试信息显示
 */

#ifndef PERFORMANCEMONITOR_H
#define PERFORMANCEMONITOR_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QQueue>
#include <QMap>
#include <QMutex>

/**
 * @brief 性能监控器类
 * 
 * 负责收集、分析和报告游戏性能数据
 */
class PerformanceMonitor : public QObject
{
    Q_OBJECT
    
    // Qt属性系统
    Q_PROPERTY(float currentFPS READ getCurrentFPS NOTIFY fpsChanged)
    Q_PROPERTY(float averageFPS READ getAverageFPS NOTIFY averageFpsChanged)
    Q_PROPERTY(qint64 memoryUsage READ getMemoryUsage NOTIFY memoryUsageChanged)
    Q_PROPERTY(bool monitoringEnabled READ isMonitoringEnabled WRITE setMonitoringEnabled NOTIFY monitoringEnabledChanged)
    
public:
    /**
     * @brief 性能指标结构
     */
    struct PerformanceMetrics {
        float fps;                  ///< 当前帧率
        float averageFps;           ///< 平均帧率
        float minFps;               ///< 最低帧率
        float maxFps;               ///< 最高帧率
        qint64 memoryUsage;         ///< 内存使用量（字节）
        float cpuUsage;             ///< CPU使用率（百分比）
        int drawCalls;              ///< 绘制调用次数
        int triangles;              ///< 三角形数量
        qint64 frameTime;           ///< 帧时间（微秒）
        qint64 renderTime;          ///< 渲染时间（微秒）
        qint64 updateTime;          ///< 更新时间（微秒）
    };
    
    /**
     * @brief 性能警告类型
     */
    enum class WarningType {
        LowFPS,                     ///< 低帧率警告
        HighMemoryUsage,            ///< 高内存使用警告
        HighCPUUsage,               ///< 高CPU使用警告
        LongFrameTime,              ///< 长帧时间警告
        TooManyDrawCalls            ///< 过多绘制调用警告
    };
    
    /**
     * @brief 获取单例实例
     * @return PerformanceMonitor的唯一实例
     */
    static PerformanceMonitor* instance();
    
    /**
     * @brief 析构函数
     */
    ~PerformanceMonitor();
    
    // ==================== 监控控制接口 ====================

    /**
     * @brief 开始性能监控
     */
    void start();

    /**
     * @brief 停止性能监控
     */
    void stop();

    /**
     * @brief 暂停性能监控
     */
    void pause();

    /**
     * @brief 恢复性能监控
     */
    void resume();

    /**
     * @brief 开始性能监控（兼容接口）
     */
    void startMonitoring();

    /**
     * @brief 停止性能监控（兼容接口）
     */
    void stopMonitoring();

    /**
     * @brief 设置监控是否启用
     * @param enabled 是否启用
     */
    void setMonitoringEnabled(bool enabled);

    /**
     * @brief 获取监控是否启用
     * @return 监控启用状态
     */
    bool isMonitoringEnabled() const;

    /**
     * @brief 获取监控是否启用（别名）
     * @return 监控启用状态
     */
    bool isEnabled() const { return isMonitoringEnabled(); }

    /**
     * @brief 设置监控间隔
     * @param intervalMs 监控间隔（毫秒）
     */
    void setMonitoringInterval(int intervalMs);

    /**
     * @brief 获取监控间隔
     * @return 监控间隔（毫秒）
     */
    int getMonitoringInterval() const;
    
    // ==================== 帧率监控接口 ====================
    
    /**
     * @brief 记录帧开始
     * 
     * 在每帧开始时调用，用于计算帧率和帧时间
     */
    void beginFrame();
    
    /**
     * @brief 记录帧结束
     * 
     * 在每帧结束时调用，完成帧率统计
     */
    void endFrame();
    
    /**
     * @brief 获取当前帧率
     * @return 当前帧率（FPS）
     */
    float getCurrentFPS() const;
    
    /**
     * @brief 获取平均帧率
     * @return 平均帧率（FPS）
     */
    float getAverageFPS() const;
    
    /**
     * @brief 获取最低帧率
     * @return 最低帧率（FPS）
     */
    float getMinFPS() const;
    
    /**
     * @brief 获取最高帧率
     * @return 最高帧率（FPS）
     */
    float getMaxFPS() const;
    
    // ==================== 内存监控接口 ====================
    
    /**
     * @brief 获取当前内存使用量
     * @return 内存使用量（字节）
     */
    qint64 getMemoryUsage() const;
    
    /**
     * @brief 获取峰值内存使用量
     * @return 峰值内存使用量（字节）
     */
    qint64 getPeakMemoryUsage() const;
    
    /**
     * @brief 重置内存统计
     */
    void resetMemoryStats();
    
    // ==================== 渲染监控接口 ====================
    
    /**
     * @brief 记录绘制调用
     * @param count 绘制调用次数
     */
    void recordDrawCalls(int count);
    
    /**
     * @brief 记录三角形数量
     * @param count 三角形数量
     */
    void recordTriangles(int count);
    
    /**
     * @brief 记录渲染时间
     * @param timeUs 渲染时间（微秒）
     */
    void recordRenderTime(qint64 timeUs);
    
    /**
     * @brief 获取绘制调用次数
     * @return 绘制调用次数
     */
    int getDrawCalls() const;
    
    /**
     * @brief 获取三角形数量
     * @return 三角形数量
     */
    int getTriangles() const;
    
    // ==================== 性能分析接口 ====================
    
    /**
     * @brief 开始性能分析段
     * @param name 分析段名称
     */
    void beginProfileSection(const QString &name);
    
    /**
     * @brief 结束性能分析段
     * @param name 分析段名称
     */
    void endProfileSection(const QString &name);
    
    /**
     * @brief 获取分析段平均时间
     * @param name 分析段名称
     * @return 平均时间（微秒）
     */
    qint64 getProfileSectionTime(const QString &name) const;
    
    /**
     * @brief 获取所有分析段信息
     * @return 分析段时间映射表
     */
    QMap<QString, qint64> getAllProfileSections() const;
    
    // ==================== 警告系统接口 ====================
    
    /**
     * @brief 设置FPS警告阈值
     * @param threshold 阈值（FPS）
     */
    void setFPSWarningThreshold(float threshold);
    
    /**
     * @brief 设置内存警告阈值
     * @param threshold 阈值（字节）
     */
    void setMemoryWarningThreshold(qint64 threshold);
    
    /**
     * @brief 设置帧时间警告阈值
     * @param threshold 阈值（毫秒）
     */
    void setFrameTimeWarningThreshold(qint64 threshold);
    
    // ==================== 数据获取接口 ====================
    
    /**
     * @brief 获取当前性能指标
     * @return 性能指标结构
     */
    PerformanceMetrics getCurrentMetrics() const;
    
    /**
     * @brief 获取性能历史数据
     * @param seconds 历史时间长度（秒）
     * @return 性能指标历史列表
     */
    QList<PerformanceMetrics> getHistoryMetrics(int seconds) const;
    
    /**
     * @brief 生成性能报告
     * @return 性能报告字符串
     */
    QString generatePerformanceReport() const;
    
    /**
     * @brief 导出性能数据
     * @param filePath 导出文件路径
     * @return 导出成功返回true，否则返回false
     */
    bool exportPerformanceData(const QString &filePath) const;

signals:
    // ==================== 性能变化信号 ====================
    
    /**
     * @brief 帧率变化信号
     * @param fps 当前帧率
     */
    void fpsChanged(float fps);
    
    /**
     * @brief 平均帧率变化信号
     * @param averageFps 平均帧率
     */
    void averageFpsChanged(float averageFps);
    
    /**
     * @brief 内存使用量变化信号
     * @param memoryUsage 内存使用量
     */
    void memoryUsageChanged(qint64 memoryUsage);
    
    /**
     * @brief 监控启用状态变化信号
     * @param enabled 启用状态
     */
    void monitoringEnabledChanged(bool enabled);
    
    /**
     * @brief 性能指标更新信号
     * @param metrics 性能指标
     */
    void metricsUpdated(const PerformanceMetrics &metrics);
    
    /**
     * @brief 性能警告信号
     * @param type 警告类型
     * @param message 警告消息
     * @param value 相关数值
     */
    void performanceWarning(WarningType type, const QString &message, double value);

    /**
     * @brief 监控已启动信号
     */
    void monitoringStarted();

    /**
     * @brief 监控已停止信号
     */
    void monitoringStopped();

    /**
     * @brief 监控已暂停信号
     */
    void monitoringPaused();

    /**
     * @brief 监控已恢复信号
     */
    void monitoringResumed();

private slots:
    /**
     * @brief 更新性能指标
     */
    void updateMetrics();

private:
    /**
     * @brief 私有构造函数（单例模式）
     * @param parent 父对象指针
     */
    explicit PerformanceMonitor(QObject *parent = nullptr);
    
    /**
     * @brief 分析段信息结构
     */
    struct ProfileSection {
        QElapsedTimer timer;
        qint64 totalTime;
        int callCount;
        bool isActive;
    };
    
    /**
     * @brief 检查性能警告
     * @param metrics 性能指标
     */
    void checkPerformanceWarnings(const PerformanceMetrics &metrics);

    /**
     * @brief 更新FPS统计
     */
    void updateFPSStats();

    /**
     * @brief 更新内存统计
     */
    void updateMemoryStats();

    /**
     * @brief 重置所有性能指标
     */
    void resetMetrics();

    /**
     * @brief 保存性能报告到文件
     */
    void savePerformanceReport();

private:
    static PerformanceMonitor* s_instance;          ///< 单例实例
    static QMutex s_mutex;                          ///< 单例互斥锁

    // 监控控制
    bool m_isEnabled;                               ///< 监控是否启用
    QTimer* m_updateTimer;                          ///< 更新定时器
    int m_updateInterval;                           ///< 监控间隔

    // 帧率统计
    QElapsedTimer m_frameTimer;                     ///< 帧计时器
    QQueue<qint64> m_frameTimes;                    ///< 帧时间队列
    float m_currentFPS;                             ///< 当前帧率
    float m_averageFPS;                             ///< 平均帧率
    float m_minFPS;                                 ///< 最低帧率
    float m_maxFPS;                                 ///< 最高帧率
    qint64 m_lastFrameTime;                         ///< 上一帧时间
    qint64 m_frameStartTime;                        ///< 帧开始时间
    int m_frameCount;                               ///< 帧计数

    // 内存统计
    qint64 m_currentMemoryUsage;                    ///< 当前内存使用量
    qint64 m_peakMemoryUsage;                       ///< 峰值内存使用量

    // 渲染统计
    int m_drawCalls;                                ///< 绘制调用次数
    int m_triangles;                                ///< 三角形数量
    qint64 m_renderTime;                            ///< 渲染时间

    // 性能分析
    QMap<QString, ProfileSection> m_profileSections; ///< 分析段映射
    mutable QMutex m_profileMutex;                  ///< 分析互斥锁

    // 历史数据
    QQueue<PerformanceMetrics> m_metricsHistory;    ///< 性能指标历史
    int m_maxHistorySize;                           ///< 最大历史记录数

    // 警告阈值
    float m_fpsWarningThreshold;                    ///< FPS警告阈值
    qint64 m_memoryWarningThreshold;                ///< 内存警告阈值
    qint64 m_frameTimeWarningThreshold;             ///< 帧时间警告阈值

    // 时间和文件
    qint64 m_startTime;                             ///< 监控开始时间
    QString m_logFilePath;                          ///< 日志文件路径
};

// ==================== 便利宏定义 ====================

/**
 * @brief 性能分析段的便利宏
 */
#define PROFILE_SCOPE(name) \
    PerformanceMonitor::instance()->beginProfileSection(name); \
    QScopeGuard profileGuard([=]() { \
        PerformanceMonitor::instance()->endProfileSection(name); \
    });

/**
 * @brief 函数性能分析的便利宏
 */
#define PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCTION__)

#endif // PERFORMANCEMONITOR_H