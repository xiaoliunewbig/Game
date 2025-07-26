/*
 * 文件名: EventSystem.cpp
 * 说明: 游戏事件系统实现文件
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 * 
 * 本文件实现了游戏的事件系统，提供解耦的事件通信机制。
 */

#include "core/EventSystem.h"
#include <QDebug>
#include <QDateTime>
#include <algorithm>

// ==================== GameEvent 实现 ====================

GameEvent::GameEvent(Type type, Priority priority)
    : m_type(type)
    , m_priority(priority)
    , m_timestamp(QDateTime::currentMSecsSinceEpoch())
{
}

void GameEvent::setData(const QString &key, const QVariant &value)
{
    m_data[key] = value;
}

QVariant GameEvent::getData(const QString &key, const QVariant &defaultValue) const
{
    return m_data.value(key, defaultValue);
}

bool GameEvent::hasData(const QString &key) const
{
    return m_data.contains(key);
}

// ==================== EventSystem 实现 ====================

EventSystem* EventSystem::s_instance = nullptr;

EventSystem::EventSystem(QObject *parent)
    : QObject(parent)
    , m_processTimer(new QTimer(this))
    , m_eventLogging(false)
    , m_nextFilterId(1)
{
    // 设置事件处理定时器
    m_processTimer->setInterval(16); // 约60FPS
    connect(m_processTimer, &QTimer::timeout, this, &EventSystem::onTimerTimeout);
    m_processTimer->start();
    
    qDebug() << "EventSystem: 事件系统初始化完成";
}

EventSystem::~EventSystem()
{
    clearEventQueue();
    m_listeners.clear();
    m_eventFilters.clear();
    m_delayedEvents.clear();
    
    if (s_instance == this) {
        s_instance = nullptr;
    }
    
    qDebug() << "EventSystem: 事件系统销毁完成";
}

EventSystem* EventSystem::instance()
{
    if (!s_instance) {
        s_instance = new EventSystem();
    }
    return s_instance;
}

// ==================== 事件发送接口 ====================

void EventSystem::sendEvent(const GameEvent &event)
{
    if (m_eventLogging) {
        qDebug() << "EventSystem: 发送同步事件" << static_cast<int>(event.getType());
    }
    
    // 应用事件过滤器
    if (!applyEventFilters(event)) {
        return;
    }
    
    // 立即分发事件
    dispatchEvent(event);
    
    // 更新统计
    m_eventStats[event.getType()]++;
}

void EventSystem::postEvent(const GameEvent &event)
{
    if (m_eventLogging) {
        qDebug() << "EventSystem: 发送异步事件" << static_cast<int>(event.getType());
    }
    
    QMutexLocker locker(&m_queueMutex);
    m_eventQueue.enqueue(event);
}

void EventSystem::sendDelayedEvent(const GameEvent &event, int delayMs)
{
    if (m_eventLogging) {
        qDebug() << "EventSystem: 发送延迟事件" << static_cast<int>(event.getType()) 
                 << "延迟" << delayMs << "毫秒";
    }
    
    DelayedEvent delayedEvent{event, QDateTime::currentMSecsSinceEpoch() + delayMs};
    m_delayedEvents.append(delayedEvent);
}

// ==================== 监听器管理接口 ====================

void EventSystem::registerListener(GameEvent::Type eventType, IEventListener* listener, int priority)
{
    if (!listener) {
        qWarning() << "EventSystem: 尝试注册空监听器";
        return;
    }
    
    ListenerInfo info(listener, priority);
    m_listeners[eventType].append(info);
    
    // 按优先级排序
    sortListenersByPriority(m_listeners[eventType]);
    
    if (m_eventLogging) {
        qDebug() << "EventSystem: 注册监听器" << listener 
                 << "事件类型" << static_cast<int>(eventType)
                 << "优先级" << priority;
    }
}

void EventSystem::registerHandler(GameEvent::Type eventType, const EventHandler &handler, int priority)
{
    if (!handler) {
        qWarning() << "EventSystem: 尝试注册空处理函数";
        return;
    }
    
    ListenerInfo info(handler, priority);
    m_listeners[eventType].append(info);
    
    // 按优先级排序
    sortListenersByPriority(m_listeners[eventType]);
    
    if (m_eventLogging) {
        qDebug() << "EventSystem: 注册处理函数"
                 << "事件类型" << static_cast<int>(eventType)
                 << "优先级" << priority;
    }
}

void EventSystem::unregisterListener(GameEvent::Type eventType, IEventListener* listener)
{
    auto it = m_listeners.find(eventType);
    if (it != m_listeners.end()) {
        it.value().removeIf([listener](const ListenerInfo& info) {
            return !info.isHandler && info.listener == listener;
        });
        
        if (it.value().isEmpty()) {
            m_listeners.erase(it);
        }
    }
    
    if (m_eventLogging) {
        qDebug() << "EventSystem: 取消注册监听器" << listener;
    }
}

void EventSystem::unregisterAllListeners(IEventListener* listener)
{
    for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it) {
        auto& listeners = it.value();
        listeners.removeIf([listener](const ListenerInfo& info) {
            return !info.isHandler && info.listener == listener;
        });
    }
    
    // 移除空的事件类型
    for (auto it = m_listeners.begin(); it != m_listeners.end();) {
        if (it.value().isEmpty()) {
            it = m_listeners.erase(it);
        } else {
            ++it;
        }
    }
    
    if (m_eventLogging) {
        qDebug() << "EventSystem: 取消注册所有监听器" << listener;
    }
}

// ==================== 事件过滤接口 ====================

int EventSystem::addEventFilter(const EventFilter &filter, int priority)
{
    FilterInfo info;
    info.id = m_nextFilterId++;
    info.filter = filter;
    info.priority = priority;
    
    m_eventFilters.append(info);
    
    if (m_eventLogging) {
        qDebug() << "EventSystem: 添加事件过滤器" << info.id;
    }
    
    return info.id;
}

void EventSystem::removeEventFilter(int filterId)
{
    m_eventFilters.removeIf([filterId](const FilterInfo& info) {
        return info.id == filterId;
    });
    
    if (m_eventLogging) {
        qDebug() << "EventSystem: 移除事件过滤器" << filterId;
    }
}

// ==================== 事件队列管理接口 ====================

void EventSystem::processEventQueue()
{
    QMutexLocker locker(&m_queueMutex);
    
    while (!m_eventQueue.isEmpty()) {
        GameEvent event = m_eventQueue.dequeue();
        locker.unlock();
        
        // 应用事件过滤器
        if (applyEventFilters(event)) {
            dispatchEvent(event);
            m_eventStats[event.getType()]++;
        }
        
        locker.relock();
    }
}

void EventSystem::clearEventQueue()
{
    QMutexLocker locker(&m_queueMutex);
    m_eventQueue.clear();
    
    if (m_eventLogging) {
        qDebug() << "EventSystem: 清空事件队列";
    }
}

int EventSystem::getQueueSize() const
{
    QMutexLocker locker(&m_queueMutex);
    return m_eventQueue.size();
}

int EventSystem::getEventCount(GameEvent::Type eventType) const
{
    return m_eventStats.value(eventType, 0);
}

void EventSystem::resetEventStats()
{
    m_eventStats.clear();
    if (m_eventLogging) {
        qDebug() << "EventSystem: 重置事件统计";
    }
}

void EventSystem::setEventLogging(bool enabled)
{
    m_eventLogging = enabled;
    qDebug() << "EventSystem: 事件日志" << (enabled ? "启用" : "禁用");
}

// ==================== 私有槽函数 ====================

void EventSystem::onTimerTimeout()
{
    // 处理异步事件队列
    processEventQueue();
    
    // 处理延迟事件
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    
    for (int i = m_delayedEvents.size() - 1; i >= 0; --i) {
        if (m_delayedEvents[i].triggerTime <= currentTime) {
            GameEvent event = m_delayedEvents[i].event;
            m_delayedEvents.removeAt(i);
            
            // 发送延迟事件
            sendEvent(event);
        }
    }
}

// ==================== 私有辅助函数 ====================

bool EventSystem::dispatchEvent(const GameEvent &event)
{
    auto it = m_listeners.find(event.getType());
    if (it == m_listeners.end()) {
        return false;
    }
    
    int handlerCount = 0;
    bool eventHandled = false;
    
    for (const ListenerInfo& info : it.value()) {
        bool handled = false;
        
        if (info.isHandler && info.handler) {
            handled = info.handler(event);
        } else if (!info.isHandler && info.listener) {
            handled = info.listener->handleEvent(event);
        }
        
        if (handled) {
            eventHandled = true;
            handlerCount++;
            
            // 如果事件被消费，停止传递给其他监听器
            break;
        }
    }
    
    // 发送事件处理完成信号
    emit eventProcessed(event.getType(), handlerCount);
    
    return eventHandled;
}

bool EventSystem::applyEventFilters(const GameEvent &event)
{
    for (const FilterInfo& info : m_eventFilters) {
        if (info.filter && !info.filter(event)) {
            if (m_eventLogging) {
                qDebug() << "EventSystem: 事件被过滤器" << info.id << "拦截";
            }
            return false;
        }
    }
    return true;
}

void EventSystem::sortListenersByPriority(QList<ListenerInfo> &listeners)
{
    std::sort(listeners.begin(), listeners.end(),
              [](const ListenerInfo& a, const ListenerInfo& b) {
                  return a.priority > b.priority;
              });
}
