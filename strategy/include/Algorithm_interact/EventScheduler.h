/*
 * File: EventScheduler.h
 * Description: Event scheduler for immediate, delayed, conditional, and periodic events.
 */
#ifndef STRATEGY_EVENTSCHEDULER_H
#define STRATEGY_EVENTSCHEDULER_H

#include "../IStrategyService.h"

#include <chrono>
#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

namespace strategy {

enum class EventType {
    IMMEDIATE,
    DELAYED,
    CONDITIONAL,
    PERIODIC
};

struct GameEvent {
    int event_id;
    std::string event_name;
    EventType type;
    std::vector<int> params;
    std::function<bool()> condition;
    std::function<std::string(const std::vector<int>&)> handler;
    std::chrono::steady_clock::time_point trigger_time;
    int repeat_interval_ms;
    bool is_active;
};

class EventScheduler {
public:
    EventScheduler();

    EventTriggerResult TriggerEvent(int event_id, const std::vector<int>& params);

    bool ScheduleDelayedEvent(int event_id, const std::vector<int>& params, int delay_ms);

    bool RegisterConditionalEvent(
        int event_id,
        std::function<bool()> condition,
        std::function<std::string(const std::vector<int>&)> handler);

    bool RegisterPeriodicEvent(
        int event_id,
        std::function<std::string(const std::vector<int>&)> handler,
        int interval_ms);

    std::vector<EventTriggerResult> ProcessPendingEvents();

    void Update();

    bool CancelEvent(int event_id);

private:
    std::unordered_map<int, GameEvent> events_;
    std::priority_queue<
        std::pair<std::chrono::steady_clock::time_point, int>,
        std::vector<std::pair<std::chrono::steady_clock::time_point, int>>,
        std::greater<std::pair<std::chrono::steady_clock::time_point, int>>
    > scheduled_events_;

    void InitializeDefaultEvents();
    void InitializeStoryEvents();
    void InitializeCombatEvents();
    void InitializeQuestEvents();

    EventTriggerResult ExecuteEvent(const GameEvent& event, const std::vector<int>& params);
};

} // namespace strategy

#endif // STRATEGY_EVENTSCHEDULER_H
