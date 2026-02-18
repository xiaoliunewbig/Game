/*
 * 文件名: ConfigManager.h
 * 说明: 配置管理器头文件
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 * 
 * 本文件定义了游戏配置管理系统，负责游戏设置的加载、保存和管理。
 * 
 * 主要功能：
 * - 游戏设置的持久化存储
 * - 配置文件的读写操作
 * - 运行时配置的动态修改
 * - 配置变更的通知机制
 * - 默认配置的管理
 * 
 * 支持的配置类型：
 * - 图形设置（分辨率、画质、特效）
 * - 音频设置（音量、音效开关）
 * - 控制设置（键位绑定、鼠标灵敏度）
 * - 游戏设置（难度、语言、自动保存）
 */

#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QSettings>
#include <QVariant>
#include <QSize>
#include <QStringList>
#include <QMutex>
#include <QTimer>

/**
 * @brief 配置管理器类
 * 
 * 单例模式的配置管理器，提供全局统一的配置访问接口。
 * 使用QSettings作为底层存储，支持跨平台的配置持久化。
 */
class ConfigManager : public QObject
{
    Q_OBJECT
    
    // Qt属性系统，支持QML绑定
    Q_PROPERTY(QString language READ getLanguage WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(bool fullscreen READ isFullscreen WRITE setFullscreen NOTIFY fullscreenChanged)
    Q_PROPERTY(float masterVolume READ getMasterVolume WRITE setMasterVolume NOTIFY masterVolumeChanged)
    
public:
    /**
     * @brief 配置分组枚举
     * 
     * 将配置按功能分组，便于管理和查找
     */
    enum class ConfigGroup {
        Graphics,    ///< 图形设置
        Audio,       ///< 音频设置
        Controls,    ///< 控制设置
        Gameplay,    ///< 游戏玩法设置
        System       ///< 系统设置
    };
    
    /**
     * @brief 获取单例实例
     * @return ConfigManager的唯一实例
     */
    static ConfigManager* instance();
    
    /**
     * @brief 析构函数
     * 
     * 确保配置在程序退出时正确保存
     */
    ~ConfigManager();
    
    // ==================== 基础配置接口 ====================
    
    /**
     * @brief 获取配置值
     * @param key 配置键名
     * @param defaultValue 默认值
     * @return 配置值，如果不存在则返回默认值
     */
    QVariant getValue(const QString &key, const QVariant &defaultValue = QVariant()) const;
    
    /**
     * @brief 设置配置值
     * @param key 配置键名
     * @param value 配置值
     */
    void setValue(const QString &key, const QVariant &value);
    
    /**
     * @brief 检查配置键是否存在
     * @param key 配置键名
     * @return 存在返回true，否则返回false
     */
    bool hasKey(const QString &key) const;
    
    /**
     * @brief 移除配置项
     * @param key 配置键名
     */
    void removeKey(const QString &key);
    
    // ==================== 图形设置接口 ====================
    
    /**
     * @brief 获取屏幕分辨率
     * @return 分辨率大小
     */
    QSize getResolution() const;
    
    /**
     * @brief 设置屏幕分辨率
     * @param resolution 新的分辨率
     */
    void setResolution(const QSize &resolution);
    
    /**
     * @brief 获取是否全屏模式
     * @return 全屏返回true，窗口模式返回false
     */
    bool isFullscreen() const;
    
    /**
     * @brief 设置全屏模式
     * @param fullscreen 是否全屏
     */
    void setFullscreen(bool fullscreen);
    
    /**
     * @brief 获取画质等级
     * @return 画质等级（0-3，0最低，3最高）
     */
    int getGraphicsQuality() const;
    
    /**
     * @brief 设置画质等级
     * @param quality 画质等级
     */
    void setGraphicsQuality(int quality);
    
    /**
     * @brief 获取是否启用垂直同步
     * @return 启用返回true，否则返回false
     */
    bool isVSyncEnabled() const;
    
    /**
     * @brief 设置垂直同步
     * @param enabled 是否启用
     */
    void setVSyncEnabled(bool enabled);
    
    // ==================== 音频设置接口 ====================
    
    /**
     * @brief 获取主音量
     * @return 音量值（0.0-1.0）
     */
    float getMasterVolume() const;
    
    /**
     * @brief 设置主音量
     * @param volume 音量值
     */
    void setMasterVolume(float volume);
    
    /**
     * @brief 获取音效音量
     * @return 音效音量（0.0-1.0）
     */
    float getSFXVolume() const;
    
    /**
     * @brief 设置音效音量
     * @param volume 音效音量
     */
    void setSFXVolume(float volume);
    
    /**
     * @brief 获取背景音乐音量
     * @return 背景音乐音量（0.0-1.0）
     */
    float getMusicVolume() const;
    
    /**
     * @brief 设置背景音乐音量
     * @param volume 背景音乐音量
     */
    void setMusicVolume(float volume);
    
    /**
     * @brief 获取是否静音
     * @return 静音返回true，否则返回false
     */
    bool isMuted() const;
    
    /**
     * @brief 设置静音状态
     * @param muted 是否静音
     */
    void setMuted(bool muted);
    
    // ==================== 游戏设置接口 ====================
    
    /**
     * @brief 获取游戏语言
     * @return 语言代码（如"zh_CN", "en_US"）
     */
    QString getLanguage() const;
    
    /**
     * @brief 设置游戏语言
     * @param language 语言代码
     */
    void setLanguage(const QString &language);
    
    /**
     * @brief 获取游戏难度
     * @return 难度等级（0-2，0简单，1普通，2困难）
     */
    int getDifficulty() const;
    
    /**
     * @brief 设置游戏难度
     * @param difficulty 难度等级
     */
    void setDifficulty(int difficulty);
    
    /**
     * @brief 获取是否启用自动保存
     * @return 启用返回true，否则返回false
     */
    bool isAutoSaveEnabled() const;
    
    /**
     * @brief 设置自动保存
     * @param enabled 是否启用
     */
    void setAutoSaveEnabled(bool enabled);
    
    /**
     * @brief 获取自动保存间隔（分钟）
     * @return 自动保存间隔
     */
    int getAutoSaveInterval() const;
    
    /**
     * @brief 设置自动保存间隔
     * @param minutes 间隔分钟数
     */
    void setAutoSaveInterval(int minutes);
    
    // ==================== 配置管理接口 ====================
    
    /**
     * @brief 加载配置文件
     * @param filePath 配置文件路径，为空则使用默认路径
     * @return 加载成功返回true，否则返回false
     */
    bool loadConfig(const QString &filePath = QString());
    
    /**
     * @brief 保存配置文件
     * @param filePath 配置文件路径，为空则使用默认路径
     * @return 保存成功返回true，否则返回false
     */
    bool saveConfig(const QString &filePath = QString());
    
    /**
     * @brief 重置为默认配置
     * @param group 要重置的配置组，为空则重置所有
     */
    void resetToDefaults(ConfigGroup group = static_cast<ConfigGroup>(-1));
    
    /**
     * @brief 获取配置文件路径
     * @return 当前使用的配置文件路径
     */
    QString getConfigFilePath() const;
    
    /**
     * @brief 获取所有配置键
     * @param group 配置组，为空则返回所有键
     * @return 配置键列表
     */
    QStringList getAllKeys(ConfigGroup group = static_cast<ConfigGroup>(-1)) const;

signals:
    // ==================== 配置变更信号 ====================
    
    /**
     * @brief 配置值变更信号
     * @param key 变更的配置键
     * @param value 新的配置值
     */
    void configChanged(const QString &key, const QVariant &value);
    
    // 图形设置变更信号
    void resolutionChanged(const QSize &resolution);
    void fullscreenChanged(bool fullscreen);
    void graphicsQualityChanged(int quality);
    void vSyncChanged(bool enabled);
    
    // 音频设置变更信号
    void masterVolumeChanged(float volume);
    void sfxVolumeChanged(float volume);
    void musicVolumeChanged(float volume);
    void mutedChanged(bool muted);
    
    // 游戏设置变更信号
    void languageChanged(const QString &language);
    void difficultyChanged(int difficulty);
    void autoSaveChanged(bool enabled);
    void autoSaveIntervalChanged(int minutes);
    
    /**
     * @brief 配置重置信号
     * @param group 被重置的配置组
     */
    void configReset(ConfigGroup group);

private:
    /**
     * @brief 私有构造函数（单例模式）
     * @param parent 父对象指针
     */
    explicit ConfigManager(QObject *parent = nullptr);
    
    /**
     * @brief 初始化默认配置
     */
    void initializeDefaults();
    
    /**
     * @brief 获取配置组前缀
     * @param group 配置组
     * @return 组前缀字符串
     */
    QString getGroupPrefix(ConfigGroup group) const;
    
    /**
     * @brief 发送配置变更信号
     * @param key 配置键
     * @param value 新值
     */
    void emitConfigChanged(const QString &key, const QVariant &value);
    void setupAutoSave();
    bool validateConfiguration();
    bool validateValue(const QString &key, const QVariant &value) const;
    void createDefaultConfig();

private:
    static ConfigManager* s_instance;  ///< 单例实例
    QSettings* m_settings;             ///< Qt设置对象
    QString m_configFilePath;          ///< 配置文件路径
    QMap<QString, QVariant> m_defaults; ///< 默认配置值
    static QMutex s_mutex;              ///< 静态互斥锁
    mutable QMutex m_mutex;             ///< 实例互斥锁
    QTimer* m_autoSaveTimer;            ///< 自动保存定时器
    bool m_autoSaveEnabled;             ///< 是否启用自动保存
    int m_autoSaveInterval;             ///< 自动保存间隔(ms)
    bool m_needsSave;                   ///< 是否有待保存的变更
};

#endif // CONFIGMANAGER_H