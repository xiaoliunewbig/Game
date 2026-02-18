/*
 * 文件名: Logger.cpp
 * 说明: 日志系统实现
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 *
 * 本文件实现游戏的日志记录功能，支持多种日志级别和输出方式。
 */

#include "utils/Logger.h"
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>
#include <QStringConverter>
#include <QMutexLocker>
#include <iostream>

Logger* Logger::s_instance = nullptr;
QMutex Logger::s_mutex;

Logger::Logger(QObject *parent)
    : QObject(parent)
    , m_logLevel(LogLevel::Info)
    , m_enableConsoleOutput(true)
    , m_enableFileOutput(true)
    , m_maxLogFiles(10)
    , m_maxFileSize(10 * 1024 * 1024) // 10MB
{
    initializeLogDirectory();
    openLogFile();
}

Logger::~Logger()
{
    if (m_logFile.isOpen()) {
        m_logFile.close();
    }
}

Logger* Logger::instance()
{
    QMutexLocker locker(&s_mutex);
    if (!s_instance) {
        s_instance = new Logger();
    }
    return s_instance;
}

void Logger::log(LogLevel level, const QString &message, const QString &category)
{
    if (level < m_logLevel) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString levelStr = logLevelToString(level);
    QString formattedMessage = QString("[%1] [%2] [%3] %4")
                              .arg(timestamp)
                              .arg(levelStr)
                              .arg(category)
                              .arg(message);
    
    // 控制台输出
    if (m_enableConsoleOutput) {
        outputToConsole(level, formattedMessage);
    }
    
    // 文件输出
    if (m_enableFileOutput && m_logFile.isOpen()) {
        outputToFile(formattedMessage);
    }
    
    // 发送信号给UI
    emit logMessage(level, message, category, timestamp);
}

void Logger::debug(const QString &message, const QString &category)
{
    log(LogLevel::Debug, message, category);
}

void Logger::info(const QString &message, const QString &category)
{
    log(LogLevel::Info, message, category);
}

void Logger::warning(const QString &message, const QString &category)
{
    log(LogLevel::Warning, message, category);
}

void Logger::error(const QString &message, const QString &category)
{
    log(LogLevel::Error, message, category);
}

void Logger::critical(const QString &message, const QString &category)
{
    log(LogLevel::Critical, message, category);
}

void Logger::setLogLevel(LogLevel level)
{
    QMutexLocker locker(&m_mutex);
    m_logLevel = level;
}

void Logger::setConsoleOutput(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_enableConsoleOutput = enabled;
}

void Logger::setFileOutput(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_enableFileOutput = enabled;
    
    if (enabled && !m_logFile.isOpen()) {
        openLogFile();
    } else if (!enabled && m_logFile.isOpen()) {
        m_logFile.close();
    }
}

void Logger::setMaxLogFiles(int count)
{
    QMutexLocker locker(&m_mutex);
    m_maxLogFiles = qMax(1, count);
}

void Logger::setMaxFileSize(qint64 size)
{
    QMutexLocker locker(&m_mutex);
    m_maxFileSize = qMax(1024LL, size);
}

void Logger::initializeLogDirectory()
{
    QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    logDir += "/logs";
    
    QDir dir;
    if (!dir.exists(logDir)) {
        dir.mkpath(logDir);
    }
    
    m_logDirectory = logDir;
}

void Logger::openLogFile()
{
    if (m_logFile.isOpen()) {
        m_logFile.close();
    }
    
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString fileName = QString("%1/game_%2.log").arg(m_logDirectory, timestamp);
    
    m_logFile.setFileName(fileName);
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        m_logStream.setDevice(&m_logFile);
        m_logStream.setEncoding(QStringConverter::Utf8);
        
        // 写入日志文件头
        m_logStream << "=== Fantasy Legend Game Log ===" << Qt::endl;
        m_logStream << "Started at: " << QDateTime::currentDateTime().toString() << Qt::endl;
        m_logStream << "===============================" << Qt::endl;
        m_logStream.flush();
    }
    
    // 清理旧日志文件
    cleanupOldLogFiles();
}

void Logger::outputToConsole(LogLevel level, const QString &message)
{
    switch (level) {
        case LogLevel::Debug:
        case LogLevel::Info:
            std::cout << message.toStdString() << std::endl;
            break;
        case LogLevel::Warning:
        case LogLevel::Error:
        case LogLevel::Critical:
            std::cerr << message.toStdString() << std::endl;
            break;
    }
}

void Logger::outputToFile(const QString &message)
{
    if (!m_logFile.isOpen()) {
        return;
    }
    
    m_logStream << message << Qt::endl;
    m_logStream.flush();
    
    // 检查文件大小，如果超过限制则轮转
    if (m_logFile.size() > m_maxFileSize) {
        rotateLogFile();
    }
}

void Logger::rotateLogFile()
{
    m_logFile.close();
    openLogFile();
}

void Logger::cleanupOldLogFiles()
{
    QDir logDir(m_logDirectory);
    QStringList filters;
    filters << "game_*.log";
    
    QFileInfoList logFiles = logDir.entryInfoList(filters, QDir::Files, QDir::Time);
    
    // 删除超过最大数量的旧日志文件
    while (logFiles.size() > m_maxLogFiles) {
        QFileInfo oldestFile = logFiles.takeLast();
        QFile::remove(oldestFile.absoluteFilePath());
    }
}

QString Logger::logLevelToString(LogLevel level)
{
    switch (level) {
        case LogLevel::Debug:    return "DEBUG";
        case LogLevel::Info:     return "INFO ";
        case LogLevel::Warning:  return "WARN ";
        case LogLevel::Error:    return "ERROR";
        case LogLevel::Critical: return "CRIT ";
        default:                 return "UNKN ";
    }
}

// 全局日志函数
void logDebug(const QString &message, const QString &category)
{
    Logger::instance()->debug(message, category);
}

void logInfo(const QString &message, const QString &category)
{
    Logger::instance()->info(message, category);
}

void logWarning(const QString &message, const QString &category)
{
    Logger::instance()->warning(message, category);
}

void logError(const QString &message, const QString &category)
{
    Logger::instance()->error(message, category);
}

void logCritical(const QString &message, const QString &category)
{
    Logger::instance()->critical(message, category);
}