/*
 * 文件名: GameConfig.h
 * 说明: 游戏配置管理器头文件
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */

#ifndef GAMECONFIG_H
#define GAMECONFIG_H

#include <QObject>
#include <QMap>
#include <QString>

/**
 * @brief 游戏配置管理器
 * 
 * 负责管理游戏的各种配置选项，包括：
 * - 音频设置（音量、音效等）
 * - 图形设置（全屏、垂直同步等）
 * - 游戏设置（语言、难度等）
 * - 键位绑定
 */
class GameConfig : public QObject
{
    Q_OBJECT
    
    // 音频属性
    Q_PROPERTY(float audioVolume READ audioVolume WRITE setAudioVolume NOTIFY audioVolumeChanged)
    Q_PROPERTY(float musicVolume READ musicVolume WRITE setMusicVolume NOTIFY musicVolumeChanged)
    Q_PROPERTY(float effectVolume READ effectVolume WRITE setEffectVolume NOTIFY effectVolumeChanged)
    
    // 图形属性
    Q_PROPERTY(bool fullscreen READ fullscreen WRITE setFullscreen NOTIFY fullscreenChanged)
    Q_PROPERTY(bool vsync READ vsync WRITE setVsync NOTIFY vsyncChanged)
    Q_PROPERTY(bool showFPS READ showFPS WRITE setShowFPS NOTIFY showFPSChanged)
    
    // 游戏属性
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QMap<QString, int> keyBindings READ keyBindings NOTIFY keyBindingsChanged)

public:
    explicit GameConfig(QObject *parent = nullptr);
    ~GameConfig();
    
    // 配置文件操作
    bool loadConfig();
    bool saveConfig();
    
    // Getter方法
    float audioVolume() const;
    float musicVolume() const;
    float effectVolume() const;
    bool fullscreen() const;
    bool vsync() const;
    bool showFPS() const;
    QString language() const;
    QMap<QString, int> keyBindings() const;
    
    // 键位绑定相关
    Q_INVOKABLE int getKeyBinding(const QString &action) const;
    Q_INVOKABLE void setKeyBinding(const QString &action, int key);

public slots:
    // Setter方法
    void setAudioVolume(float volume);
    void setMusicVolume(float volume);
    void setEffectVolume(float volume);
    void setFullscreen(bool fullscreen);
    void setVsync(bool vsync);
    void setShowFPS(bool showFPS);
    void setLanguage(const QString &language);

signals:
    // 属性变更信号
    void audioVolumeChanged();
    void musicVolumeChanged();
    void effectVolumeChanged();
    void fullscreenChanged();
    void vsyncChanged();
    void showFPSChanged();
    void languageChanged();
    void keyBindingsChanged();

private:
    // 私有方法
    bool createDefaultConfig();
    QString getConfigPath() const;
    
    // 音频设置
    float m_audioVolume;
    float m_musicVolume;
    float m_effectVolume;
    
    // 图形设置
    bool m_fullscreen;
    bool m_vsync;
    bool m_showFPS;
    
    // 游戏设置
    QString m_language;
    
    // 键位绑定
    QMap<QString, int> m_keyBindings;
};

#endif // GAMECONFIG_H