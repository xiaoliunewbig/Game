/*
 * 文件名: StrategyTypes.h
 * 说明: 策略层通用数据类型定义。
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 *
 * 本文件定义策略层使用的通用数据结构和枚举类型，
 * 为各个模块提供统一的数据类型支持。
 */
#ifndef STRATEGY_STRATEGYTYPES_H
#define STRATEGY_STRATEGYTYPES_H

#include <string>
#include <vector>
#include <unordered_map>

namespace strategy {

/**
 * @brief 策略层错误码
 */
enum class StrategyErrorCode {
    SUCCESS = 0,
    INVALID_PARAMETER,
    SERVICE_UNAVAILABLE,
    RULE_NOT_FOUND,
    EVENT_FAILED,
    STATE_UPDATE_FAILED,
    ALGORITHM_SERVICE_ERROR
};

/**
 * @brief 策略层操作结果
 */
template<typename T>
struct StrategyResult {
    StrategyErrorCode error_code;
    std::string message;
    T data;
    
    bool IsSuccess() const { return error_code == StrategyErrorCode::SUCCESS; }
};

/**
 * @brief 游戏实体类型
 */
enum class EntityType {
    PLAYER,
    NPC,
    MONSTER,
    ITEM,
    ENVIRONMENT
};

/**
 * @brief 游戏实体基础信息
 */
struct GameEntity {
    int entity_id;
    EntityType type;
    std::string name;
    std::unordered_map<std::string, int> attributes;
    std::vector<std::string> tags;
};

} // namespace strategy

#endif // STRATEGY_STRATEGYTYPES_H