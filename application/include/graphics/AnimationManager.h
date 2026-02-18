/*
 * 文件名: AnimationManager.h
 * 说明: 动画管理器头文件
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */

#ifndef ANIMATIONMANAGER_H
#define ANIMATIONMANAGER_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QString>

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

/**
 * @brief 动画管理器类
 * 
 * 负责游戏动画的创建、播放和管理，支持：
 * - 帧动画播放
 * - 动画循环
 * - 播放速度控制
 * - 动画状态管理
 */
class AnimationManager : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(bool animationEnabled READ isAnimationEnabled WRITE setAnimationEnabled NOTIFY animationEnabledChanged)
    Q_PROPERTY(float globalSpeed READ getGlobalSpeed WRITE setGlobalSpeed NOTIFY globalSpeedChanged)
    Q_PROPERTY(int animationCount READ getAnimationCount NOTIFY animationCountChanged)
    
public:
    /**
     * @brief 动画状态枚举
     */
    enum class AnimationState {
        Stopped,    ///< 停止
        Playing,    ///< 播放中
        Paused      ///< 暂停
    };
    Q_ENUM(AnimationState)
    
    explicit AnimationManager(QObject *parent = nullptr);
    ~AnimationManager();
    
    // 动画创建和管理
    int createAnimation(const QString &name, const QString &spriteName, 
                       const QVector<int> &frames, int frameDuration, bool loop = true);
    bool removeAnimation(int animationId);
    bool hasAnimation(int animationId) const;
    
    // 动画播放控制
    bool playAnimation(int animationId);
    bool pauseAnimation(int animationId);
    bool stopAnimation(int animationId);
    void stopAllAnimations();
    
    // 动画状态查询
    AnimationState getAnimationState(int animationId) const;
    int getCurrentFrame(int animationId) const;
    float getAnimationProgress(int animationId) const;
    
    // 动画速度控制
    bool setAnimationSpeed(int animationId, float speed);
    float getAnimationSpeed(int animationId) const;
    void setGlobalSpeed(float speed);
    float getGlobalSpeed() const;
    
    // 系统控制
    void setAnimationEnabled(bool enabled);
    bool isAnimationEnabled() const;
    
    // 统计信息
    QVector<int> getActiveAnimations() const;
    int getAnimationCount() const;

signals:
    // 动画生命周期信号
    void animationCreated(int animationId, const QString &name);
    void animationRemoved(int animationId);
    void animationStarted(int animationId);
    void animationPaused(int animationId);
    void animationStopped(int animationId);
    void animationFinished(int animationId);
    
    // 动画播放信号
    void frameChanged(int animationId, int frameIndex);
    
    // 系统状态信号
    void animationEnabledChanged(bool enabled);
    void globalSpeedChanged(float speed);
    void animationCountChanged(int count);

private slots:
    void updateAnimations();

private:
    /**
     * @brief 动画数据结构
     */
    struct AnimationData {
        int id;                         ///< 动画ID
        QString name;                   ///< 动画名称
        QString spriteName;             ///< 精灵名称
        QVector<int> frames;            ///< 帧序列
        int frameDuration;              ///< 每帧持续时间（毫秒）
        int totalDuration;              ///< 总持续时间（毫秒）
        bool loop;                      ///< 是否循环
        AnimationState state;           ///< 当前状态
        int currentFrame;               ///< 当前帧索引
        int currentTime;                ///< 当前播放时间
        float speed;                    ///< 播放速度倍率
    };
    
    void updateSingleAnimation(AnimationData &animData, int deltaTime);
    int generateAnimationId();
    
private:
    QMap<int, AnimationData> m_animations;  ///< 动画数据映射
    QTimer *m_updateTimer;                  ///< 更新定时器
    bool m_animationEnabled;                ///< 动画是否启用
    float m_globalSpeed;                    ///< 全局播放速度
};

#endif // ANIMATIONMANAGER_H