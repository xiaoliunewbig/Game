/*
 * File: WorldStateEngine.h
 * Description: World state engine for relation graphs and serialized state snapshots.
 */
#ifndef STRATEGY_WORLDSTATEENGINE_H
#define STRATEGY_WORLDSTATEENGINE_H

#include <string>
#include <unordered_map>
#include <vector>

namespace strategy {

/**
 * @brief NPC relation information.
 */
struct NPCRelation {
    int npc_id;
    int target_id;
    std::string relation_type;  // friend, enemy, neutral, ally
    int trust_level;            // -100 to 100
    std::vector<std::string> shared_history;
};

/**
 * @brief World state container.
 */
struct WorldState {
    std::unordered_map<int, NPCRelation> npc_relations;
    std::unordered_map<std::string, int> global_variables;
    std::unordered_map<std::string, bool> world_flags;
    std::vector<std::string> active_events;
    long long last_update_time;
};

/**
 * @brief Query response model for game state.
 */
struct GameState {
    WorldState world_state;
    std::string state_json;
    bool is_valid;
};

/**
 * @brief Handles world state update/query operations.
 */
class WorldStateEngine {
public:
    WorldStateEngine();

    bool UpdateWorldState(const std::string& world_state_json);
    WorldState GetCurrentWorldState() const;
    GameState QueryGameState(const std::string& query_type);

    bool UpdateNPCRelation(int npc_id, int target_id, const std::string& relation_type, int trust_change);
    NPCRelation GetNPCRelation(int npc_id, int target_id) const;

    void SetGlobalVariable(const std::string& var_name, int value);
    int GetGlobalVariable(const std::string& var_name) const;

    void SetWorldFlag(const std::string& flag_name, bool value);
    bool GetWorldFlag(const std::string& flag_name) const;

    std::vector<int> GetAIDecisionContext(int npc_id) const;

private:
    WorldState current_state_;

    void InitializeDefaultState();
    std::string SerializeWorldState() const;
    bool DeserializeWorldState(const std::string& json_data);
};

} // namespace strategy

#endif // STRATEGY_WORLDSTATEENGINE_H
