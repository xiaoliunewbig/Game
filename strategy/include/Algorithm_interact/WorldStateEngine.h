/*
 * 文件名: WorldStateEngine.h
 * 说明: 世界状态引擎，负责NPC关系图谱和世界状态持久化。
 * 作者: 彭承康
 * 创建时间: 2025-07-19
 *
 * 本类管理游戏世界的动态状态，包括NPC关系、玩家声望、世界事件状态等，
 * 与算法层协作处理AI决策所需的上下文信息。
 */
#ifndef STRATEGY_WORLDSTATEENGINE_H
#define STRATEGY_WORLDSTATEENGINE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace strategy {

/**
 * @brief NPC关系数据
 */
struct NPCRelation {
    int npc_id;
    int target_id;
    std::string relation_type;  // "friend", "enemy", "neutral", "ally"
    int trust_level;  // -100 to 100
    std::vector<std::string> shared_history;
};

/**
 * @brief 世界状态数据
 */
struct WorldState {
    std::unordered_map<int, NPCRelation> npc_relations;
    std::unordered_map<std::string, int> global_variables;
    std::unordered_map<std::string, bool> world_flags;
    std::vector<std::string> active_events;
    long long last_update_time;
};

/**
 * @brief 游戏状态查询结果
 */
struct GameState {
    WorldState world_state;
    std::string state_json;
    bool is_valid;
};

/**
 * @brief 世界状态引擎
 */
class WorldStateEngine {
public:
    WorldStateEngine();

    /**
     * @brief 更新世界状态
     */
    bool UpdateWorldState(const std::string& world_state_json);

    /**
     * @brief 获取当前世界状态
     */
    WorldState GetCurrentWorldState() const;

    /**
     * @brief 查询游戏状态
     */
    GameState QueryGameState(const std::string& query_type);

    /**
     * @brief 更新NPC关系
     */
    bool UpdateNPCRelation(int npc_id, int target_id, const std::string& relation_type, int trust_change);

    /**
     * @brief 获取NPC关系
     */
    NPCRelation GetNPCRelation(int npc_id, int target_id) const;

    /**
     * @brief 设置全局变量
     */
    void SetGlobalVariable(const std::string& var_name, int value);

    /**
     * @brief 获取全局变量
     */
    int GetGlobalVariable(const std::string& var_name) const;

    /**
     * @brief 设置世界标志
     */
    void SetWorldFlag(const std::string& flag_name, bool value);

    /**
     * @brief 检查世界标志
     */
    bool GetWorldFlag(const std::string& flag_name) const;

    /**
     * @brief 获取AI决策上下文
     */
    std::vector<int> GetAIDecisionContext(int npc_id) const;

private:
    WorldState current_state_;
    
    void InitializeDefaultState();
    std::string SerializeWorldState() const;
    bool DeserializeWorldState(const std::string& json_data);
};

} // namespace strategy

#endif // STRATEGY_WORLDSTATEENGINE_H