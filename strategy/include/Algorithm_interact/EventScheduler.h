/*
 * 文件名: EventScheduler.h
 * 说明: 事件调度中心，负责任务触发系统和状态更新队列。
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 *
 * 本类管理游戏中的事件调度，包括定时事件、条件触发事件、任务系统等，
 * 与算法层协作处理复杂的游戏逻辑。
 */
#ifndef STRATEGY_EVENTSCHEDULER_H
#define STRATEGY_EVENTSCHEDULER_H

#include <string>
#include <vector>
#include <queue>
#include <functional>
#include <chrono>
#include <memory>
#include <unordered_map>
#include "../IStrategyService.h"  // 使用 IStrategyService.h 中的 EventTriggerResult

namespace strategy {

/**
 * @brief 事件类型枚举
 */
enum class EventType {
    IMMEDIATE,    // 立即执行
    DELAYED,      // 延时执行
    CONDITIONAL,  // 条件触发
    PERIODIC      // 周期执行
};

/**
 * @brief 游戏事件结构
 */
struct GameEvent {
    int event_id;
    std::string event_name;
    EventType type;
    std::vector<int> params;
    std::function<bool()> condition;  // 条件函数
    std::function<std::string(const std::vector<int>&)> handler;  // 处理函数
    std::chrono::steady_clock::time_point trigger_time;
    int repeat_interval_ms;  // 周期事件的间隔
    bool is_active;
};

/**
 * @brief 事件调度器
 */
class EventScheduler {
public:
    EventScheduler();

    /**
     * @brief 触发事件
     */
    EventTriggerResult TriggerEvent(int event_id, const std::vector<int>& params);

    /**
     * @brief 调度延时事件
     */
    bool ScheduleDelayedEvent(int event_id, const std::vector<int>& params, int delay_ms);

    /**
     * @brief 注册条件事件
     */
    bool RegisterConditionalEvent(int event_id, std::function<bool()> condition, 
                                 std::function<std::string(const std::vector<int>&)> handler);

    /**
     * @brief 注册周期事件
     */
    bool RegisterPeriodicEvent(int event_id, std::function<std::string(const std::vector<int>&)> handler, 
                              int interval_ms);

    /**
     * @brief 处理待执行事件
     */
    std::vector<EventTriggerResult> ProcessPendingEvents();

    /**
     * @brief 更新事件调度器
     */
    void Update();

    /**
     * @brief 取消事件
     */
    bool CancelEvent(int event_id);

private:
    std::unordered_map<int, GameEvent> events_;
    std::priority_queue<std::pair<std::chrono::steady_clock::time_point, int>> scheduled_events_;
    
    void InitializeDefaultEvents();
    void InitializeStoryEvents();
    void InitializeCombatEvents();
    void InitializeQuestEvents();
    
    EventTriggerResult ExecuteEvent(const GameEvent& event, const std::vector<int>& params);
};

} // namespace strategy

#endif // STRATEGY_EVENTSCHEDULER_H
