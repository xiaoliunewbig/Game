/*
 * 文件名: AudioManager.cpp
 * 说明: 音频管理器实现 - 统一管理游戏中的所有音频播放和音效处理
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 * 版本: v1.0.0
 * 
 * 功能描述:
 * - 管理背景音乐(BGM)和音效(SFX)的播放
 * - 提供音量控制和静音功能
 * - 支持音频淡入淡出效果
 * - 实现音频资源的缓存和预加载
 * - 提供3D空间音效支持（预留）
 * 
 * 音频分类:
 * - Master: 主音量控制
 * - Music: 背景音乐音量
 * - SFX: 音效音量
 * - Voice: 语音音量（预留）
 * 
 * 依赖项:
 * - Qt Multimedia (QMediaPlayer, QAudioOutput等)
 * - ConfigManager (配置管理)
 * 
 * 使用示例:
 * AudioManager* audio = AudioManager::instance();
 * audio->initialize();
 * audio->playMusic("bgm_main.mp3");
 * audio->playSFX("button_click.wav");
 */

#include "audio/AudioManager.h"
#include "config/ConfigManager.h"
#include <QDebug>
#include <QTimer>

/**
 * @brief 构造函数
 * @param parent 父对象指针，用于Qt对象树管理
 * 
 * 初始化音频管理器的基本状态和默认音量设置
 */
AudioManager::AudioManager(QObject *parent)
    : QObject(parent)
    , m_isInitialized(false)       // 初始化状态标志
    , m_masterVolume(1.0f)          // 主音量 (0.0-1.0)
    , m_musicVolume(0.7f)           // 音乐音量 (0.0-1.0)
    , m_sfxVolume(0.8f)             // 音效音量 (0.0-1.0)
    , m_isMuted(false)              // 静音状态
{
    qDebug() << "AudioManager: 构造函数调用，设置默认音量参数";
    qDebug() << "AudioManager: 默认音量 - Master:" << m_masterVolume 
             << "Music:" << m_musicVolume << "SFX:" << m_sfxVolume;
}

/**
 * @brief 析构函数
 * 
 * 停止所有音频播放，清理音频资源
 */
AudioManager::~AudioManager()
{
    qDebug() << "AudioManager: 析构函数调用，开始清理音频资源";
    shutdown();
    qDebug() << "AudioManager: 析构完成";
}

/**
 * @brief 初始化音频管理器
 * @return bool 初始化是否成功
 * 
 * 初始化流程：
 * 1. 初始化Qt音频系统
 * 2. 创建音频播放器实例
 * 3. 从配置文件加载音频设置
 * 4. 设置音频设备和格式
 * 5. 连接配置变更信号
 */
bool AudioManager::initialize()
{
    qDebug() << "AudioManager: 开始初始化音频系统";
    
    try {
        // 检查音频设备可用性
        if (!checkAudioDeviceAvailability()) {
            qWarning() << "AudioManager: 音频设备不可用，使用静默模式";
            // 继续初始化，但标记为静默模式
        }
        
        // 初始化音频播放器
        initializeAudioPlayers();
        
        // 从配置加载音频设置
        loadAudioSettings();
        
        // 应用初始音量设置
        applyVolumeSettings();
        
        m_isInitialized = true;
        qDebug() << "AudioManager: 音频系统初始化成功";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "AudioManager: 初始化失败:" << e.what();
        return false;
    }
}

/**
 * @brief 检查音频设备可用性
 * @return bool 音频设备是否可用
 * 
 * 检查系统是否有可用的音频输出设备
 */
bool AudioManager::checkAudioDeviceAvailability()
{
    QAudioDevice defaultDevice = QMediaDevices::defaultAudioOutput();
    if (defaultDevice.isNull()) {
        qWarning() << "AudioManager: 无可用音频输出设备";
        return false;
    }

    qDebug() << "AudioManager: 音频设备检查完成 -" << defaultDevice.description();
    return true;
}

/**
 * @brief 初始化音频播放器
 * 
 * 创建和配置不同类型的音频播放器：
 * - 背景音乐播放器（支持循环播放）
 * - 音效播放器池（支持同时播放多个音效）
 * - 语音播放器（预留）
 */
void AudioManager::initializeAudioPlayers()
{
    qDebug() << "AudioManager: 初始化音频播放器";

    // 创建背景音乐播放器
    m_audioOutput = std::make_unique<QAudioOutput>(this);
    m_musicPlayer = std::make_unique<QMediaPlayer>(this);
    m_musicPlayer->setAudioOutput(m_audioOutput.get());

    // 连接播放状态变更信号
    connect(m_musicPlayer.get(), &QMediaPlayer::playbackStateChanged,
            this, &AudioManager::onMediaPlayerStateChanged);

    // 初始化音效ID计数器
    m_nextEffectId = 1;

    qDebug() << "AudioManager: 音频播放器初始化完成";
}

/**
 * @brief 从配置文件加载音频设置
 * 
 * 从ConfigManager加载用户的音频偏好设置：
 * - 各类音量级别
 * - 静音状态
 * - 音频质量设置
 * - 音效开关状态
 * 
 * 同时连接配置变更信号，实现实时设置更新
 */
void AudioManager::loadAudioSettings()
{
    qDebug() << "AudioManager: 从配置加载音频设置";
    
    ConfigManager* config = ConfigManager::instance();
    
    // 加载音量设置
    m_masterVolume = config->getMasterVolume();
    m_musicVolume = config->getMusicVolume();
    m_sfxVolume = config->getSFXVolume();
    m_isMuted = config->isMuted();
    
    qDebug() << "AudioManager: 配置加载完成 - Master:" << m_masterVolume 
             << "Music:" << m_musicVolume << "SFX:" << m_sfxVolume 
             << "Muted:" << m_isMuted;
    
    // 连接配置变更信号，实现实时设置更新
    connect(config, &ConfigManager::masterVolumeChanged,
            this, &AudioManager::setMasterVolume);
    connect(config, &ConfigManager::musicVolumeChanged,
            this, &AudioManager::setMusicVolume);
    connect(config, &ConfigManager::sfxVolumeChanged,
            this, &AudioManager::setSFXVolume);
    connect(config, &ConfigManager::mutedChanged,
            this, &AudioManager::setMuted);
    
    qDebug() << "AudioManager: 配置信号连接完成";
}

/**
 * @brief 应用音量设置到音频系统
 * 
 * 将当前的音量设置应用到所有音频播放器
 */
void AudioManager::applyVolumeSettings()
{
    qDebug() << "AudioManager: 应用音量设置到音频系统";

    // 应用到背景音乐播放器
    if (m_audioOutput) {
        float effectiveVolume = m_isMuted ? 0.0f : (m_masterVolume * m_musicVolume);
        m_audioOutput->setVolume(effectiveVolume);
    }

    // 应用到音效播放器
    float sfxEffectiveVolume = m_isMuted ? 0.0f : (m_masterVolume * m_sfxVolume);
    for (auto &effect : m_effectPlayers) {
        effect->setVolume(sfxEffectiveVolume);
    }
    for (auto &effect : m_preloadedEffects) {
        effect->setVolume(sfxEffectiveVolume);
    }

    qDebug() << "AudioManager: 音量设置应用完成";
}

/**
 * @brief 设置主音量
 * @param volume 音量值 (0.0-1.0)
 * 
 * 设置全局主音量，影响所有音频输出
 * 音量值会被限制在有效范围内
 */
void AudioManager::setMasterVolume(float volume)
{
    // 限制音量范围
    float newVolume = qBound(0.0f, volume, 1.0f);
    
    if (qFuzzyCompare(m_masterVolume, newVolume)) {
        return; // 音量未变化，无需更新
    }
    
    qDebug() << "AudioManager: 设置主音量从" << m_masterVolume << "到" << newVolume;
    
    m_masterVolume = newVolume;
    
    // 应用新的音量设置
    applyVolumeSettings();
    
    // 发送音量变更信号
    emit masterVolumeChanged(m_masterVolume);
    
    qDebug() << "AudioManager: 主音量设置完成";
}

/**
 * @brief 设置音乐音量
 * @param volume 音量值 (0.0-1.0)
 * 
 * 设置背景音乐的音量级别
 */
void AudioManager::setMusicVolume(float volume)
{
    float newVolume = qBound(0.0f, volume, 1.0f);
    
    if (qFuzzyCompare(m_musicVolume, newVolume)) {
        return;
    }
    
    qDebug() << "AudioManager: 设置音乐音量从" << m_musicVolume << "到" << newVolume;
    
    m_musicVolume = newVolume;

    // 应用到音乐播放器
    if (m_audioOutput) {
        float effectiveVolume = m_isMuted ? 0.0f : (m_masterVolume * m_musicVolume);
        m_audioOutput->setVolume(effectiveVolume);
    }

    emit musicVolumeChanged(m_musicVolume);
    qDebug() << "AudioManager: 音乐音量设置完成";
}

/**
 * @brief 设置音效音量
 * @param volume 音量值 (0.0-1.0)
 * 
 * 设置游戏音效的音量级别
 */
void AudioManager::setSFXVolume(float volume)
{
    float newVolume = qBound(0.0f, volume, 1.0f);
    
    if (qFuzzyCompare(m_sfxVolume, newVolume)) {
        return;
    }
    
    qDebug() << "AudioManager: 设置音效音量从" << m_sfxVolume << "到" << newVolume;
    
    m_sfxVolume = newVolume;

    // 应用到音效播放器
    float effectiveVolume = m_isMuted ? 0.0f : (m_masterVolume * m_sfxVolume);
    for (auto &effect : m_effectPlayers) {
        effect->setVolume(effectiveVolume);
    }
    for (auto &effect : m_preloadedEffects) {
        effect->setVolume(effectiveVolume);
    }

    emit sfxVolumeChanged(m_sfxVolume);
    qDebug() << "AudioManager: 音效音量设置完成";
}

/**
 * @brief 设置静音状态
 * @param muted 是否静音
 * 
 * 切换全局静音状态，静音时所有音频输出音量为0
 */
void AudioManager::setMuted(bool muted)
{
    if (m_isMuted == muted) {
        return; // 状态未变化
    }
    
    qDebug() << "AudioManager: 设置静音状态从" << m_isMuted << "到" << muted;
    
    m_isMuted = muted;
    
    // 应用静音设置
    applyVolumeSettings();
    
    emit mutedChanged(m_isMuted);
    qDebug() << "AudioManager: 静音状态设置完成";
}

/**
 * @brief 关闭音频管理器
 * 
 * 执行完整的音频系统关闭流程：
 * 1. 停止所有正在播放的音频
 * 2. 释放音频播放器资源
 * 3. 断开所有信号连接
 * 4. 重置状态变量
 */
void AudioManager::shutdown()
{
    qDebug() << "AudioManager: 开始关闭音频系统";
    
    if (!m_isInitialized) {
        qDebug() << "AudioManager: 音频系统未初始化，跳过关闭流程";
        return;
    }
    
    // 停止所有音频播放
    if (m_musicPlayer) {
        m_musicPlayer->stop();
    }

    // 停止所有音效
    for (auto *effect : m_effectPlayers) {
        effect->stop();
    }
    qDeleteAll(m_effectPlayers);
    m_effectPlayers.clear();

    // 清理预加载音效
    for (auto *effect : m_preloadedEffects) {
        effect->stop();
    }
    qDeleteAll(m_preloadedEffects);
    m_preloadedEffects.clear();

    // 释放播放器资源
    m_musicPlayer.reset();
    m_audioOutput.reset();

    // 重置状态
    m_isInitialized = false;

    qDebug() << "AudioManager: 音频系统关闭完成";
}

float AudioManager::effectVolume() const
{
    return m_sfxVolume;
}

void AudioManager::setEffectVolume(float volume)
{
    setSFXVolume(volume);
}

void AudioManager::playMusic(const QString &musicFile, bool loop, bool fadeIn)
{
    Q_UNUSED(fadeIn)

    if (!m_isInitialized) {
        qWarning() << "AudioManager: 未初始化，无法播放音乐";
        return;
    }

    qDebug() << "AudioManager: 播放背景音乐:" << musicFile;
    m_currentMusic = musicFile;
    emit currentMusicChanged();

    if (m_musicPlayer) {
        m_musicPlayer->setSource(QUrl::fromLocalFile(musicFile));
        m_musicPlayer->setLoops(loop ? QMediaPlayer::Infinite : 1);
        m_musicPlayer->play();
    }
}

void AudioManager::stopMusic(bool fadeOut)
{
    Q_UNUSED(fadeOut)

    qDebug() << "AudioManager: 停止背景音乐";
    m_currentMusic.clear();
    emit currentMusicChanged();

    if (m_musicPlayer) {
        m_musicPlayer->stop();
    }
}

void AudioManager::pauseMusic()
{
    qDebug() << "AudioManager: 暂停背景音乐";
    if (m_musicPlayer) {
        m_musicPlayer->pause();
    }
}

void AudioManager::resumeMusic()
{
    qDebug() << "AudioManager: 恢复背景音乐";
    if (m_musicPlayer) {
        m_musicPlayer->play();
    }
}

int AudioManager::playEffect(const QString &effectFile, float volume)
{
    if (!m_isInitialized) {
        qWarning() << "AudioManager: 未初始化，无法播放音效";
        return -1;
    }

    int effectId = generateEffectId();

    qDebug() << "AudioManager: 播放音效:" << effectFile << "ID:" << effectId;

    float actualVolume = (volume < 0) ? calculateActualVolume(AudioType::SoundEffect) : volume;

    // 检查并发数限制
    if (m_effectPlayers.size() >= MAX_CONCURRENT_EFFECTS) {
        qWarning() << "AudioManager: 音效并发数已达上限:" << MAX_CONCURRENT_EFFECTS;
        return -1;
    }

    // 创建 QSoundEffect 并播放
    QSoundEffect* effect = new QSoundEffect(this);
    effect->setSource(QUrl::fromLocalFile(effectFile));
    effect->setVolume(actualVolume);
    effect->play();

    // 播放完毕自动清理
    int capturedId = effectId;
    connect(effect, &QSoundEffect::playingChanged, this, [this, capturedId]() {
        auto it = m_effectPlayers.find(capturedId);
        if (it != m_effectPlayers.end() && !(*it)->isPlaying()) {
            (*it)->deleteLater();
            m_effectPlayers.erase(it);
            emit effectFinished(capturedId);
        }
    });

    m_effectPlayers[effectId] = effect;

    return effectId;
}

void AudioManager::stopEffect(int effectId)
{
    auto it = m_effectPlayers.find(effectId);
    if (it != m_effectPlayers.end()) {
        (*it)->stop();
        (*it)->deleteLater();
        m_effectPlayers.erase(it);
        qDebug() << "AudioManager: 停止音效 ID:" << effectId;
    }
}

void AudioManager::stopAllEffects()
{
    for (auto *effect : m_effectPlayers) {
        effect->stop();
    }
    qDeleteAll(m_effectPlayers);
    m_effectPlayers.clear();
    qDebug() << "AudioManager: 停止所有音效";
}

void AudioManager::playUISound(const QString &uiSound)
{
    qDebug() << "AudioManager: 播放UI音效:" << uiSound;

    // UI 音效使用预加载的缓存
    if (m_preloadedEffects.contains(uiSound)) {
        float vol = calculateActualVolume(AudioType::UI);
        m_preloadedEffects[uiSound]->setVolume(vol);
        m_preloadedEffects[uiSound]->play();
    } else {
        // 如果未预加载，直接播放
        playEffect(uiSound);
    }
}

void AudioManager::preloadAudio(const QString &audioFile)
{
    if (m_preloadedEffects.contains(audioFile)) {
        return;
    }

    qDebug() << "AudioManager: 预加载音频:" << audioFile;

    QSoundEffect* effect = new QSoundEffect(this);
    effect->setSource(QUrl::fromLocalFile(audioFile));
    m_preloadedEffects[audioFile] = effect;
}

void AudioManager::unloadAudio(const QString &audioFile)
{
    auto it = m_preloadedEffects.find(audioFile);
    if (it != m_preloadedEffects.end()) {
        delete *it;
        m_preloadedEffects.erase(it);
        qDebug() << "AudioManager: 卸载音频:" << audioFile;
    }
}

void AudioManager::onMediaPlayerStateChanged(QMediaPlayer::PlaybackState state)
{
    if (state == QMediaPlayer::StoppedState && !m_currentMusic.isEmpty()) {
        emit musicFinished(m_currentMusic);
    }
}

void AudioManager::onAudioOutputChanged()
{
    qDebug() << "AudioManager: 音频输出设备改变";
    applyVolumeSettings();
}

float AudioManager::calculateActualVolume(AudioType type) const
{
    if (m_isMuted) {
        return 0.0f;
    }

    float typeVolume = 1.0f;
    switch (type) {
    case AudioType::BackgroundMusic:
        typeVolume = m_musicVolume;
        break;
    case AudioType::SoundEffect:
    case AudioType::UI:
        typeVolume = m_sfxVolume;
        break;
    case AudioType::Voice:
        typeVolume = 1.0f;
        break;
    }

    return m_masterVolume * typeVolume;
}

int AudioManager::generateEffectId()
{
    return m_nextEffectId++;
}
