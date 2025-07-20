/*
 * 文件名: GameConfig.cpp
 * 说明: 游戏配置管理器实现
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 本文件实现游戏配置的加载、保存和管理功能。
 */

#include "config/GameConfig.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

GameConfig::GameConfig(QObject *parent)
    : QObject(parent)
    , m_audioVolume(1.0f)
    , m_musicVolume(0.8f)
    , m_effectVolume(1.0f)
    , m_fullscreen(false)
    , m_vsync(true)
    , m_showFPS(false)
    , m_language("zh_CN")
{
    loadConfig();
}

GameConfig::~GameConfig()
{
    saveConfig();
}

bool GameConfig::loadConfig()
{
    QString configPath = getConfigPath();
    QFile file(configPath);
    
    if (!file.exists()) {
        qDebug() << "GameConfig: 配置文件不存在，使用默认配置";
        return createDefaultConfig();
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "GameConfig: 无法打开配置文件:" << configPath;
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "GameConfig: JSON解析错误:" << error.errorString();
        return false;
    }
    
    QJsonObject config = doc.object();
    
    // 加载音频设置
    QJsonObject audio = config["audio"].toObject();
    m_audioVolume = audio["volume"].toDouble(1.0);
    m_musicVolume = audio["music"].toDouble(0.8);
    m_effectVolume = audio["effects"].toDouble(1.0);
    
    // 加载图形设置
    QJsonObject graphics = config["graphics"].toObject();
    m_fullscreen = graphics["fullscreen"].toBool(false);
    m_vsync = graphics["vsync"].toBool(true);
    m_showFPS = graphics["showFPS"].toBool(false);
    
    // 加载游戏设置
    QJsonObject game = config["game"].toObject();
    m_language = game["language"].toString("zh_CN");
    
    // 加载键位绑定
    QJsonObject controls = config["controls"].toObject();
    m_keyBindings.clear();
    for (auto it = controls.begin(); it != controls.end(); ++it) {
        m_keyBindings[it.key()] = it.value().toInt();
    }
    
    qDebug() << "GameConfig: 配置加载成功";
    return true;
}

bool GameConfig::saveConfig()
{
    QString configPath = getConfigPath();
    QDir configDir = QFileInfo(configPath).dir();
    
    if (!configDir.exists()) {
        configDir.mkpath(".");
    }
    
    QJsonObject config;
    
    // 保存音频设置
    QJsonObject audio;
    audio["volume"] = m_audioVolume;
    audio["music"] = m_musicVolume;
    audio["effects"] = m_effectVolume;
    config["audio"] = audio;
    
    // 保存图形设置
    QJsonObject graphics;
    graphics["fullscreen"] = m_fullscreen;
    graphics["vsync"] = m_vsync;
    graphics["showFPS"] = m_showFPS;
    config["graphics"] = graphics;
    
    // 保存游戏设置
    QJsonObject game;
    game["language"] = m_language;
    config["game"] = game;
    
    // 保存键位绑定
    QJsonObject controls;
    for (auto it = m_keyBindings.begin(); it != m_keyBindings.end(); ++it) {
        controls[it.key()] = it.value();
    }
    config["controls"] = controls;
    
    QJsonDocument doc(config);
    QFile file(configPath);
    
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "GameConfig: 无法写入配置文件:" << configPath;
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    
    qDebug() << "GameConfig: 配置保存成功";
    return true;
}

bool GameConfig::createDefaultConfig()
{
    // 设置默认键位绑定
    m_keyBindings = {
        {"move_up", Qt::Key_W},
        {"move_down", Qt::Key_S},
        {"move_left", Qt::Key_A},
        {"move_right", Qt::Key_D},
        {"attack", Qt::Key_Space},
        {"skill_1", Qt::Key_1},
        {"skill_2", Qt::Key_2},
        {"skill_3", Qt::Key_3},
        {"inventory", Qt::Key_I},
        {"menu", Qt::Key_Escape}
    };
    
    return saveConfig();
}

QString GameConfig::getConfigPath() const
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    return configDir + "/FantasyLegend/config.json";
}

// Getter方法实现
float GameConfig::audioVolume() const { return m_audioVolume; }
float GameConfig::musicVolume() const { return m_musicVolume; }
float GameConfig::effectVolume() const { return m_effectVolume; }
bool GameConfig::fullscreen() const { return m_fullscreen; }
bool GameConfig::vsync() const { return m_vsync; }
bool GameConfig::showFPS() const { return m_showFPS; }
QString GameConfig::language() const { return m_language; }
QMap<QString, int> GameConfig::keyBindings() const { return m_keyBindings; }

// Setter方法实现
void GameConfig::setAudioVolume(float volume)
{
    if (qFuzzyCompare(m_audioVolume, volume)) return;
    m_audioVolume = qBound(0.0f, volume, 1.0f);
    emit audioVolumeChanged();
}

void GameConfig::setMusicVolume(float volume)
{
    if (qFuzzyCompare(m_musicVolume, volume)) return;
    m_musicVolume = qBound(0.0f, volume, 1.0f);
    emit musicVolumeChanged();
}

void GameConfig::setEffectVolume(float volume)
{
    if (qFuzzyCompare(m_effectVolume, volume)) return;
    m_effectVolume = qBound(0.0f, volume, 1.0f);
    emit effectVolumeChanged();
}

void GameConfig::setFullscreen(bool fullscreen)
{
    if (m_fullscreen == fullscreen) return;
    m_fullscreen = fullscreen;
    emit fullscreenChanged();
}

void GameConfig::setVsync(bool vsync)
{
    if (m_vsync == vsync) return;
    m_vsync = vsync;
    emit vsyncChanged();
}

void GameConfig::setShowFPS(bool showFPS)
{
    if (m_showFPS == showFPS) return;
    m_showFPS = showFPS;
    emit showFPSChanged();
}

void GameConfig::setLanguage(const QString &language)
{
    if (m_language == language) return;
    m_language = language;
    emit languageChanged();
}

void GameConfig::setKeyBinding(const QString &action, int key)
{
    if (m_keyBindings.value(action) == key) return;
    m_keyBindings[action] = key;
    emit keyBindingsChanged();
}

int GameConfig::getKeyBinding(const QString &action) const
{
    return m_keyBindings.value(action, -1);
}