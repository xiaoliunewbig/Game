/*
 * test_strategy_logic.cpp
 * Strategy rule/world/service logic tests
 */
#include <gtest/gtest.h>

#include "Algorithm_interact/GameRuleManager.h"
#include "Algorithm_interact/StrategyService.h"
#include "Algorithm_interact/WorldStateEngine.h"

TEST(GameRuleManagerTests, ReturnsAllRulesSortedByPriority) {
    strategy::GameRuleManager manager;

    const strategy::GameRules rules = manager.GetRulesByCategory("all");
    ASSERT_GE(rules.rules.size(), 3u);
    EXPECT_GE(rules.rules[0].priority, rules.rules[1].priority);
}

TEST(GameRuleManagerTests, CombatRuleConditionMatchesContext) {
    strategy::GameRuleManager manager;

    const std::unordered_map<std::string, std::string> context = {
        {"enemy_distance", "3"},
        {"player_health", "100"}
    };

    EXPECT_TRUE(manager.CheckRuleCondition("combat_start", context));
}

TEST(WorldStateEngineTests, ParsesStatePayloadIntoMaps) {
    strategy::WorldStateEngine engine;

    const bool ok = engine.UpdateWorldState(
        R"({
            "global_variables": {"player_level": 7, "combat_count": 2},
            "world_flags": {"tutorial_complete": true, "in_combat": false},
            "active_events": ["story_start"]
        })");

    ASSERT_TRUE(ok);
    const strategy::WorldState state = engine.GetCurrentWorldState();

    EXPECT_EQ(state.global_variables.at("player_level"), 7);
    EXPECT_EQ(state.global_variables.at("combat_count"), 2);
    EXPECT_TRUE(state.world_flags.at("tutorial_complete"));
    ASSERT_EQ(state.active_events.size(), 1u);
    EXPECT_EQ(state.active_events[0], "story_start");
}

TEST(StrategyServiceTests, TriggerCombatEventAppliesGlobalEffects) {
    strategy::StrategyService service;

    strategy::WorldStateUpdate update;
    update.world_state_json =
        R"({
            "global_variables": {"player_level": 6, "player_exp": 10},
            "world_flags": {"tutorial_complete": true}
        })";
    const strategy::WorldStateResult update_result = service.UpdateWorldState(update);

    ASSERT_TRUE(update_result.success);

    strategy::EventTriggerRequest request;
    request.event_id = 2001;
    request.params = {50};

    const strategy::EventTriggerResult event_result = service.TriggerEvent(request);

    EXPECT_TRUE(event_result.triggered);

    const strategy::GameState state = service.QueryGameState("global_vars");
    auto it = state.world_state.global_variables.find("combat_count");
    ASSERT_NE(it, state.world_state.global_variables.end());
    EXPECT_GE(it->second, 1);
}
