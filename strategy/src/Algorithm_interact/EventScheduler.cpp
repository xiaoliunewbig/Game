/*
 * 文件名: EventScheduler.cpp
 * 说明: 事件调度中心的具体实现。
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 *
 * 本文件实现事件调度的核心逻辑，包括事件注册、触发、延时执行、
 * 条件检查等功能。
 */
#include "Algorithm_interact/EventScheduler.h"
#include <sstream>
#include <algorithm>

namespace strategy {

EventScheduler::EventScheduler() {
    InitializeDefaultEvents();
}

EventTriggerResult EventScheduler::TriggerEvent(int event_id, const std::vector<int>& params) {
    auto it = events_.find(event_id);
    if (it == events_.end() || !it->second.is_active) {
        EventTriggerResult result;
        result.triggered = false;
        result.result_json = "{\"error\":\"事件不存在或未激活\"}";
        return result;
    }
    
    return ExecuteEvent(it->second, params);
}

bool EventScheduler::ScheduleDelayedEvent(int event_id, const std::vector<int>& params, int delay_ms) {
    auto it = events_.find(event_id);
    if (it == events_.end()) {
        return false;
    }
    
    GameEvent delayed_event = it->second;
    delayed_event.type = EventType::DELAYED;
    delayed_event.params = params;
    delayed_event.trigger_time = std::chrono::steady_clock::now() + 
                                std::chrono::milliseconds(delay_ms);
    
    events_[event_id] = delayed_event;
    scheduled_events_.push({delayed_event.trigger_time, event_id});
    
    return true;
}

bool EventScheduler::RegisterConditionalEvent(int event_id, std::function<bool()> condition, 
                                             std::function<std::string(const std::vector<int>&)> handler) {
    GameEvent conditional_event;
    conditional_event.event_id = event_id;
    conditional_event.event_name = "conditional_event_" + std::to_string(event_id);
    conditional_event.type = EventType::CONDITIONAL;
    conditional_event.condition = condition;
    conditional_event.handler = handler;
    conditional_event.is_active = true;
    
    events_[event_id] = conditional_event;
    return true;
}

bool EventScheduler::RegisterPeriodicEvent(int event_id, 
                                          std::function<std::string(const std::vector<int>&)> handler, 
                                          int interval_ms) {
    GameEvent periodic_event;
    periodic_event.event_id = event_id;
    periodic_event.event_name = "periodic_event_" + std::to_string(event_id);
    periodic_event.type = EventType::PERIODIC;
    periodic_event.handler = handler;
    periodic_event.repeat_interval_ms = interval_ms;
    periodic_event.trigger_time = std::chrono::steady_clock::now() + 
                                 std::chrono::milliseconds(interval_ms);
    periodic_event.is_active = true;
    
    events_[event_id] = periodic_event;
    scheduled_events_.push({periodic_event.trigger_time, event_id});
    
    return true;
}

std::vector<EventTriggerResult> EventScheduler::ProcessPendingEvents() {
    std::vector<EventTriggerResult> results;
    auto now = std::chrono::steady_clock::now();
    
    while (!scheduled_events_.empty() && scheduled_events_.top().first <= now) {
        int event_id = scheduled_events_.top().second;
        scheduled_events_.pop();
        
        auto it = events_.find(event_id);
        if (it != events_.end() && it->second.is_active) {
            EventTriggerResult result = ExecuteEvent(it->second, it->second.params);
            results.push_back(result);
            
            // 如果是周期事件，重新调度
            if (it->second.type == EventType::PERIODIC) {
                it->second.trigger_time = now + 
                    std::chrono::milliseconds(it->second.repeat_interval_ms);
                scheduled_events_.push({it->second.trigger_time, event_id});
            }
        }
    }
    
    return results;
}

void EventScheduler::Update() {
    // 检查条件事件
    for (auto& pair : events_) {
        GameEvent& event = pair.second;
        if (event.type == EventType::CONDITIONAL && event.is_active && event.condition) {
            if (event.condition()) {
                ExecuteEvent(event, event.params);
            }
        }
    }
    
    // 处理待执行事件
    ProcessPendingEvents();
}

bool EventScheduler::CancelEvent(int event_id) {
    auto it = events_.find(event_id);
    if (it != events_.end()) {
        it->second.is_active = false;
        return true;
    }
    return false;
}

void EventScheduler::InitializeDefaultEvents() {
    InitializeStoryEvents();
    InitializeCombatEvents();
    InitializeQuestEvents();
}

void EventScheduler::InitializeStoryEvents() {
    // 剧情开始事件
    GameEvent story_start;
    story_start.event_id = 1001;
    story_start.event_name = "story_chapter_start";
    story_start.type = EventType::IMMEDIATE;
    story_start.handler = [](const std::vector<int>& params) -> std::string {
        std::ostringstream oss;
        oss << "{\"event\":\"story_start\",\"chapter\":" << (params.empty() ? 1 : params[0]) << "}";
        return oss.str();
    };
    story_start.is_active = true;
    
    events_[story_start.event_id] = story_start;
}

void EventScheduler::InitializeCombatEvents() {
    // 战斗胜利事件
    GameEvent combat_victory;
    combat_victory.event_id = 2001;
    combat_victory.event_name = "combat_victory";
    combat_victory.type = EventType::IMMEDIATE;
    combat_victory.handler = [](const std::vector<int>& params) -> std::string {
        int exp_gained = params.size() > 0 ? params[0] : 50;
        std::ostringstream oss;
        oss << "{\"event\":\"combat_victory\",\"exp_gained\":" << exp_gained << "}";
        return oss.str();
    };
    combat_victory.is_active = true;
    
    events_[combat_victory.event_id] = combat_victory;
}

void EventScheduler::InitializeQuestEvents() {
    // 任务完成事件
    GameEvent quest_complete;
    quest_complete.event_id = 3001;
    quest_complete.event_name = "quest_complete";
    quest_complete.type = EventType::IMMEDIATE;
    quest_complete.handler = [](const std::vector<int>& params) -> std::string {
        int quest_id = params.size() > 0 ? params[0] : 0;
        int reward = params.size() > 1 ? params[1] : 100;
        std::ostringstream oss;
        oss << "{\"event\":\"quest_complete\",\"quest_id\":" << quest_id 
            << ",\"reward\":" << reward << "}";
        return oss.str();
    };
    quest_complete.is_active = true;
    
    events_[quest_complete.event_id] = quest_complete;
}

EventTriggerResult EventScheduler::ExecuteEvent(const GameEvent& event, const std::vector<int>& params) {
    EventTriggerResult result;
    
    try {
        if (event.handler) {
            result.result_json = event.handler(params);
            result.triggered = true;
            
            // 模拟受影响的NPC和状态变化
            if (!params.empty()) {
                result.affected_npcs.push_back(params[0]);
            }
            result.state_changes.push_back("event_" + std::to_string(event.event_id) + "_executed");
        } else {
            result.triggered = false;
            result.result_json = "{\"error\":\"事件处理器未定义\"}";
        }
    } catch (const std::exception& e) {
        result.triggered = false;
        result.result_json = "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
    
    return result;
}

} // namespace strategy
