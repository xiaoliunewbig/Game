/*
 * 文件名: IStrategyService.h
 * 说明: 策略层对外服务接口，定义与算法层的数据交互方式。
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 *
 * 本接口用于策略层与算法层之间的数据交互，包括世界状态管理、事件触发等功能。
 */
#ifndef STRATEGY_ISTRATEGYSERVICE_H
#define STRATEGY_ISTRATEGYSERVICE_H

#include <string>
#include <vector>

namespace strategy {

/**
 * @brief 世界状态更新请求
 * 包含世界状态的序列化数据。
 */
struct WorldStateUpdate {
    std::string world_state_json; ///< 世界状态JSON字符串
};

/**
 * @brief 世界状态更新结果
 * 表示更新操作是否成功及相关信息。
 */
struct WorldStateResult {
    bool success;             ///< 是否成功
    std::string message;      ///< 结果说明或错误信息
};

/**
 * @brief 事件触发请求
 * 包含事件编号及参数。
 */
struct EventTriggerRequest {
    int event_id;                 ///< 事件编号
    std::vector<int> params;      ///< 事件参数
};

/**
 * @brief 事件触发结果
 * 表示事件是否被成功触发及结果数据。
 */
struct EventTriggerResult {
    bool triggered;               ///< 是否成功触发
    std::string result_json;      ///< 结果数据（JSON格式）
    std::vector<int> affected_npcs;      // 添加这个字段
    std::vector<std::string> state_changes;  // 添加这个字段
};

/**
 * @brief 策略服务接口（抽象类）
 * 提供世界状态管理、事件调度等服务。
 */
class IStrategyService {
public:
    virtual ~IStrategyService() = default;
    /**
     * @brief 更新世界状态
     * @param update 世界状态更新请求
     * @return WorldStateResult 更新结果
     */
    virtual WorldStateResult UpdateWorldState(const WorldStateUpdate& update) = 0;
    /**
     * @brief 触发事件
     * @param request 事件触发请求
     * @return EventTriggerResult 事件触发结果
     */
    virtual EventTriggerResult TriggerEvent(const EventTriggerRequest& request) = 0;
};

} // namespace strategy

#endif // STRATEGY_ISTRATEGYSERVICE_H 
