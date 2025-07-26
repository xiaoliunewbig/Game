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

