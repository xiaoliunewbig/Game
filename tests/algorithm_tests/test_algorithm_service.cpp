/*
 * test_algorithm_service.cpp
 * AlgorithmService integration-level unit tests
 */
#include <gtest/gtest.h>
#include "AlgorithmService.h"

using namespace algorithm;

class AlgorithmServiceTest : public ::testing::Test {
protected:
    AlgorithmService service;
};

// ============================================================================
// Character registration and retrieval
// ============================================================================

TEST_F(AlgorithmServiceTest, RegisterAndRetrieveCharacter) {
    CharacterStats stats;
    stats.character_id = 10;
    stats.name = "TestWarrior";
    stats.profession = Profession::Warrior;
    stats.max_hp = 250.0f;
    stats.current_hp = 250.0f;
    stats.attack = 30.0f;
    stats.defense = 20.0f;

    service.RegisterCharacter(stats);

    const CharacterStats* retrieved = service.GetCharacterStats(10);
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->character_id, 10);
    EXPECT_EQ(retrieved->name, "TestWarrior");
    EXPECT_FLOAT_EQ(retrieved->max_hp, 250.0f);
}

TEST_F(AlgorithmServiceTest, GetCharacterStatsReturnsNullForUnregistered) {
    EXPECT_EQ(service.GetCharacterStats(9999), nullptr);
}

// ============================================================================
// Default stats
// ============================================================================

TEST_F(AlgorithmServiceTest, GetDefaultStatsWarrior) {
    auto stats = service.GetDefaultStats(Profession::Warrior);
    EXPECT_EQ(stats.profession, Profession::Warrior);
    EXPECT_GT(stats.max_hp, 0.0f);
    EXPECT_GT(stats.attack, 0.0f);
}

TEST_F(AlgorithmServiceTest, GetDefaultStatsMage) {
    auto stats = service.GetDefaultStats(Profession::Mage);
    EXPECT_EQ(stats.profession, Profession::Mage);
    EXPECT_GT(stats.magic_attack, 0.0f);
    EXPECT_GT(stats.max_mp, 0.0f);
}

TEST_F(AlgorithmServiceTest, GetDefaultStatsArcher) {
    auto stats = service.GetDefaultStats(Profession::Archer);
    EXPECT_EQ(stats.profession, Profession::Archer);
    EXPECT_GT(stats.speed, 0.0f);
}

TEST_F(AlgorithmServiceTest, GetDefaultStatsWithLevel) {
    auto lvl1 = service.GetDefaultStats(Profession::Warrior, 1);
    auto lvl10 = service.GetDefaultStats(Profession::Warrior, 10);
    EXPECT_GT(lvl10.max_hp, lvl1.max_hp);
    EXPECT_GT(lvl10.attack, lvl1.attack);
}

// ============================================================================
// AI Decision
// ============================================================================

TEST_F(AlgorithmServiceTest, MakeAIDecisionReturnsValidAction) {
    service.RegisterNPCType(1, NPCType::Warrior);

    AIDecisionRequest req;
    req.npc_id = 1;
    req.context = {5, 80, 1, 5};  // near, 80% hp, in combat, medium threat

    auto result = service.MakeAIDecision(req);
    EXPECT_GE(result.action_id, 0);
    EXPECT_LE(result.action_id, 6);
    EXPECT_FALSE(result.description.empty());
}

TEST_F(AlgorithmServiceTest, MakeAIDecisionMageType) {
    service.RegisterNPCType(2, NPCType::Mage);

    AIDecisionRequest req;
    req.npc_id = 2;
    req.context = {15, 60, 1, 7};

    auto result = service.MakeAIDecision(req);
    EXPECT_GE(result.action_id, 0);
    EXPECT_LE(result.action_id, 6);
}

TEST_F(AlgorithmServiceTest, MakeAIDecisionLowHp) {
    service.RegisterNPCType(3, NPCType::Archer);

    AIDecisionRequest req;
    req.npc_id = 3;
    req.context = {20, 15, 1, 3};  // far, very low hp

    auto result = service.MakeAIDecision(req);
    EXPECT_GE(result.action_id, 0);
    EXPECT_LE(result.action_id, 6);
}

TEST_F(AlgorithmServiceTest, MakeAIDecisionInvalidRequestReturnsDefault) {
    AIDecisionRequest req;
    req.npc_id = -1;
    req.context = {10, 80};

    auto result = service.MakeAIDecision(req);
    EXPECT_EQ(result.action_id, 0);  // default/idle on validation failure
}

// ============================================================================
// Element multiplier
// ============================================================================

TEST_F(AlgorithmServiceTest, ElementMultiplierFireBeatsGrass) {
    EXPECT_FLOAT_EQ(service.GetElementMultiplier(Element::Fire, Element::Grass), 2.0f);
}

TEST_F(AlgorithmServiceTest, ElementMultiplierWaterBeatsFire) {
    EXPECT_FLOAT_EQ(service.GetElementMultiplier(Element::Water, Element::Fire), 2.0f);
}

TEST_F(AlgorithmServiceTest, ElementMultiplierSameElement) {
    EXPECT_FLOAT_EQ(service.GetElementMultiplier(Element::Fire, Element::Fire), 0.75f);
}

TEST_F(AlgorithmServiceTest, ElementMultiplierNeutral) {
    EXPECT_FLOAT_EQ(service.GetElementMultiplier(Element::None, Element::None), 1.0f);
}

// ============================================================================
// Skill cooldown flow
// ============================================================================

TEST_F(AlgorithmServiceTest, SkillInitiallyReady) {
    EXPECT_TRUE(service.IsSkillReady(1, 100));
}

TEST_F(AlgorithmServiceTest, SkillNotReadyAfterCooldownStart) {
    service.StartSkillCooldown(1, 100, 5000);
    EXPECT_FALSE(service.IsSkillReady(1, 100));
}

TEST_F(AlgorithmServiceTest, SkillReadyAfterCooldownExpires) {
    service.StartSkillCooldown(1, 100, 3000);
    service.TickCooldowns(3000);
    EXPECT_TRUE(service.IsSkillReady(1, 100));
}

TEST_F(AlgorithmServiceTest, SkillNotReadyDuringCooldown) {
    service.StartSkillCooldown(1, 100, 5000);
    service.TickCooldowns(2000);
    EXPECT_FALSE(service.IsSkillReady(1, 100));
}

// ============================================================================
// Status effect management
// ============================================================================

TEST_F(AlgorithmServiceTest, NoStatusEffectsInitially) {
    auto effects = service.GetStatusEffects(1);
    EXPECT_TRUE(effects.empty());
}

TEST_F(AlgorithmServiceTest, AddAndGetStatusEffect) {
    StatusEffect burn = StatusEffectHelper::createBurn(20.0f, 3);
    service.AddStatusEffect(1, burn);

    auto effects = service.GetStatusEffects(1);
    ASSERT_EQ(effects.size(), 1u);
    EXPECT_EQ(effects[0].type, StatusEffectType::Burn);
    EXPECT_EQ(effects[0].duration_turns, 3);
}

TEST_F(AlgorithmServiceTest, AddMultipleStatusEffects) {
    service.AddStatusEffect(1, StatusEffectHelper::createBurn(20.0f, 3));
    service.AddStatusEffect(1, StatusEffectHelper::createPoison(0.05f, 4));

    auto effects = service.GetStatusEffects(1);
    EXPECT_EQ(effects.size(), 2u);
}

TEST_F(AlgorithmServiceTest, TickStatusEffectsDecrements) {
    service.AddStatusEffect(1, StatusEffectHelper::createBurn(20.0f, 2));
    service.TickStatusEffects(1);

    auto effects = service.GetStatusEffects(1);
    ASSERT_EQ(effects.size(), 1u);
    EXPECT_EQ(effects[0].duration_turns, 1);
}

TEST_F(AlgorithmServiceTest, TickStatusEffectsRemovesExpired) {
    service.AddStatusEffect(1, StatusEffectHelper::createBurn(20.0f, 1));
    service.TickStatusEffects(1);

    auto effects = service.GetStatusEffects(1);
    EXPECT_TRUE(effects.empty());
}

TEST_F(AlgorithmServiceTest, ClearStatusEffects) {
    service.AddStatusEffect(1, StatusEffectHelper::createBurn(20.0f, 5));
    service.AddStatusEffect(1, StatusEffectHelper::createPoison(0.05f, 5));

    service.ClearStatusEffects(1);

    auto effects = service.GetStatusEffects(1);
    EXPECT_TRUE(effects.empty());
}

TEST_F(AlgorithmServiceTest, StatusEffectsIsolatedPerCharacter) {
    service.AddStatusEffect(1, StatusEffectHelper::createBurn(20.0f, 3));
    service.AddStatusEffect(2, StatusEffectHelper::createPoison(0.05f, 4));

    EXPECT_EQ(service.GetStatusEffects(1).size(), 1u);
    EXPECT_EQ(service.GetStatusEffects(2).size(), 1u);
    EXPECT_EQ(service.GetStatusEffects(1)[0].type, StatusEffectType::Burn);
    EXPECT_EQ(service.GetStatusEffects(2)[0].type, StatusEffectType::Poison);
}

// ============================================================================
// Skill validation
// ============================================================================

TEST_F(AlgorithmServiceTest, ValidateSkillLearnInvalidSkillFails) {
    auto result = service.ValidateSkillLearn(9999, Profession::Warrior, {});
    EXPECT_FALSE(result.success);
}
