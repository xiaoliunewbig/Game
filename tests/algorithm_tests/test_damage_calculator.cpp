/*
 * test_damage_calculator.cpp
 * DamageCalculator unit tests (through AlgorithmService)
 */
#include <gtest/gtest.h>
#include "AlgorithmService.h"

using namespace algorithm;

class DamageCalculatorTest : public ::testing::Test {
protected:
    AlgorithmService service;

    void SetUp() override {
        // Register a Fire Warrior attacker
        CharacterStats attacker;
        attacker.character_id = 1;
        attacker.name = "FireWarrior";
        attacker.profession = Profession::Warrior;
        attacker.element = Element::Fire;
        attacker.max_hp = 200.0f;
        attacker.current_hp = 200.0f;
        attacker.max_mp = 50.0f;
        attacker.current_mp = 50.0f;
        attacker.attack = 30.0f;
        attacker.defense = 15.0f;
        attacker.magic_attack = 5.0f;
        attacker.magic_defense = 8.0f;
        attacker.crit_rate = 0.1f;
        attacker.crit_damage = 0.5f;
        service.RegisterCharacter(attacker);

        // Register a Grass Mage defender
        CharacterStats defender;
        defender.character_id = 2;
        defender.name = "GrassMage";
        defender.profession = Profession::Mage;
        defender.element = Element::Grass;
        defender.max_hp = 120.0f;
        defender.current_hp = 120.0f;
        defender.max_mp = 100.0f;
        defender.current_mp = 100.0f;
        defender.attack = 5.0f;
        defender.defense = 6.0f;
        defender.magic_attack = 28.0f;
        defender.magic_defense = 18.0f;
        defender.crit_rate = 0.05f;
        defender.crit_damage = 0.7f;
        service.RegisterCharacter(defender);
    }
};

TEST_F(DamageCalculatorTest, CalculateDamageReturnsPositive) {
    DamageRequest req;
    req.attacker_id = 1;
    req.defender_id = 2;
    req.skill_id = 1;  // Use basic attack skill

    auto result = service.CalculateDamage(req);
    EXPECT_GT(result.damage, 0);
}

TEST_F(DamageCalculatorTest, DamageResultHasEffectDescription) {
    DamageRequest req;
    req.attacker_id = 1;
    req.defender_id = 2;
    req.skill_id = 1;

    auto result = service.CalculateDamage(req);
    EXPECT_FALSE(result.effect.empty());
}

TEST_F(DamageCalculatorTest, ExtendedDamageFireVsGrassHasElementAdvantage) {
    ExtendedDamageRequest ext;
    ext.attacker = *service.GetCharacterStats(1);
    ext.defender = *service.GetCharacterStats(2);
    ext.skill_id = 1;
    ext.skill_level = 1;

    auto result = service.CalculateExtendedDamage(ext);
    // Fire vs Grass should give element multiplier of 2.0
    EXPECT_GT(result.element_multiplier, 1.0f);
}

TEST_F(DamageCalculatorTest, UnregisteredAttackerReturnsZeroDamage) {
    DamageRequest req;
    req.attacker_id = 999;  // Not registered
    req.defender_id = 2;
    req.skill_id = 1;

    auto result = service.CalculateDamage(req);
    EXPECT_EQ(result.damage, 0);
}

TEST_F(DamageCalculatorTest, SameAttackerDefenderReturnsZeroDamage) {
    DamageRequest req;
    req.attacker_id = 1;
    req.defender_id = 1;  // Same as attacker
    req.skill_id = 1;

    auto result = service.CalculateDamage(req);
    EXPECT_EQ(result.damage, 0);
}
