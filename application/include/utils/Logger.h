/*
 * 文件名: Logger.h
 * 说明: 日志系统头文件
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QString>

/**
 * @brief 日志级别枚举
 */
enum class LogLevel
{
    Debug = 0,      // 调试信息
    Info = 1,       // 一般信息
    Warning = 2,    // 警告信息
    Error = 3,      // 错误信息
    Critical = 4    // 严重错误
};

/**
 * @brief 日志系统类
 * 
 * 提供线程安全的日志记录功能，支持：
 * - 多种日志级别
 * - 控制台和文件输出
 * - 日志文件轮转
 * - 自动清理旧日志
 */
class Logger : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取日志系统单例实例
     * 
     * @return Logger* 日志系统实例
     */
    static Logger* instance();

    /**
     * @brief 记录日志
     * 
     * @param level 日志级别
     * @param message 日志消息
     * @param category 日志分类（默认为"General"）
     */
    void log(LogLevel level, const QString &message, const QString &category = "General");

    /**
     * @brief 记录调试日志
     * 
     * @param message 日志消息
     * @param category 日志分类
     */
    void debug(const QString &message, const QString &category = "General");

    /**
     * @brief 记录信息日志
     * 
     * @param message 日志消息
     * @param category 日志分类
     */
    void info(const QString &message, const QString &category = "General");

    /**
     * @brief 记录警告日志
     * 
     * @param message 日志消息
     * @param category 日志分类
     */
    void warning(const QString &message, const QString &category = "General");

    /**
     * @brief 记录错误日志
     * 
     * @param message 日志消息
     * @param category 日志分类
     */
    void error(const QString &message, const QString &category = "General");

    /**
     * @brief 记录严重错误日志
     * 
     * @param message 日志消息
     * @param category 日志分类
     */
    void critical(const QString &message, const QString &category = "General");

    /**
     * @brief 设置日志级别
     * 
     * @param level 最低记录的日志级别
     */
    void setLogLevel(LogLevel level);

    /**
     * @brief 设置控制台输出
     * 
     * @param enabled 是否启用控制台输出
     */
    void setConsoleOutput(bool enabled);

    /**
     * @brief 设置文件输出
     * 
     * @param enabled 是否启用文件输出
     */
    void setFileOutput(bool enabled);

    /**
     * @brief 设置最大日志文件数量
     * 
     * @param count 最大文件数量
     */
    void setMaxLogFiles(int count);

    /**
     * @brief 设置单个日志文件最大大小
     * 
     * @param size 最大文件大小（字节）
     */
    void setMaxFileSize(qint64 size);

signals:
    /**
     * @brief 日志消息信号
     * 
     * @param level 日志级别
     * @param message 日志消息
     * @param category 日志分类
     * @param timestamp 时间戳
     */
    void logMessage(LogLevel level, const QString &message, 
                   const QString &category, const QString &timestamp);

private:
    explicit Logger(QObject *parent = nullptr);
    ~Logger();

    // 禁用拷贝构造和赋值
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief 初始化日志目录
     */
    void initializeLogDirectory();

    /**
     * @brief 打开日志文件
     */
    void openLogFile();

    /**
     * @brief 输出到控制台
     * 
     * @param level 日志级别
     * @param message 格式化后的消息
     */
    void outputToConsole(LogLevel level, const QString &message);

    /**
     * @brief 输出到文件
     * 
     * @param message 格式化后的消息
     */
    void outputToFile(const QString &message);

    /**
     * @brief 轮转日志文件
     */
    void rotateLogFile();

    /**
     * @brief 清理旧日志文件
     */
    void cleanupOldLogFiles();

    /**
     * @brief 日志级别转字符串
     * 
     * @param level 日志级别
     * @return QString 级别字符串
     */
    QString logLevelToString(LogLevel level);

    // 单例相关
    static Logger* s_instance;
    static QMutex s_mutex;

    // 配置参数
    LogLevel m_logLevel;
    bool m_enableConsoleOutput;
    bool m_enableFileOutput;
    int m_maxLogFiles;
    qint64 m_maxFileSize;

    // 文件操作
    QString m_logDirectory;
    QFile m_logFile;
    QTextStream m_logStream;
    QMutex m_mutex;
};

// 全局日志函数
void logDebug(const QString &message, const QString &category = "General");
void logInfo(const QString &message, const QString &category = "General");
void logWarning(const QString &message, const QString &category = "General");
void logError(const QString &message, const QString &category = "General");
void logCritical(const QString &message, const QString &category = "General");

#endif // LOGGER_H