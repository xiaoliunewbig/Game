/*
 * 文件名: EventSystem.h
 * 说明: 游戏事件系统头文件
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 * 
 * 本文件定义了游戏的事件系统，提供解耦的事件通信机制。
 * 
 * 设计特点：
 * - 基于观察者模式的事件分发
 * - 类型安全的事件处理
 * - 支持事件优先级和过滤
 * - 异步和同步事件处理
 * - 事件队列管理
 * 
 * 使用场景：
 * - 游戏逻辑模块间通信
 * - UI事件响应
 * - 系统状态变更通知
 * - 游戏事件触发
 */

#ifndef EVENTSYSTEM_H
#define EVENTSYSTEM_H

#include <QObject>
#include <QVariant>
#include <QMap>
#include <QQueue>
#include <QTimer>
#include <QMutex>
#include <QJsonObject>
#include <functional>

/**
 * @brief 游戏事件基类
 * 
 * 所有游戏事件的基类，提供事件的基本信息和接口
 */
class GameEvent
{
public:
    /**
     * @brief 事件类型枚举
     */
    enum class Type {
        // 系统事件
        SystemStart,
        SystemShutdown,
        SceneChanged,
        
        // 游戏事件
        PlayerLevelUp,
        PlayerDied,
        EnemyDefeated,
        ItemCollected,
        QuestCompleted,
        
        // UI事件
        MenuOpened,
        MenuClosed,
        DialogShown,
        DialogClosed,
        
        // 输入事件
        KeyPressed,
        KeyReleased,
        MouseClicked,
        
        // 音频事件
        SoundPlayed,
        MusicChanged,
        
        // 自定义事件
        Custom = 1000
    };
    
    /**
     * @brief 事件优先级
     */
    enum class Priority {
        Low = 0,
        Normal = 1,
        High = 2,
        Critical = 3
    };
    
    /**
     * @brief 构造函数
     * @param type 事件类型
     * @param priority 事件优先级
     */
    explicit GameEvent(Type type, Priority priority = Priority::Normal);
    
    /**
     * @brief 虚析构函数
     */
    virtual ~GameEvent() = default;
    
    /**
     * @brief 获取事件类型
     * @return 事件类型
     */
    Type getType() const { return m_type; }
    
    /**
     * @brief 获取事件优先级
     * @return 事件优先级
     */
    Priority getPriority() const { return m_priority; }
    
    /**
     * @brief 获取事件时间戳
     * @return 事件创建时间
     */
    qint64 getTimestamp() const { return m_timestamp; }
    
    /**
     * @brief 设置事件数据
     * @param key 数据键
     * @param value 数据值
     */
    void setData(const QString &key, const QVariant &value);
    
    /**
     * @brief 获取事件数据
     * @param key 数据键
     * @param defaultValue 默认值
     * @return 数据值
     */
    QVariant getData(const QString &key, const QVariant &defaultValue = QVariant()) const;
    
    /**
     * @brief 检查是否包含指定数据
     * @param key 数据键
     * @return 包含返回true，否则返回false
     */
    bool hasData(const QString &key) const;
    
    /**
     * @brief 获取所有数据
     * @return 数据映射表
     */
    QMap<QString, QVariant> getAllData() const { return m_data; }

private:
    Type m_type;                        ///< 事件类型
    Priority m_priority;                ///< 事件优先级
    qint64 m_timestamp;                 ///< 事件时间戳
    QMap<QString, QVariant> m_data;     ///< 事件数据
};

/**
 * @brief 事件监听器接口
 * 
 * 事件处理器需要实现此接口来接收事件通知
 */
class IEventListener
{
public:
    virtual ~IEventListener() = default;
    
    /**
     * @brief 处理事件
     * @param event 事件对象
     * @return 是否处理了事件（true表示事件被消费，不再传递给其他监听器）
     */
    virtual bool handleEvent(const GameEvent &event) = 0;
};

/**
 * @brief 事件系统类
 * 
 * 游戏事件系统的核心类，负责事件的注册、分发和管理
 */
class EventSystem : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief 事件处理函数类型
     */
    using EventHandler = std::function<bool(const GameEvent&)>;
    
    /**
     * @brief 获取单例实例
     * @return EventSystem的唯一实例
     */
    static EventSystem* instance();
    
    /**
     * @brief 析构函数
     */
    ~EventSystem();
    
    // ==================== 事件发送接口 ====================
    
    /**
     * @brief 发送同步事件
     * @param event 事件对象
     * 
     * 立即处理事件，按优先级顺序通知所有监听器
     */
    void sendEvent(const GameEvent &event);
    
    /**
     * @brief 发送异步事件
     * @param event 事件对象
     * 
     * 将事件加入队列，在下一个事件循环中处理
     */
    void postEvent(const GameEvent &event);
    
    /**
     * @brief 发送延迟事件
     * @param event 事件对象
     * @param delayMs 延迟毫秒数
     * 
     * 在指定延迟后发送事件
     */
    void sendDelayedEvent(const GameEvent &event, int delayMs);
    
    // ==================== 监听器管理接口 ====================
    
    /**
     * @brief 注册事件监听器
     * @param eventType 事件类型
     * @param listener 监听器对象
     * @param priority 监听器优先级（数值越大优先级越高）
     */
    void registerListener(GameEvent::Type eventType, IEventListener* listener, int priority = 0);
    
    /**
     * @brief 注册事件处理函数
     * @param eventType 事件类型
     * @param handler 处理函数
     * @param priority 处理器优先级
     */
    void registerHandler(GameEvent::Type eventType, const EventHandler &handler, int priority = 0);
    
    /**
     * @brief 取消注册监听器
     * @param eventType 事件类型
     * @param listener 监听器对象
     */
    void unregisterListener(GameEvent::Type eventType, IEventListener* listener);
    
    /**
     * @brief 取消注册所有监听器
     * @param listener 监听器对象
     */
    void unregisterAllListeners(IEventListener* listener);
    
    // ==================== 事件过滤接口 ====================
    
    /**
     * @brief 事件过滤器函数类型
     */
    using EventFilter = std::function<bool(const GameEvent&)>;
    
    /**
     * @brief 添加事件过滤器
     * @param filter 过滤器函数
     * @param priority 过滤器优先级
     * @return 过滤器ID
     */
    int addEventFilter(const EventFilter &filter, int priority = 0);
    
    /**
     * @brief 移除事件过滤器
     * @param filterId 过滤器ID
     */
    void removeEventFilter(int filterId);
    
    // ==================== 事件队列管理接口 ====================
    
    /**
     * @brief 处理事件队列
     * 
     * 处理所有待处理的异步事件
     */
    void processEventQueue();
    
    /**
     * @brief 清空事件队列
     */
    void clearEventQueue();
    
    /**
     * @brief 获取队列中的事件数量
     * @return 事件数量
     */
    int getQueueSize() const;
    
    // ==================== 统计和调试接口 ====================
    
    /**
     * @brief 获取事件统计信息
     * @param eventType 事件类型
     * @return 事件处理次数
     */
    int getEventCount(GameEvent::Type eventType) const;
    
    /**
     * @brief 重置事件统计
     */
    void resetEventStats();
    
    /**
     * @brief 设置事件日志记录
     * @param enabled 是否启用
     */
    void setEventLogging(bool enabled);

signals:
    /**
     * @brief 事件处理完成信号
     * @param eventType 事件类型
     * @param handlerCount 处理器数量
     */
    void eventProcessed(GameEvent::Type eventType, int handlerCount);

private slots:
    /**
     * @brief 处理定时器事件
     */
    void onTimerTimeout();

private:
    /**
     * @brief 私有构造函数（单例模式）
     * @param parent 父对象指针
     */
    explicit EventSystem(QObject *parent = nullptr);
    
    /**
     * @brief 监听器信息结构
     */
    struct ListenerInfo {
        IEventListener* listener;
        EventHandler handler;
        int priority;
        bool isHandler;  // 区分监听器和处理函数
        
        ListenerInfo(IEventListener* l, int p) 
            : listener(l), priority(p), isHandler(false) {}
        ListenerInfo(const EventHandler& h, int p) 
            : listener(nullptr), handler(h), priority(p), isHandler(true) {}
    };
    
    /**
     * @brief 过滤器信息结构
     */
    struct FilterInfo {
        EventFilter filter;
        int priority;
        int id;
    };
    
    /**
     * @brief 延迟事件结构
     */
    struct DelayedEvent {
        GameEvent event;
        qint64 triggerTime;
    };
    
    /**
     * @brief 分发事件给监听器
     * @param event 事件对象
     * @return 是否有监听器处理了事件
     */
    bool dispatchEvent(const GameEvent &event);
    
    /**
     * @brief 应用事件过滤器
     * @param event 事件对象
     * @return 是否通过过滤器
     */
    bool applyEventFilters(const GameEvent &event);
    
    /**
     * @brief 按优先级排序监听器
     * @param listeners 监听器列表
     */
    void sortListenersByPriority(QList<ListenerInfo> &listeners);

private:
    static EventSystem* s_instance;                                    ///< 单例实例
    QMap<GameEvent::Type, QList<ListenerInfo>> m_listeners;           ///< 事件监听器映射
    QQueue<GameEvent> m_eventQueue;                                   ///< 异步事件队列
    QList<DelayedEvent> m_delayedEvents;                             ///< 延迟事件列表
    QList<FilterInfo> m_eventFilters;                                ///< 事件过滤器列表
    QMap<GameEvent::Type, int> m_eventStats;                        ///< 事件统计
    QTimer* m_processTimer;                                          ///< 事件处理定时器
    bool m_eventLogging;                                            ///< 是否启用事件日志
    int m_nextFilterId;                                             ///< 下一个过滤器ID
    mutable QMutex m_queueMutex;                                      ///< 队列互斥锁   
};

// ==================== 便利内联函数 ====================

/**
 * @brief 发送游戏事件的便利函数
 * @param type 事件类型
 */
inline void sendGameEvent(GameEvent::Type type) {
    GameEvent event(type);
    EventSystem::instance()->sendEvent(event);
}

/**
 * @brief 发送带数据的游戏事件
 * @param type 事件类型
 * @param data 事件数据
 */
inline void sendGameEvent(GameEvent::Type type, const QJsonObject &data) {
    GameEvent event(type);
    for (auto it = data.begin(); it != data.end(); ++it) {
        event.setData(it.key(), it.value().toVariant());
    }
    EventSystem::instance()->sendEvent(event);
}

/**
 * @brief 发送异步游戏事件的便利函数
 * @param type 事件类型
 */
inline void postGameEvent(GameEvent::Type type) {
    GameEvent event(type);
    EventSystem::instance()->postEvent(event);
}

/**
 * @brief 发送带数据的异步游戏事件
 * @param type 事件类型
 * @param data 事件数据
 */
inline void postGameEvent(GameEvent::Type type, const QJsonObject &data) {
    GameEvent event(type);
    for (auto it = data.begin(); it != data.end(); ++it) {
        event.setData(it.key(), it.value().toVariant());
    }
    EventSystem::instance()->postEvent(event);
}

// 保留旧宏以兼容已有代码，但推荐使用上方的内联函数
#define SEND_EVENT(type, ...) \
    do { \
        GameEvent event(GameEvent::Type::type); \
        __VA_ARGS__ \
        EventSystem::instance()->sendEvent(event); \
    } while(0)

#define POST_EVENT(type, ...) \
    do { \
        GameEvent event(GameEvent::Type::type); \
        __VA_ARGS__ \
        EventSystem::instance()->postEvent(event); \
    } while(0)

#endif // EVENTSYSTEM_H
