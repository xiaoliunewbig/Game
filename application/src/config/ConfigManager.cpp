/*
 * 文件名: ConfigManager.cpp
 * 说明: 配置管理器实现
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 */

#include "config/ConfigManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>

ConfigManager* ConfigManager::s_instance = nullptr;

ConfigManager* ConfigManager::instance()
{
    if (!s_instance) {
        s_instance = new ConfigManager();
    }
    return s_instance;
}

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
    , m_settings(nullptr)
{
    // 设置配置文件路径
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    m_configFilePath = configDir + "/game_config.ini";
    
    // 创建QSettings对象
    m_settings = new QSettings(m_configFilePath, QSettings::IniFormat, this);
    m_settings->setIniCodec("UTF-8");
    
    // 初始化默认配置
    initializeDefaults();
    
    // 加载现有配置
    loadConfig();
    
    qDebug() << "ConfigManager: 初始化完成，配置文件:" << m_configFilePath;
}

ConfigManager::~ConfigManager()
{
    saveConfig();
}

QVariant ConfigManager::getValue(const QString &key, const QVariant &defaultValue) const
{
    return m_settings->value(key, defaultValue);
}

void ConfigManager::setValue(const QString &key, const QVariant &value)
{
    QVariant oldValue = m_settings->value(key);
    if (oldValue != value) {
        m_settings->setValue(key, value);
        emitConfigChanged(key, value);
    }
}

bool ConfigManager::hasKey(const QString &key) const
{
    return m_settings->contains(key);
}

void ConfigManager::removeKey(const QString &key)
{
    if (m_settings->contains(key)) {
        m_settings->remove(key);
        emit configChanged(key, QVariant());
    }
}

// 图形设置实现
QSize ConfigManager::getResolution() const
{
    return getValue("Graphics/resolution", QSize(1920, 1080)).toSize();
}

void ConfigManager::setResolution(const QSize &resolution)
{
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
    return getValue("Audio/master_volume", 1.0f).toFloat();
}

void ConfigManager::setMasterVolume(float volume)
{
    volume = qBound(0.0f, volume, 1.0f);
    setValue("Audio/master_volume", volume);
    emit masterVolumeChanged(volume);
}

float ConfigManager::getSFXVolume() const
{
    return getValue("Audio/sfx_volume", 0.8f).toFloat();
}

void ConfigManager::setSFXVolume(float volume)
{
    volume = qBound(0.0f, volume, 1.0f);
    setValue("Audio/sfx_volume", volume);
    emit sfxVolumeChanged(volume);
}

float ConfigManager::getMusicVolume() const
{
    return getValue("Audio/music_volume", 0.7f).toFloat();
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
    return getValue("Gameplay/language", "zh_CN").toString();
}

void ConfigManager::setLanguage(const QString &language)
{
    setValue("Gameplay/language", language);
    emit languageChanged(language);
}

int ConfigManager::getDifficulty() const
{
    return getValue("Gameplay/difficulty", 1).toInt();
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
    return getValue("Gameplay/auto_save_interval", 5).toInt();
}

void ConfigManager::setAutoSaveInterval(int minutes)
{
    minutes = qMax(1, minutes);
    setValue("Gameplay/auto_save_interval", minutes);
    emit autoSaveIntervalChanged(minutes);
}

bool ConfigManager::loadConfig(const QString &filePath)
{
    if (!filePath.isEmpty()) {
        m_configFilePath = filePath;
        delete m_settings;
        m_settings = new QSettings(m_configFilePath, QSettings::IniFormat, this);
        m_settings->setIniCodec("UTF-8");
    }
    
    m_settings->sync();
    qDebug() << "ConfigManager: 配置加载完成";
    return true;
}

bool ConfigManager::saveConfig(const QString &filePath)
{
    if (!filePath.isEmpty()) {
        m_configFilePath = filePath;
    }
    
    m_settings->sync();
    qDebug() << "ConfigManager: 配置保存完成";
    return true;
}

void ConfigManager::resetToDefaults(ConfigGroup group)
{
    QString prefix = (group == static_cast<ConfigGroup>(-1)) ? "" : getGroupPrefix(group);
    
    for (auto it = m_defaults.begin(); it != m_defaults.end(); ++it) {
        if (prefix.isEmpty() || it.key().startsWith(prefix)) {
            setValue(it.key(), it.value());
        }
    }
    
    emit configReset(group);
    qDebug() << "ConfigManager: 重置配置完成";
}

QString ConfigManager::getConfigFilePath() const
{
    return m_configFilePath;
}

QStringList ConfigManager::getAllKeys(ConfigGroup group) const
{
    QStringList keys = m_settings->allKeys();
    
    if (group != static_cast<ConfigGroup>(-1)) {
        QString prefix = getGroupPrefix(group);
        keys = keys.filter(QRegExp("^" + prefix));
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
    
    // 音频设置默认值
    m_defaults["Audio/master_volume"] = 1.0f;
    m_defaults["Audio/sfx_volume"] = 0.8f;
    m_defaults["Audio/music_volume"] = 0.7f;
    m_defaults["Audio/muted"] = false;
    
    // 游戏设置默认值
    m_defaults["Gameplay/language"] = "zh_CN";
    m_defaults["Gameplay/difficulty"] = 1;
    m_defaults["Gameplay/auto_save"] = true;
    m_defaults["Gameplay/auto_save_interval"] = 5;
    
    // 控制设置默认值
    m_defaults["Controls/mouse_sensitivity"] = 1.0f;
    m_defaults["Controls/invert_mouse"] = false;
    
    // 系统设置默认值
    m_defaults["System/show_fps"] = false;
    m_defaults["System/enable_logging"] = true;
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
    } else if (key == "Audio/master_volume") {
        emit masterVolumeChanged(value.toFloat());
    } else if (key == "Gameplay/language") {
        emit languageChanged(value.toString());
    }
    // ... 其他特定信号
}