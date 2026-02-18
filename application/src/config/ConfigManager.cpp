/*
 * 文件名: ConfigManager.cpp
 * 说明: 配置管理器实现
 * 作者: 彭承康
 * 创建时间: 2025-07-24
 * 版本: v2.0.0
 * 
 * 更新内容:
 * - 完善错误处理机制
 * - 添加配置验证功能
 * - 优化性能和内存使用
 * - 增强日志记录
 */

#include "config/ConfigManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include <QMutexLocker>
#include <QTimer>
#include <QRegularExpression>

ConfigManager* ConfigManager::s_instance = nullptr;
QMutex ConfigManager::s_mutex;

ConfigManager* ConfigManager::instance()
{
    // 双重检查锁定模式，确保线程安全
    if (!s_instance) {
        QMutexLocker locker(&s_mutex);
        if (!s_instance) {
            s_instance = new ConfigManager();
        }
    }
    return s_instance;
}

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
    , m_settings(nullptr)
    , m_autoSaveTimer(nullptr)
    , m_autoSaveEnabled(true)
    , m_autoSaveInterval(5000) // 5秒自动保存
{
    // 设置配置文件路径
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    m_configFilePath = configDir + "/game_config.ini";
    
    // 创建QSettings对象
    m_settings = new QSettings(m_configFilePath, QSettings::IniFormat, this);
    
    // 初始化默认配置
    initializeDefaults();
    
    // 加载现有配置
    loadConfig();
    
    // 设置自动保存定时器
    setupAutoSave();
    
    // 验证配置完整性
    validateConfiguration();
    
    qDebug() << "ConfigManager: 初始化完成，配置文件:" << m_configFilePath;
}

ConfigManager::~ConfigManager()
{
    // 停止自动保存
    if (m_autoSaveTimer) {
        m_autoSaveTimer->stop();
    }
    
    // 保存配置
    saveConfig();
    
    qDebug() << "ConfigManager: 配置管理器销毁";
}

QVariant ConfigManager::getValue(const QString &key, const QVariant &defaultValue) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_settings) {
        qWarning() << "ConfigManager: Settings对象为空";
        return defaultValue;
    }
    
    QVariant value = m_settings->value(key, defaultValue);
    
    // 验证值的有效性
    if (!validateValue(key, value)) {
        qWarning() << "ConfigManager: 配置值无效，使用默认值:" << key;
        return m_defaults.value(key, defaultValue);
    }
    
    return value;
}

void ConfigManager::setValue(const QString &key, const QVariant &value)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_settings) {
        qWarning() << "ConfigManager: Settings对象为空";
        return;
    }
    
    // 验证值的有效性
    if (!validateValue(key, value)) {
        qWarning() << "ConfigManager: 尝试设置无效配置值:" << key << value;
        return;
    }
    
    QVariant oldValue = m_settings->value(key);
    if (oldValue != value) {
        m_settings->setValue(key, value);
        
        // 标记需要保存
        m_needsSave = true;
        
        // 发送变更信号
        emitConfigChanged(key, value);
        
        qDebug() << "ConfigManager: 配置已更新:" << key << "=" << value;
    }
}

bool ConfigManager::hasKey(const QString &key) const
{
    QMutexLocker locker(&m_mutex);
    return m_settings ? m_settings->contains(key) : false;
}

void ConfigManager::removeKey(const QString &key)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_settings) {
        return;
    }
    
    if (m_settings->contains(key)) {
        m_settings->remove(key);
        m_needsSave = true;
        emit configChanged(key, QVariant());
        qDebug() << "ConfigManager: 配置项已删除:" << key;
    }
}

// 图形设置实现
QSize ConfigManager::getResolution() const
{
    QSize resolution = getValue("Graphics/resolution", QSize(1920, 1080)).toSize();
    
    // 验证分辨率的合理性
    if (resolution.width() < 800 || resolution.height() < 600) {
        qWarning() << "ConfigManager: 分辨率过小，使用默认值";
        return QSize(1920, 1080);
    }
    
    return resolution;
}

void ConfigManager::setResolution(const QSize &resolution)
{
    if (resolution.width() < 800 || resolution.height() < 600) {
        qWarning() << "ConfigManager: 分辨率过小，拒绝设置:" << resolution;
        return;
    }
    
    setValue("Graphics/resolution", resolution);
    emit resolutionChanged(resolution);
}

bool ConfigManager::isFullscreen() const
{
    return getValue("Graphics/fullscreen", false).toBool();
}

void ConfigManager::setFullscreen(bool fullscreen)
{
    setValue("Graphics/fullscreen", fullscreen);
    emit fullscreenChanged(fullscreen);
}

int ConfigManager::getGraphicsQuality() const
{
    return getValue("Graphics/quality", 2).toInt();
}

void ConfigManager::setGraphicsQuality(int quality)
{
    quality = qBound(0, quality, 3);
    setValue("Graphics/quality", quality);
    emit graphicsQualityChanged(quality);
}

bool ConfigManager::isVSyncEnabled() const
{
    return getValue("Graphics/vsync", true).toBool();
}

void ConfigManager::setVSyncEnabled(bool enabled)
{
    setValue("Graphics/vsync", enabled);
    emit vSyncChanged(enabled);
}

// 音频设置实现
float ConfigManager::getMasterVolume() const
{
    return qBound(0.0f, getValue("Audio/master_volume", 1.0f).toFloat(), 1.0f);
}

void ConfigManager::setMasterVolume(float volume)
{
    volume = qBound(0.0f, volume, 1.0f);
    setValue("Audio/master_volume", volume);
    emit masterVolumeChanged(volume);
}

float ConfigManager::getSFXVolume() const
{
    return qBound(0.0f, getValue("Audio/sfx_volume", 0.8f).toFloat(), 1.0f);
}

void ConfigManager::setSFXVolume(float volume)
{
    volume = qBound(0.0f, volume, 1.0f);
    setValue("Audio/sfx_volume", volume);
    emit sfxVolumeChanged(volume);
}

float ConfigManager::getMusicVolume() const
{
    return qBound(0.0f, getValue("Audio/music_volume", 0.7f).toFloat(), 1.0f);
}

void ConfigManager::setMusicVolume(float volume)
{
    volume = qBound(0.0f, volume, 1.0f);
    setValue("Audio/music_volume", volume);
    emit musicVolumeChanged(volume);
}

bool ConfigManager::isMuted() const
{
    return getValue("Audio/muted", false).toBool();
}

void ConfigManager::setMuted(bool muted)
{
    setValue("Audio/muted", muted);
    emit mutedChanged(muted);
}

// 游戏设置实现
QString ConfigManager::getLanguage() const
{
    QString language = getValue("Gameplay/language", "zh_CN").toString();
    
    // 验证语言代码
    QStringList supportedLanguages = {"zh_CN", "en_US", "ja_JP", "ko_KR"};
    if (!supportedLanguages.contains(language)) {
        qWarning() << "ConfigManager: 不支持的语言:" << language;
        return "zh_CN";
    }
    
    return language;
}

void ConfigManager::setLanguage(const QString &language)
{
    QStringList supportedLanguages = {"zh_CN", "en_US", "ja_JP", "ko_KR"};
    if (!supportedLanguages.contains(language)) {
        qWarning() << "ConfigManager: 不支持的语言:" << language;
        return;
    }
    
    setValue("Gameplay/language", language);
    emit languageChanged(language);
}

int ConfigManager::getDifficulty() const
{
    return qBound(0, getValue("Gameplay/difficulty", 1).toInt(), 2);
}

void ConfigManager::setDifficulty(int difficulty)
{
    difficulty = qBound(0, difficulty, 2);
    setValue("Gameplay/difficulty", difficulty);
    emit difficultyChanged(difficulty);
}

bool ConfigManager::isAutoSaveEnabled() const
{
    return getValue("Gameplay/auto_save", true).toBool();
}

void ConfigManager::setAutoSaveEnabled(bool enabled)
{
    setValue("Gameplay/auto_save", enabled);
    emit autoSaveChanged(enabled);
}

int ConfigManager::getAutoSaveInterval() const
{
    return qMax(1, getValue("Gameplay/auto_save_interval", 5).toInt());
}

void ConfigManager::setAutoSaveInterval(int minutes)
{
    minutes = qMax(1, minutes);
    setValue("Gameplay/auto_save_interval", minutes);
    emit autoSaveIntervalChanged(minutes);
}

bool ConfigManager::loadConfig(const QString &filePath)
{
    QMutexLocker locker(&m_mutex);
    
    if (!filePath.isEmpty()) {
        m_configFilePath = filePath;
        delete m_settings;
        m_settings = new QSettings(m_configFilePath, QSettings::IniFormat, this);
    }
    
    if (!m_settings) {
        qCritical() << "ConfigManager: 无法创建Settings对象";
        return false;
    }
    
    // 检查配置文件是否存在
    QFile configFile(m_configFilePath);
    if (!configFile.exists()) {
        qDebug() << "ConfigManager: 配置文件不存在，将创建默认配置";
        createDefaultConfig();
        return true;
    }
    
    // 同步设置
    m_settings->sync();
    
    // 验证配置完整性
    if (!validateConfiguration()) {
        qWarning() << "ConfigManager: 配置验证失败，部分设置将使用默认值";
    }
    
    qDebug() << "ConfigManager: 配置加载完成";
    return true;
}

bool ConfigManager::saveConfig(const QString &filePath)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_settings) {
        qCritical() << "ConfigManager: Settings对象为空，无法保存";
        return false;
    }
    
    if (!filePath.isEmpty()) {
        m_configFilePath = filePath;
    }
    
    // 同步到文件
    m_settings->sync();
    
    // 检查是否保存成功
    if (m_settings->status() != QSettings::NoError) {
        qCritical() << "ConfigManager: 配置保存失败，状态:" << m_settings->status();
        return false;
    }
    
    m_needsSave = false;
    qDebug() << "ConfigManager: 配置保存完成";
    return true;
}

void ConfigManager::resetToDefaults(ConfigGroup group)
{
    QMutexLocker locker(&m_mutex);
    
    QString prefix = (group == static_cast<ConfigGroup>(-1)) ? "" : getGroupPrefix(group);
    
    QStringList keysToReset;
    for (auto it = m_defaults.begin(); it != m_defaults.end(); ++it) {
        if (prefix.isEmpty() || it.key().startsWith(prefix)) {
            keysToReset.append(it.key());
        }
    }
    
    // 重置配置值
    for (const QString& key : keysToReset) {
        m_settings->setValue(key, m_defaults[key]);
    }
    
    m_needsSave = true;
    emit configReset(group);
    
    // 发送所有相关的变更信号
    for (const QString& key : keysToReset) {
        emitConfigChanged(key, m_defaults[key]);
    }
    
    qDebug() << "ConfigManager: 重置配置完成，组:" << static_cast<int>(group);
}

QString ConfigManager::getConfigFilePath() const
{
    return m_configFilePath;
}

QStringList ConfigManager::getAllKeys(ConfigGroup group) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_settings) {
        return QStringList();
    }
    
    QStringList keys = m_settings->allKeys();
    
    if (group != static_cast<ConfigGroup>(-1)) {
        QString prefix = getGroupPrefix(group);
        keys = keys.filter(QRegularExpression("^" + QRegularExpression::escape(prefix)));
    }
    
    return keys;
}

void ConfigManager::initializeDefaults()
{
    // 图形设置默认值
    m_defaults["Graphics/resolution"] = QSize(1920, 1080);
    m_defaults["Graphics/fullscreen"] = false;
    m_defaults["Graphics/quality"] = 2;
    m_defaults["Graphics/vsync"] = true;
    m_defaults["Graphics/anti_aliasing"] = true;
    m_defaults["Graphics/texture_quality"] = 2;
    m_defaults["Graphics/shadow_quality"] = 2;
    m_defaults["Graphics/effect_quality"] = 2;
    
    // 音频设置默认值
    m_defaults["Audio/master_volume"] = 1.0f;
    m_defaults["Audio/sfx_volume"] = 0.8f;
    m_defaults["Audio/music_volume"] = 0.7f;
    m_defaults["Audio/voice_volume"] = 0.9f;
    m_defaults["Audio/muted"] = false;
    m_defaults["Audio/audio_device"] = "default";
    
    // 游戏设置默认值
    m_defaults["Gameplay/language"] = "zh_CN";
    m_defaults["Gameplay/difficulty"] = 1;
    m_defaults["Gameplay/auto_save"] = true;
    m_defaults["Gameplay/auto_save_interval"] = 5;
    m_defaults["Gameplay/show_tutorials"] = true;
    m_defaults["Gameplay/combat_speed"] = 1.0f;
    
    // 控制设置默认值
    m_defaults["Controls/mouse_sensitivity"] = 1.0f;
    m_defaults["Controls/invert_mouse"] = false;
    m_defaults["Controls/key_move_up"] = static_cast<int>(Qt::Key_W);
    m_defaults["Controls/key_move_down"] = static_cast<int>(Qt::Key_S);
    m_defaults["Controls/key_move_left"] = static_cast<int>(Qt::Key_A);
    m_defaults["Controls/key_move_right"] = static_cast<int>(Qt::Key_D);
    m_defaults["Controls/key_attack"] = static_cast<int>(Qt::Key_Space);
    m_defaults["Controls/key_inventory"] = static_cast<int>(Qt::Key_I);
    
    // 系统设置默认值
    m_defaults["System/show_fps"] = false;
    m_defaults["System/enable_logging"] = true;
    m_defaults["System/log_level"] = 2; // Info level
    m_defaults["System/performance_monitoring"] = false;
    m_defaults["System/auto_update"] = true;
    
    qDebug() << "ConfigManager: 默认配置初始化完成，项目数:" << m_defaults.size();
}

QString ConfigManager::getGroupPrefix(ConfigGroup group) const
{
    switch (group) {
        case ConfigGroup::Graphics: return "Graphics/";
        case ConfigGroup::Audio: return "Audio/";
        case ConfigGroup::Controls: return "Controls/";
        case ConfigGroup::Gameplay: return "Gameplay/";
        case ConfigGroup::System: return "System/";
        default: return "";
    }
}

void ConfigManager::emitConfigChanged(const QString &key, const QVariant &value)
{
    emit configChanged(key, value);
    
    // 发送特定的信号
    if (key == "Graphics/resolution") {
        emit resolutionChanged(value.toSize());
    } else if (key == "Graphics/fullscreen") {
        emit fullscreenChanged(value.toBool());
    } else if (key == "Graphics/quality") {
        emit graphicsQualityChanged(value.toInt());
    } else if (key == "Graphics/vsync") {
        emit vSyncChanged(value.toBool());
    } else if (key == "Audio/master_volume") {
        emit masterVolumeChanged(value.toFloat());
    } else if (key == "Audio/sfx_volume") {
        emit sfxVolumeChanged(value.toFloat());
    } else if (key == "Audio/music_volume") {
        emit musicVolumeChanged(value.toFloat());
    } else if (key == "Audio/muted") {
        emit mutedChanged(value.toBool());
    } else if (key == "Gameplay/language") {
        emit languageChanged(value.toString());
    } else if (key == "Gameplay/difficulty") {
        emit difficultyChanged(value.toInt());
    } else if (key == "Gameplay/auto_save") {
        emit autoSaveChanged(value.toBool());
    } else if (key == "Gameplay/auto_save_interval") {
        emit autoSaveIntervalChanged(value.toInt());
    }
}

void ConfigManager::setupAutoSave()
{
    if (!m_autoSaveEnabled) {
        return;
    }
    
    m_autoSaveTimer = new QTimer(this);
    m_autoSaveTimer->setSingleShot(false);
    m_autoSaveTimer->setInterval(m_autoSaveInterval);
    
    connect(m_autoSaveTimer, &QTimer::timeout, this, [this]() {
        if (m_needsSave) {
            saveConfig();
        }
    });
    
    m_autoSaveTimer->start();
    qDebug() << "ConfigManager: 自动保存已启用，间隔:" << m_autoSaveInterval << "ms";
}

bool ConfigManager::validateConfiguration()
{
    bool isValid = true;
    
    // 验证关键配置项
    QStringList criticalKeys = {
        "Graphics/resolution",
        "Audio/master_volume",
        "Gameplay/language"
    };
    
    for (const QString& key : criticalKeys) {
        if (!hasKey(key)) {
            qWarning() << "ConfigManager: 缺少关键配置项:" << key;
            setValue(key, m_defaults[key]);
            isValid = false;
        }
    }
    
    return isValid;
}

bool ConfigManager::validateValue(const QString &key, const QVariant &value) const
{
    // 根据键名验证值的有效性
    if (key.startsWith("Audio/") && key.endsWith("_volume")) {
        float vol = value.toFloat();
        return vol >= 0.0f && vol <= 1.0f;
    }
    
    if (key == "Graphics/quality") {
        int quality = value.toInt();
        return quality >= 0 && quality <= 3;
    }
    
    if (key == "Gameplay/difficulty") {
        int difficulty = value.toInt();
        return difficulty >= 0 && difficulty <= 2;
    }
    
    if (key == "Graphics/resolution") {
        QSize size = value.toSize();
        return size.width() >= 800 && size.height() >= 600;
    }
    
    // 默认认为有效
    return true;
}

void ConfigManager::createDefaultConfig()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_settings) {
        return;
    }
    
    // 写入所有默认值
    for (auto it = m_defaults.begin(); it != m_defaults.end(); ++it) {
        m_settings->setValue(it.key(), it.value());
    }
    
    m_settings->sync();
    m_needsSave = false;
    
    qDebug() << "ConfigManager: 默认配置文件已创建";
}
