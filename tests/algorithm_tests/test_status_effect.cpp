/*
 * test_status_effect.cpp
 * StatusEffect and StatusEffectHelper unit tests
 */
#include <gtest/gtest.h>
#include "StatusEffect.h"

using namespace algorithm;

// ============================================================================
// StatusEffect struct tests
// ============================================================================

TEST(StatusEffect, AttackUpMultiplier) {
    StatusEffect e;
    e.type = StatusEffectType::AttackUp;
    e.magnitude = 0.3f;
    EXPECT_FLOAT_EQ(e.getAttackMultiplier(), 1.3f);
}

TEST(StatusEffect, NonBuffAttackMultiplier) {
    StatusEffect e;
    e.type = StatusEffectType::Burn;
    e.magnitude = 0.3f;
    EXPECT_FLOAT_EQ(e.getAttackMultiplier(), 1.0f);
}

TEST(StatusEffect, DefenseDownMultiplier) {
    StatusEffect e;
    e.type = StatusEffectType::DefenseDown;
    e.magnitude = 0.3f;
    EXPECT_FLOAT_EQ(e.getDefenseMultiplier(), 0.7f);
}

TEST(StatusEffect, NonDebuffDefenseMultiplier) {
    StatusEffect e;
    e.type = StatusEffectType::AttackUp;
    e.magnitude = 0.3f;
    EXPECT_FLOAT_EQ(e.getDefenseMultiplier(), 1.0f);
}

TEST(StatusEffect, BurnDamageOverTimeIsFixed) {
    StatusEffect e;
    e.type = StatusEffectType::Burn;
    e.magnitude = 25.0f;
    EXPECT_FLOAT_EQ(e.getDamageOverTime(1000.0f), 25.0f);
}

TEST(StatusEffect, PoisonDamageOverTimeIsPercentage) {
    StatusEffect e;
    e.type = StatusEffectType::Poison;
    e.magnitude = 0.05f;
    EXPECT_FLOAT_EQ(e.getDamageOverTime(1000.0f), 50.0f);
}

TEST(StatusEffect, BleedDamageOverTimeIsFixed) {
    StatusEffect e;
    e.type = StatusEffectType::Bleed;
    e.magnitude = 15.0f;
    EXPECT_FLOAT_EQ(e.getDamageOverTime(1000.0f), 15.0f);
}

TEST(StatusEffect, NoDamageOverTimeForNone) {
    StatusEffect e;
    e.type = StatusEffectType::None;
    EXPECT_FLOAT_EQ(e.getDamageOverTime(1000.0f), 0.0f);
}

TEST(StatusEffect, StunPreventsAction) {
    StatusEffect e;
    e.type = StatusEffectType::Stun;
    EXPECT_TRUE(e.preventsAction());
}

TEST(StatusEffect, FreezePreventsAction) {
    StatusEffect e;
    e.type = StatusEffectType::Freeze;
    EXPECT_TRUE(e.preventsAction());
}

TEST(StatusEffect, BurnDoesNotPreventAction) {
    StatusEffect e;
    e.type = StatusEffectType::Burn;
    EXPECT_FALSE(e.preventsAction());
}

TEST(StatusEffect, AttackUpDoesNotPreventAction) {
    StatusEffect e;
    e.type = StatusEffectType::AttackUp;
    EXPECT_FALSE(e.preventsAction());
}

TEST(StatusEffect, TickDecrementsDuration) {
    StatusEffect e;
    e.duration_turns = 3;
    EXPECT_TRUE(e.tick());   // 3 -> 2, still active
    EXPECT_EQ(e.duration_turns, 2);
    EXPECT_TRUE(e.tick());   // 2 -> 1, still active
    EXPECT_EQ(e.duration_turns, 1);
    EXPECT_FALSE(e.tick());  // 1 -> 0, expired
    EXPECT_EQ(e.duration_turns, 0);
}

TEST(StatusEffect, TickAlreadyExpired) {
    StatusEffect e;
    e.duration_turns = 0;
    EXPECT_FALSE(e.tick());
    EXPECT_EQ(e.duration_turns, 0);
}

TEST(StatusEffect, RegenerationHealOverTime) {
    StatusEffect e;
    e.type = StatusEffectType::Regeneration;
    e.magnitude = 0.1f;
    EXPECT_FLOAT_EQ(e.getHealOverTime(500.0f), 50.0f);
}

TEST(StatusEffect, ShieldAmount) {
    StatusEffect e;
    e.type = StatusEffectType::Shield;
    e.magnitude = 100.0f;
    EXPECT_FLOAT_EQ(e.getShieldAmount(), 100.0f);
}

TEST(StatusEffect, FreezeSpeedMultiplier) {
    StatusEffect e;
    e.type = StatusEffectType::Freeze;
    e.magnitude = 0.5f;
    EXPECT_FLOAT_EQ(e.getSpeedMultiplier(), 0.5f);
}

// ============================================================================
// StatusEffectHelper tests
// ============================================================================

TEST(StatusEffectHelper, TotalAttackMultiplierMultipleBuffs) {
    std::vector<StatusEffect> effects;
    effects.push_back(StatusEffectHelper::createAttackUp(0.2f, 3));
    effects.push_back(StatusEffectHelper::createAttackUp(0.3f, 3));
    // (1.0 + 0.2) * (1.0 + 0.3) = 1.2 * 1.3 = 1.56
    EXPECT_NEAR(StatusEffectHelper::getTotalAttackMultiplier(effects), 1.56f, 0.001f);
}

TEST(StatusEffectHelper, TotalAttackMultiplierNoBuffs) {
    std::vector<StatusEffect> effects;
    effects.push_back(StatusEffectHelper::createBurn(10.0f, 3));
    EXPECT_FLOAT_EQ(StatusEffectHelper::getTotalAttackMultiplier(effects), 1.0f);
}

TEST(StatusEffectHelper, TotalDefenseMultiplier) {
    std::vector<StatusEffect> effects;
    effects.push_back(StatusEffectHelper::createDefenseDown(0.2f, 3));
    effects.push_back(StatusEffectHelper::createDefenseDown(0.3f, 3));
    // (1.0 - 0.2) * (1.0 - 0.3) = 0.8 * 0.7 = 0.56
    EXPECT_NEAR(StatusEffectHelper::getTotalDefenseMultiplier(effects), 0.56f, 0.001f);
}

TEST(StatusEffectHelper, IsControlledWithStun) {
    std::vector<StatusEffect> effects;
    effects.push_back(StatusEffectHelper::createBurn(10.0f, 3));
    effects.push_back(StatusEffectHelper::createStun(2));
    EXPECT_TRUE(StatusEffectHelper::isControlled(effects));
}

TEST(StatusEffectHelper, IsNotControlledWithoutCC) {
    std::vector<StatusEffect> effects;
    effects.push_back(StatusEffectHelper::createBurn(10.0f, 3));
    effects.push_back(StatusEffectHelper::createPoison(0.05f, 3));
    EXPECT_FALSE(StatusEffectHelper::isControlled(effects));
}

TEST(StatusEffectHelper, IsControlledWithFreeze) {
    std::vector<StatusEffect> effects;
    effects.push_back(StatusEffectHelper::createFreeze(0.5f, 2));
    EXPECT_TRUE(StatusEffectHelper::isControlled(effects));
}

TEST(StatusEffectHelper, TotalDamageOverTime) {
    std::vector<StatusEffect> effects;
    effects.push_back(StatusEffectHelper::createBurn(20.0f, 3));
    effects.push_back(StatusEffectHelper::createPoison(0.05f, 3));
    // burn=20 + poison=1000*0.05=50 = 70
    EXPECT_FLOAT_EQ(StatusEffectHelper::getTotalDamageOverTime(effects, 1000.0f), 70.0f);
}

TEST(StatusEffectHelper, TotalShieldAmount) {
    std::vector<StatusEffect> effects;
    effects.push_back(StatusEffectHelper::createShield(100.0f, 3));
    effects.push_back(StatusEffectHelper::createShield(50.0f, 2));
    EXPECT_FLOAT_EQ(StatusEffectHelper::getTotalShieldAmount(effects), 150.0f);
}

TEST(StatusEffectHelper, EmptyEffectsDefaultMultipliers) {
    std::vector<StatusEffect> empty;
    EXPECT_FLOAT_EQ(StatusEffectHelper::getTotalAttackMultiplier(empty), 1.0f);
    EXPECT_FLOAT_EQ(StatusEffectHelper::getTotalDefenseMultiplier(empty), 1.0f);
    EXPECT_FALSE(StatusEffectHelper::isControlled(empty));
    EXPECT_FLOAT_EQ(StatusEffectHelper::getTotalDamageOverTime(empty, 1000.0f), 0.0f);
    EXPECT_FLOAT_EQ(StatusEffectHelper::getTotalShieldAmount(empty), 0.0f);
}
