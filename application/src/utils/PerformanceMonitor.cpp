/*
 * 文件名: PerformanceMonitor.cpp
 * 说明: 性能监控器实现文件
 * 作者: 彭承康
 * 创建时间: 2025-07-24
 * 版本: v1.0.0
 * 
 * 功能描述:
 * - 实时监控游戏性能指标
 * - 提供性能分析和优化建议
 * - 支持性能数据的记录和导出
 * - 实现性能警告和通知机制
 */

#include "utils/PerformanceMonitor.h"
#include <QTimer>
#include <QDebug>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QThread>
#include <QMutexLocker>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#elif defined(Q_OS_LINUX)
#include <unistd.h>
#include <sys/resource.h>
#include <fstream>
#endif

PerformanceMonitor* PerformanceMonitor::s_instance = nullptr;
QMutex PerformanceMonitor::s_mutex;

PerformanceMonitor* PerformanceMonitor::instance()
{
    if (!s_instance) {
        QMutexLocker locker(&s_mutex);
        if (!s_instance) {
            s_instance = new PerformanceMonitor();
        }
    }
    return s_instance;
}

PerformanceMonitor::PerformanceMonitor(QObject *parent)
    : QObject(parent)
    , m_isEnabled(false)
    , m_updateInterval(1000)  // 1秒更新间隔
    , m_maxHistorySize(300)   // 保存5分钟历史数据
    , m_fpsWarningThreshold(30.0f)
    , m_memoryWarningThreshold(1024 * 1024 * 1024)  // 1GB
    , m_frameTimeWarningThreshold(33333)  // 33.33ms (30fps)
{
    // 初始化性能指标
    resetMetrics();
    
    // 创建更新定时器
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(m_updateInterval);
    connect(m_updateTimer, &QTimer::timeout, this, &PerformanceMonitor::updateMetrics);
    
    // 创建数据目录
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/performance";
    QDir().mkpath(dataDir);
    m_logFilePath = dataDir + "/performance.log";
    
    qDebug() << "PerformanceMonitor: 性能监控器初始化完成";
}

PerformanceMonitor::~PerformanceMonitor()
{
    stop();
    
    // 保存最终报告
    if (m_isEnabled) {
        savePerformanceReport();
    }
    
    qDebug() << "PerformanceMonitor: 性能监控器销毁";
}

void PerformanceMonitor::start()
{
    if (m_isEnabled) {
        qDebug() << "PerformanceMonitor: 已经在运行中";
        return;
    }
    
    m_isEnabled = true;
    m_startTime = QDateTime::currentMSecsSinceEpoch();
    
    // 重置指标
    resetMetrics();
    
    // 启动定时器
    m_updateTimer->start();
    
    emit monitoringStarted();
    qDebug() << "PerformanceMonitor: 性能监控已启动";
}

void PerformanceMonitor::stop()
{
    if (!m_isEnabled) {
        return;
    }
    
    m_isEnabled = false;
    m_updateTimer->stop();
    
    emit monitoringStopped();
    qDebug() << "PerformanceMonitor: 性能监控已停止";
}

void PerformanceMonitor::pause()
{
    if (!m_isEnabled) {
        return;
    }
    
    m_updateTimer->stop();
    emit monitoringPaused();
    qDebug() << "PerformanceMonitor: 性能监控已暂停";
}

void PerformanceMonitor::resume()
{
    if (!m_isEnabled) {
        return;
    }
    
    m_updateTimer->start();
    emit monitoringResumed();
    qDebug() << "PerformanceMonitor: 性能监控已恢复";
}

void PerformanceMonitor::beginFrame()
{
    if (!m_isEnabled) {
        return;
    }

    m_frameStartTime = QDateTime::currentMSecsSinceEpoch() * 1000; // 转换为微秒
}

void PerformanceMonitor::endFrame()
{
    if (!m_isEnabled) {
        return;
    }

    qint64 now = QDateTime::currentMSecsSinceEpoch() * 1000;
    qint64 frameTime = now - m_frameStartTime;

    m_frameTimes.enqueue(frameTime);
    if (m_frameTimes.size() > 120) {
        m_frameTimes.dequeue();
    }

    m_frameCount++;
    m_lastFrameTime = frameTime;

    updateFPSStats();
}

float PerformanceMonitor::getCurrentFPS() const
{
    return m_currentFPS;
}

float PerformanceMonitor::getAverageFPS() const
{
    return m_averageFPS;
}

float PerformanceMonitor::getMinFPS() const
{
    return m_minFPS;
}

float PerformanceMonitor::getMaxFPS() const
{
    return m_maxFPS;
}

qint64 PerformanceMonitor::getMemoryUsage() const
{
    return m_currentMemoryUsage;
}

qint64 PerformanceMonitor::getPeakMemoryUsage() const
{
    return m_peakMemoryUsage;
}

void PerformanceMonitor::resetMemoryStats()
{
    m_currentMemoryUsage = 0;
    m_peakMemoryUsage = 0;
}

void PerformanceMonitor::startMonitoring()
{
    start();
}

void PerformanceMonitor::stopMonitoring()
{
    stop();
}

void PerformanceMonitor::setMonitoringEnabled(bool enabled)
{
    if (enabled) {
        start();
    } else {
        stop();
    }
    emit monitoringEnabledChanged(enabled);
}

bool PerformanceMonitor::isMonitoringEnabled() const
{
    return m_isEnabled;
}

void PerformanceMonitor::setMonitoringInterval(int intervalMs)
{
    m_updateInterval = qMax(100, intervalMs);
    m_updateTimer->setInterval(m_updateInterval);
}

int PerformanceMonitor::getMonitoringInterval() const
{
    return m_updateInterval;
}

void PerformanceMonitor::recordDrawCalls(int count)
{
    m_drawCalls = count;
}

void PerformanceMonitor::recordTriangles(int count)
{
    m_triangles = count;
}

void PerformanceMonitor::recordRenderTime(qint64 timeUs)
{
    m_renderTime = timeUs;
}

int PerformanceMonitor::getDrawCalls() const
{
    return m_drawCalls;
}

int PerformanceMonitor::getTriangles() const
{
    return m_triangles;
}

void PerformanceMonitor::beginProfileSection(const QString &name)
{
    QMutexLocker locker(&m_profileMutex);

    auto &section = m_profileSections[name];
    section.timer.start();
    section.isActive = true;
}

void PerformanceMonitor::endProfileSection(const QString &name)
{
    QMutexLocker locker(&m_profileMutex);

    auto it = m_profileSections.find(name);
    if (it != m_profileSections.end() && it->isActive) {
        qint64 elapsed = it->timer.nsecsElapsed() / 1000; // 纳秒转微秒
        it->totalTime += elapsed;
        it->callCount++;
        it->isActive = false;
    }
}

qint64 PerformanceMonitor::getProfileSectionTime(const QString &name) const
{
    QMutexLocker locker(&m_profileMutex);

    auto it = m_profileSections.find(name);
    if (it != m_profileSections.end() && it->callCount > 0) {
        return it->totalTime / it->callCount;
    }
    return 0;
}

QMap<QString, qint64> PerformanceMonitor::getAllProfileSections() const
{
    QMutexLocker locker(&m_profileMutex);

    QMap<QString, qint64> result;
    for (auto it = m_profileSections.begin(); it != m_profileSections.end(); ++it) {
        if (it->callCount > 0) {
            result[it.key()] = it->totalTime / it->callCount;
        }
    }
    return result;
}

void PerformanceMonitor::setFPSWarningThreshold(float threshold)
{
    m_fpsWarningThreshold = threshold;
}

void PerformanceMonitor::setMemoryWarningThreshold(qint64 threshold)
{
    m_memoryWarningThreshold = threshold;
}

void PerformanceMonitor::setFrameTimeWarningThreshold(qint64 threshold)
{
    m_frameTimeWarningThreshold = threshold;
}

PerformanceMonitor::PerformanceMetrics PerformanceMonitor::getCurrentMetrics() const
{
    PerformanceMetrics metrics;
    metrics.fps = m_currentFPS;
    metrics.averageFps = m_averageFPS;
    metrics.minFps = m_minFPS;
    metrics.maxFps = m_maxFPS;
    metrics.memoryUsage = m_currentMemoryUsage;
    metrics.cpuUsage = 0.0f;
    metrics.drawCalls = m_drawCalls;
    metrics.triangles = m_triangles;
    metrics.frameTime = m_lastFrameTime;
    metrics.renderTime = m_renderTime;
    metrics.updateTime = 0;
    return metrics;
}

QList<PerformanceMonitor::PerformanceMetrics> PerformanceMonitor::getHistoryMetrics(int seconds) const
{
    int count = qMin(seconds, m_metricsHistory.size());
    QList<PerformanceMetrics> result;
    int start = m_metricsHistory.size() - count;
    for (int i = start; i < m_metricsHistory.size(); ++i) {
        result.append(m_metricsHistory.at(i));
    }
    return result;
}

QString PerformanceMonitor::generatePerformanceReport() const
{
    QString report;
    report += "=== 性能监控报告 ===\n";
    report += QString("当前 FPS: %1\n").arg(m_currentFPS, 0, 'f', 1);
    report += QString("平均 FPS: %1\n").arg(m_averageFPS, 0, 'f', 1);
    report += QString("最低 FPS: %1\n").arg(m_minFPS, 0, 'f', 1);
    report += QString("最高 FPS: %1\n").arg(m_maxFPS, 0, 'f', 1);
    report += QString("内存使用: %1 MB\n").arg(m_currentMemoryUsage / (1024.0 * 1024.0), 0, 'f', 2);
    report += QString("峰值内存: %1 MB\n").arg(m_peakMemoryUsage / (1024.0 * 1024.0), 0, 'f', 2);
    report += QString("绘制调用: %1\n").arg(m_drawCalls);
    report += QString("三角形数: %1\n").arg(m_triangles);
    report += QString("帧时间: %1 us\n").arg(m_lastFrameTime);
    report += QString("渲染时间: %1 us\n").arg(m_renderTime);

    // 添加分析段信息
    QMap<QString, qint64> sections = getAllProfileSections();
    if (!sections.isEmpty()) {
        report += "\n--- 性能分析段 ---\n";
        for (auto it = sections.begin(); it != sections.end(); ++it) {
            report += QString("  %1: %2 us\n").arg(it.key()).arg(it.value());
        }
    }

    report += "==================\n";
    return report;
}

bool PerformanceMonitor::exportPerformanceData(const QString &filePath) const
{
    QJsonObject root;

    // 基本信息
    QJsonObject summary;
    summary["currentFPS"] = static_cast<double>(m_currentFPS);
    summary["averageFPS"] = static_cast<double>(m_averageFPS);
    summary["minFPS"] = static_cast<double>(m_minFPS);
    summary["maxFPS"] = static_cast<double>(m_maxFPS);
    summary["memoryUsageMB"] = m_currentMemoryUsage / (1024.0 * 1024.0);
    summary["peakMemoryUsageMB"] = m_peakMemoryUsage / (1024.0 * 1024.0);
    root["summary"] = summary;

    // 历史数据
    QJsonArray history;
    for (const auto &metrics : m_metricsHistory) {
        QJsonObject entry;
        entry["fps"] = static_cast<double>(metrics.fps);
        entry["memory"] = metrics.memoryUsage;
        entry["frameTime"] = metrics.frameTime;
        history.append(entry);
    }
    root["history"] = history;

    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "PerformanceMonitor: 无法写入性能数据文件:" << filePath;
        return false;
    }

    file.write(doc.toJson());
    file.close();

    qDebug() << "PerformanceMonitor: 性能数据已导出到:" << filePath;
    return true;
}

void PerformanceMonitor::updateMetrics()
{
    updateMemoryStats();

    PerformanceMetrics metrics = getCurrentMetrics();

    // 添加到历史数据
    m_metricsHistory.enqueue(metrics);
    while (m_metricsHistory.size() > m_maxHistorySize) {
        m_metricsHistory.dequeue();
    }

    // 检查性能警告
    checkPerformanceWarnings(metrics);

    emit metricsUpdated(metrics);
}

void PerformanceMonitor::updateFPSStats()
{
    if (m_frameTimes.isEmpty()) {
        return;
    }

    // 计算当前 FPS
    if (m_lastFrameTime > 0) {
        m_currentFPS = 1000000.0f / m_lastFrameTime; // 微秒转 FPS
    }

    // 计算平均 FPS
    qint64 totalTime = 0;
    for (qint64 t : m_frameTimes) {
        totalTime += t;
    }
    if (totalTime > 0) {
        m_averageFPS = m_frameTimes.size() * 1000000.0f / totalTime;
    }

    // 更新最小/最大 FPS
    if (m_currentFPS > 0) {
        if (m_currentFPS < m_minFPS || m_minFPS == 0) {
            m_minFPS = m_currentFPS;
        }
        if (m_currentFPS > m_maxFPS) {
            m_maxFPS = m_currentFPS;
        }
    }

    emit fpsChanged(m_currentFPS);
    emit averageFpsChanged(m_averageFPS);
}

void PerformanceMonitor::updateMemoryStats()
{
    qint64 memUsage = 0;

#ifdef Q_OS_WIN
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(),
                             reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc),
                             sizeof(pmc))) {
        memUsage = static_cast<qint64>(pmc.WorkingSetSize);
    }
#elif defined(Q_OS_LINUX)
    std::ifstream statm("/proc/self/statm");
    if (statm.is_open()) {
        long pages = 0;
        statm >> pages; // total program size
        memUsage = pages * sysconf(_SC_PAGESIZE);
    }
#endif

    m_currentMemoryUsage = memUsage;
    if (memUsage > m_peakMemoryUsage) {
        m_peakMemoryUsage = memUsage;
    }

    emit memoryUsageChanged(m_currentMemoryUsage);
}

void PerformanceMonitor::checkPerformanceWarnings(const PerformanceMetrics &metrics)
{
    // 低帧率警告
    if (metrics.fps > 0 && metrics.fps < m_fpsWarningThreshold) {
        emit performanceWarning(WarningType::LowFPS,
                               QString("帧率过低: %1 FPS").arg(metrics.fps, 0, 'f', 1),
                               metrics.fps);
    }

    // 高内存使用警告
    if (metrics.memoryUsage > m_memoryWarningThreshold) {
        emit performanceWarning(WarningType::HighMemoryUsage,
                               QString("内存使用过高: %1 MB")
                                   .arg(metrics.memoryUsage / (1024.0 * 1024.0), 0, 'f', 1),
                               static_cast<double>(metrics.memoryUsage));
    }

    // 长帧时间警告
    if (metrics.frameTime > m_frameTimeWarningThreshold) {
        emit performanceWarning(WarningType::LongFrameTime,
                               QString("帧时间过长: %1 ms")
                                   .arg(metrics.frameTime / 1000.0, 0, 'f', 2),
                               static_cast<double>(metrics.frameTime));
    }
}

void PerformanceMonitor::resetMetrics()
{
    m_currentFPS = 0.0f;
    m_averageFPS = 0.0f;
    m_minFPS = 0.0f;
    m_maxFPS = 0.0f;
    m_lastFrameTime = 0;
    m_frameCount = 0;
    m_currentMemoryUsage = 0;
    m_peakMemoryUsage = 0;
    m_drawCalls = 0;
    m_triangles = 0;
    m_renderTime = 0;
    m_frameStartTime = 0;
    m_frameTimes.clear();
    m_metricsHistory.clear();
    m_profileSections.clear();
}

void PerformanceMonitor::savePerformanceReport()
{
    QString reportDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/performance";
    QDir().mkpath(reportDir);

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString reportPath = QString("%1/report_%2.json").arg(reportDir, timestamp);

    exportPerformanceData(reportPath);
}