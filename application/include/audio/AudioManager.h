/*
 * 文件名: AudioManager.h
 * 说明: 音频管理器类，负责游戏音频的播放和管理。
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 本类管理游戏中的所有音频功能，包括背景音乐、音效、
 * 语音等的播放控制。提供音量调节、音频缓存、3D音效等
 * 高级功能，为游戏提供丰富的听觉体验。
 */
#pragma once

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSoundEffect>
#include <QHash>
#include <QQueue>
#include <memory>

/**
 * @brief 音频类型枚举
 * 
 * 定义不同类型的音频，用于分类管理和音量控制。
 */
enum class AudioType {
    BackgroundMusic,    ///< 背景音乐
    SoundEffect,        ///< 音效
    Voice,              ///< 语音
    UI                  ///< 界面音效
};

/**
 * @brief 音频管理器类
 * 
 * 该类负责游戏音频系统的完整管理，包括：
 * - 背景音乐的播放和切换
 * - 音效的播放和管理
 * - 音量控制和静音功能
 * - 音频资源的加载和缓存
 * - 3D音效和空间音频
 * 
 * 通过Q_PROPERTY暴露属性给QML，实现音频控制的UI绑定。
 */
class AudioManager : public QObject
{
    Q_OBJECT
    
    // QML可访问的属性
    Q_PROPERTY(float masterVolume READ masterVolume WRITE setMasterVolume NOTIFY masterVolumeChanged)
    Q_PROPERTY(float musicVolume READ musicVolume WRITE setMusicVolume NOTIFY musicVolumeChanged)
    Q_PROPERTY(float effectVolume READ effectVolume WRITE setEffectVolume NOTIFY effectVolumeChanged)
    Q_PROPERTY(bool isMuted READ isMuted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(QString currentMusic READ currentMusic NOTIFY currentMusicChanged)

public:
    /**
     * @brief 构造函数
     * 
     * 初始化音频管理器，创建媒体播放器和音频输出设备。
     * 
     * @param parent 父对象指针，用于Qt对象树管理
     */
    explicit AudioManager(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     * 
     * 清理音频资源，停止所有播放。
     */
    ~AudioManager();

    /**
     * @brief 初始化音频管理器
     * 
     * 初始化音频系统，加载音频资源：
     * - 创建媒体播放器
     * - 配置音频输出设备
     * - 加载音频资源列表
     * - 预加载常用音效
     * 
     * @return bool 初始化是否成功
     * @retval true 初始化成功，音频系统可用
     * @retval false 初始化失败，音频功能不可用
     */
    bool initialize();
    
    /**
     * @brief 关闭音频管理器
     * 
     * 停止所有音频播放，释放音频资源。
     */
    void shutdown();

    // 音量控制属性访问器
    
    /**
     * @brief 获取主音量
     * 
     * @return float 主音量（0.0-1.0）
     */
    float masterVolume() const { return m_masterVolume; }
    
    /**
     * @brief 设置主音量
     * 
     * @param volume 主音量（0.0-1.0）
     */
    void setMasterVolume(float volume);
    
    /**
     * @brief 获取音乐音量
     * 
     * @return float 音乐音量（0.0-1.0）
     */
    float musicVolume() const { return m_musicVolume; }
    
    /**
     * @brief 设置音乐音量
     * 
     * @param volume 音乐音量（0.0-1.0）
     */
    void setMusicVolume(float volume);
    
    /**
     * @brief 获取音效音量
     * 
     * @return float 音效音量（0.0-1.0）
     */
    float effectVolume() const { return m_effectVolume; }
    
    /**
     * @brief 设置音效音量
     * 
     * @param volume 音效音量（0.0-1.0）
     */
    void setEffectVolume(float volume);
    
    /**
     * @brief 获取静音状态
     * 
     * @return bool 是否静音
     */
    bool isMuted() const { return m_isMuted; }
    
    /**
     * @brief 设置静音状态
     * 
     * @param muted 是否静音
     */
    void setMuted(bool muted);
    
    /**
     * @brief 获取当前播放的音乐
     * 
     * @return QString 当前音乐文件名
     */
    QString currentMusic() const { return m_currentMusic; }

    // QML可调用的音频操作接口
    
    /**
     * @brief 播放背景音乐
     * 
     * 播放指定的背景音乐，支持循环播放和淡入淡出。
     * 
     * @param musicFile 音乐文件路径
     * @param loop 是否循环播放
     * @param fadeIn 是否淡入播放
     */
    Q_INVOKABLE void playMusic(const QString &musicFile, bool loop = true, bool fadeIn = true);
    
    /**
     * @brief 停止背景音乐
     * 
     * @param fadeOut 是否淡出停止
     */
    Q_INVOKABLE void stopMusic(bool fadeOut = true);
    
    /**
     * @brief 暂停背景音乐
     */
    Q_INVOKABLE void pauseMusic();
    
    /**
     * @brief 恢复背景音乐
     */
    Q_INVOKABLE void resumeMusic();
    
    /**
     * @brief 播放音效
     * 
     * 播放指定的音效文件。
     * 
     * @param effectFile 音效文件路径
     * @param volume 音效音量（0.0-1.0），-1表示使用默认音量
     * @return int 音效播放ID，用于控制特定音效
     */
    Q_INVOKABLE int playEffect(const QString &effectFile, float volume = -1.0f);
    
    /**
     * @brief 停止指定音效
     * 
     * @param effectId 音效播放ID
     */
    Q_INVOKABLE void stopEffect(int effectId);
    
    /**
     * @brief 停止所有音效
     */
    Q_INVOKABLE void stopAllEffects();
    
    /**
     * @brief 播放UI音效
     * 
     * 播放界面交互音效（按钮点击、菜单切换等）。
     * 
     * @param uiSound UI音效类型（"click", "hover", "error"等）
     */
    Q_INVOKABLE void playUISound(const QString &uiSound);
    
    /**
     * @brief 预加载音频文件
     * 
     * 预先加载音频文件到内存，减少播放延迟。
     * 
     * @param audioFile 音频文件路径
     */
    Q_INVOKABLE void preloadAudio(const QString &audioFile);
    
    /**
     * @brief 卸载音频文件
     * 
     * 从内存中卸载音频文件，释放内存。
     * 
     * @param audioFile 音频文件路径
     */
    Q_INVOKABLE void unloadAudio(const QString &audioFile);

signals:
    /**
     * @brief 主音量改变信号
     */
    void masterVolumeChanged();
    
    /**
     * @brief 音乐音量改变信号
     */
    void musicVolumeChanged();
    
    /**
     * @brief 音效音量改变信号
     */
    void effectVolumeChanged();
    
    /**
     * @brief 静音状态改变信号
     */
    void mutedChanged();
    
    /**
     * @brief 当前音乐改变信号
     */
    void currentMusicChanged();
    
    /**
     * @brief 音乐播放完成信号
     * 
     * @param musicFile 播放完成的音乐文件
     */
    void musicFinished(const QString &musicFile);
    
    /**
     * @brief 音效播放完成信号
     * 
     * @param effectId 播放完成的音效ID
     */
    void effectFinished(int effectId);

private slots:
    /**
     * @brief 媒体播放器状态改变处理
     * 
     * @param state 新的播放状态
     */
    void onMediaPlayerStateChanged(QMediaPlayer::PlaybackState state);
    
    /**
     * @brief 音频输出设备改变处理
     */
    void onAudioOutputChanged();

private:
    /**
     * @brief 更新实际音量
     * 
     * 根据主音量、分类音量和静音状态计算实际播放音量。
     * 
     * @param type 音频类型
     * @return float 实际播放音量
     */
    float calculateActualVolume(AudioType type) const;
    
    /**
     * @brief 生成音效ID
     * 
     * @return int 新的音效播放ID
     */
    int generateEffectId();

private:
    /**
     * @brief 背景音乐播放器
     * 
     * 用于播放背景音乐的媒体播放器。
     */
    std::unique_ptr<QMediaPlayer> m_musicPlayer;
    
    /**
     * @brief 音频输出设备
     * 
     * 音频输出设备，控制音量和音频路由。
     */
    std::unique_ptr<QAudioOutput> m_audioOutput;
    
    /**
     * @brief 音效播放器映射
     * 
     * 存储所有活跃的音效播放器，键为音效ID。
     */
    QHash<int, std::unique_ptr<QSoundEffect>> m_effectPlayers;
    
    /**
     * @brief 预加载的音效缓存
     * 
     * 预加载到内存中的音效文件缓存。
     */
    QHash<QString, std::unique_ptr<QSoundEffect>> m_preloadedEffects;
    
    /**
     * @brief 音效播放队列
     * 
     * 等待播放的音效队列，用于管理并发播放。
     */
    QQueue<QString> m_effectQueue;
    
    /**
     * @brief 主音量
     * 
     * 全局主音量控制（0.0-1.0）。
     */
    float m_masterVolume;
    
    /**
     * @brief 音乐音量
     * 
     * 背景音乐音量控制（0.0-1.0）。
     */
    float m_musicVolume;
    
    /**
     * @brief 音效音量
     * 
     * 音效音量控制（0.0-1.0）。
     */
    float m_effectVolume;
    
    /**
     * @brief 静音状态
     * 
     * 全局静音开关。
     */
    bool m_isMuted;
    
    /**
     * @brief 当前播放的音乐
     * 
     * 当前正在播放的背景音乐文件名。
     */
    QString m_currentMusic;
    
    /**
     * @brief 下一个音效ID
     * 
     * 用于生成唯一音效播放ID的计数器。
     */
    int m_nextEffectId;
    
    /**
     * @brief 最大并发音效数量
     * 
     * 允许同时播放的最大音效数量。
     */
    static const int MAX_CONCURRENT_EFFECTS = 16;
};