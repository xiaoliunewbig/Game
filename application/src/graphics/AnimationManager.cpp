/*
 * 文件名: AnimationManager.cpp
 * 说明: 动画管理器实现
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */

#include "graphics/AnimationManager.h"
#include <QDebug>
#include <QTimer>
#include <algorithm>

AnimationManager::AnimationManager(QObject *parent)
    : QObject(parent)
    , m_updateTimer(new QTimer(this))
    , m_animationEnabled(true)
    , m_globalSpeed(1.0f)
{
    // 设置更新定时器
    m_updateTimer->setInterval(16); // 约60FPS
    connect(m_updateTimer, &QTimer::timeout, this, &AnimationManager::updateAnimations);
    
    qDebug() << "AnimationManager: 初始化动画管理器";
}

AnimationManager::~AnimationManager()
{
    stopAllAnimations();
    qDebug() << "AnimationManager: 销毁动画管理器";
}

int AnimationManager::createAnimation(const QString &name, const QString &spriteName, 
                                    const QVector<int> &frames, int frameDuration, bool loop)
{
    if (name.isEmpty() || spriteName.isEmpty() || frames.isEmpty()) {
        qWarning() << "AnimationManager: 无效的动画参数";
        return -1;
    }
    
    // 生成唯一ID
    int animationId = generateAnimationId();
    
    // 创建动画数据
    AnimationData animData;
    animData.id = animationId;
    animData.name = name;
    animData.spriteName = spriteName;
    animData.frames = frames;
    animData.frameDuration = frameDuration;
    animData.totalDuration = frames.size() * frameDuration;
    animData.loop = loop;
    animData.state = AnimationState::Stopped;
    animData.currentFrame = 0;
    animData.currentTime = 0;
    animData.speed = 1.0f;
    
    // 存储动画
    m_animations[animationId] = animData;
    
    qDebug() << "AnimationManager: 创建动画" << name << "ID:" << animationId 
             << "帧数:" << frames.size() << "持续时间:" << animData.totalDuration << "ms";
    
    emit animationCreated(animationId, name);
    
    return animationId;
}

bool AnimationManager::playAnimation(int animationId)
{
    auto it = m_animations.find(animationId);
    if (it == m_animations.end()) {
        qWarning() << "AnimationManager: 动画不存在:" << animationId;
        return false;
    }
    
    AnimationData &animData = it.value();
    
    if (animData.state == AnimationState::Playing) {
        return true; // 已经在播放
    }
    
    animData.state = AnimationState::Playing;
    
    // 启动更新定时器
    if (!m_updateTimer->isActive()) {
        m_updateTimer->start();
    }
    
    qDebug() << "AnimationManager: 播放动画" << animData.name << "ID:" << animationId;
    emit animationStarted(animationId);
    
    return true;
}

bool AnimationManager::pauseAnimation(int animationId)
{
    auto it = m_animations.find(animationId);
    if (it == m_animations.end()) {
        qWarning() << "AnimationManager: 动画不存在:" << animationId;
        return false;
    }
    
    AnimationData &animData = it.value();
    
    if (animData.state == AnimationState::Playing) {
        animData.state = AnimationState::Paused;
        qDebug() << "AnimationManager: 暂停动画" << animData.name << "ID:" << animationId;
        emit animationPaused(animationId);
    }
    
    return true;
}

bool AnimationManager::stopAnimation(int animationId)
{
    auto it = m_animations.find(animationId);
    if (it == m_animations.end()) {
        qWarning() << "AnimationManager: 动画不存在:" << animationId;
        return false;
    }
    
    AnimationData &animData = it.value();
    
    animData.state = AnimationState::Stopped;
    animData.currentFrame = 0;
    animData.currentTime = 0;
    
    qDebug() << "AnimationManager: 停止动画" << animData.name << "ID:" << animationId;
    emit animationStopped(animationId);
    
    return true;
}

void AnimationManager::stopAllAnimations()
{
    for (auto &animData : m_animations) {
        if (animData.state == AnimationState::Playing || 
            animData.state == AnimationState::Paused) {
            animData.state = AnimationState::Stopped;
            animData.currentFrame = 0;
            animData.currentTime = 0;
            emit animationStopped(animData.id);
        }
    }
    
    m_updateTimer->stop();
    qDebug() << "AnimationManager: 停止所有动画";
}

bool AnimationManager::removeAnimation(int animationId)
{
    auto it = m_animations.find(animationId);
    if (it == m_animations.end()) {
        return false;
    }
    
    QString name = it->name;
    m_animations.erase(it);
    
    qDebug() << "AnimationManager: 移除动画" << name << "ID:" << animationId;
    emit animationRemoved(animationId);
    
    return true;
}

bool AnimationManager::hasAnimation(int animationId) const
{
    return m_animations.contains(animationId);
}

AnimationManager::AnimationState AnimationManager::getAnimationState(int animationId) const
{
    auto it = m_animations.find(animationId);
    if (it != m_animations.end()) {
        return it->state;
    }
    return AnimationState::Stopped;
}

int AnimationManager::getCurrentFrame(int animationId) const
{
    auto it = m_animations.find(animationId);
    if (it != m_animations.end()) {
        return it->frames[it->currentFrame];
    }
    return -1;
}

float AnimationManager::getAnimationProgress(int animationId) const
{
    auto it = m_animations.find(animationId);
    if (it != m_animations.end() && it->totalDuration > 0) {
        return static_cast<float>(it->currentTime) / it->totalDuration;
    }
    return 0.0f;
}

bool AnimationManager::setAnimationSpeed(int animationId, float speed)
{
    auto it = m_animations.find(animationId);
    if (it == m_animations.end()) {
        return false;
    }
    
    it->speed = qMax(0.1f, speed); // 最小速度0.1x
    return true;
}

float AnimationManager::getAnimationSpeed(int animationId) const
{
    auto it = m_animations.find(animationId);
    if (it != m_animations.end()) {
        return it->speed;
    }
    return 1.0f;
}

void AnimationManager::setGlobalSpeed(float speed)
{
    m_globalSpeed = qMax(0.0f, speed);
    qDebug() << "AnimationManager: 设置全局动画速度:" << m_globalSpeed;
    emit globalSpeedChanged(m_globalSpeed);
}

float AnimationManager::getGlobalSpeed() const
{
    return m_globalSpeed;
}

void AnimationManager::setAnimationEnabled(bool enabled)
{
    if (m_animationEnabled != enabled) {
        m_animationEnabled = enabled;
        
        if (enabled) {
            // 重新启动有活动动画的定时器
            bool hasActiveAnimations = std::any_of(m_animations.begin(), m_animations.end(),
                [](const AnimationData &data) {
                    return data.state == AnimationState::Playing;
                });
            
            if (hasActiveAnimations && !m_updateTimer->isActive()) {
                m_updateTimer->start();
            }
        } else {
            // 停止定时器
            m_updateTimer->stop();
        }
        
        qDebug() << "AnimationManager: 动画" << (enabled ? "启用" : "禁用");
        emit animationEnabledChanged(enabled);
    }
}

bool AnimationManager::isAnimationEnabled() const
{
    return m_animationEnabled;
}

QVector<int> AnimationManager::getActiveAnimations() const
{
    QVector<int> activeIds;
    for (auto it = m_animations.begin(); it != m_animations.end(); ++it) {
        if (it->state == AnimationState::Playing) {
            activeIds.append(it.key());
        }
    }
    return activeIds;
}

int AnimationManager::getAnimationCount() const
{
    return m_animations.size();
}

void AnimationManager::updateAnimations()
{
    if (!m_animationEnabled || m_globalSpeed <= 0.0f) {
        return;
    }
    
    bool hasActiveAnimations = false;
    const int deltaTime = m_updateTimer->interval();
    
    for (auto &animData : m_animations) {
        if (animData.state == AnimationState::Playing) {
            hasActiveAnimations = true;
            updateSingleAnimation(animData, deltaTime);
        }
    }
    
    // 如果没有活动动画，停止定时器
    if (!hasActiveAnimations) {
        m_updateTimer->stop();
    }
}

void AnimationManager::updateSingleAnimation(AnimationData &animData, int deltaTime)
{
    // 计算实际时间增量
    float effectiveSpeed = animData.speed * m_globalSpeed;
    int effectiveDelta = static_cast<int>(deltaTime * effectiveSpeed);
    
    animData.currentTime += effectiveDelta;
    
    // 计算当前帧
    int frameIndex = (animData.currentTime / animData.frameDuration) % animData.frames.size();
    
    if (frameIndex != animData.currentFrame) {
        animData.currentFrame = frameIndex;
        emit frameChanged(animData.id, animData.frames[frameIndex]);
    }
    
    // 检查动画是否完成
    if (!animData.loop && animData.currentTime >= animData.totalDuration) {
        animData.state = AnimationState::Stopped;
        animData.currentFrame = animData.frames.size() - 1; // 停在最后一帧
        animData.currentTime = animData.totalDuration;
        
        qDebug() << "AnimationManager: 动画完成" << animData.name << "ID:" << animData.id;
        emit animationFinished(animData.id);
    } else if (animData.loop && animData.currentTime >= animData.totalDuration) {
        // 循环动画重置
        animData.currentTime = 0;
        animData.currentFrame = 0;
    }
}

int AnimationManager::generateAnimationId()
{
    static int nextId = 1;
    return nextId++;
}