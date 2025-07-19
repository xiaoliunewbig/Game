/*
 * 文件名: StrategyService.cpp
 * 说明: 策略服务实现类的具体实现。
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 *
 * 本文件实现StrategyService类的所有方法，作为策略层的统一入口，
 * 协调各个子模块完成游戏规则管理、世界状态管理和事件调度功能。
 */
#include "Algorithm_interact/StrategyService.h"
#include <sstream>

namespace strategy {

StrategyService::StrategyService() 
    : rule_manager_(std::make_unique<GameRuleManager>())
    , world_engine_(std::make_unique<WorldStateEngine>())
    , event_scheduler_(std::make_unique<EventScheduler>()) {
}

WorldStateResult StrategyService::UpdateWorldState(const WorldStateUpdate& update) {
    WorldStateResult result;
    
    try {
        bool success = world_engine_->UpdateWorldState(update.world_state_json);
        
        if (success) {
            result.success = true;
            result.message = "世界状态更新成功";
            
            // 触发相关事件
            event_scheduler_->Update();
        } else {
            result.success = false;
            result.message = "世界状态更新失败：数据格式错误";
        }
    } catch (const std::exception& e) {
        result.success = false;
        result.message = std::string("世界状态更新异常：") + e.what();
    }
    
    return result;
}

EventTriggerResult StrategyService::TriggerEvent(const EventTriggerRequest& request) {
    EventTriggerResult result;
    
    try {
        // 检查事件触发条件
        std::string rule_id = "event_" + std::to_string(request.event_id);
        auto context = world_engine_->GetCurrentWorldState();
        
        // 将世界状态转换为规则检查所需的格式
        std::unordered_map<std::string, std::string> rule_context;
        for (const auto& var : context.global_variables) {
            rule_context[var.first] = std::to_string(var.second);
        }
        
        bool can_trigger = rule_manager_->CheckRuleCondition(rule_id, rule_context);
        
        if (can_trigger) {
            result = event_scheduler_->TriggerEvent(request.event_id, request.params);
            
            // 应用规则效果
            auto effects = rule_manager_->ApplyRuleEffect(rule_id);
            for (const auto& effect : effects) {
                if (effect.first.find("global_") == 0) {
                    std::string var_name = effect.first.substr(7);
                    world_engine_->SetGlobalVariable(var_name, std::stoi(effect.second));
                } else if (effect.first.find("flag_") == 0) {
                    std::string flag_name = effect.first.substr(5);
                    world_engine_->SetWorldFlag(flag_name, effect.second == "true");
                }
            }
        } else {
            result.triggered = false;
            result.result_json = "{\"error\":\"事件触发条件不满足\"}";
        }
    } catch (const std::exception& e) {
        result.triggered = false;
        result.result_json = "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
    
    return result;
}

GameRules StrategyService::GetGameRules(const std::string& rule_category) {
    return rule_manager_->GetRulesByCategory(rule_category);
}

GameState StrategyService::QueryGameState(const std::string& query_type) {
    return world_engine_->QueryGameState(query_type);
}

} // namespace strategy
