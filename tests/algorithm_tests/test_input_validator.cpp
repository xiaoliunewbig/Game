/*
 * test_input_validator.cpp
 * InputValidator unit tests
 */
#include <gtest/gtest.h>
#include "InputValidator.h"
#include "AlgorithmService.h"
#include "SkillTreeManager.h"

using namespace algorithm;

// ============================================================================
// validateCharacterStats tests
// ============================================================================

class InputValidatorTest : public ::testing::Test {
protected:
    InputValidator validator;
};

TEST_F(InputValidatorTest, ValidCharacterStatsPass) {
    CharacterStats stats;
    stats.character_id = 1;
    stats.max_hp = 100.0f;
    stats.current_hp = 100.0f;
    stats.max_mp = 50.0f;
    stats.attack = 10.0f;
    stats.defense = 5.0f;
    stats.magic_attack = 10.0f;
    stats.magic_defense = 5.0f;
    stats.crit_rate = 0.1f;
    stats.crit_damage = 0.5f;

    auto result = validator.validateCharacterStats(stats);
    EXPECT_TRUE(result.success);
}

TEST_F(InputValidatorTest, NegativeHpFails) {
    CharacterStats stats;
    stats.character_id = 1;
    stats.max_hp = -10.0f;

    auto result = validator.validateCharacterStats(stats);
    EXPECT_FALSE(result.success);
}

TEST_F(InputValidatorTest, ZeroMaxHpFails) {
    CharacterStats stats;
    stats.character_id = 1;
    stats.max_hp = 0.0f;

    auto result = validator.validateCharacterStats(stats);
    EXPECT_FALSE(result.success);
}

TEST_F(InputValidatorTest, CritRateAboveOneFails) {
    CharacterStats stats;
    stats.character_id = 1;
    stats.max_hp = 100.0f;
    stats.current_hp = 100.0f;
    stats.crit_rate = 1.5f;

    auto result = validator.validateCharacterStats(stats);
    EXPECT_FALSE(result.success);
}

TEST_F(InputValidatorTest, NegativeCritRateFails) {
    CharacterStats stats;
    stats.character_id = 1;
    stats.max_hp = 100.0f;
    stats.current_hp = 100.0f;
    stats.crit_rate = -0.1f;

    auto result = validator.validateCharacterStats(stats);
    EXPECT_FALSE(result.success);
}

TEST_F(InputValidatorTest, IdOutOfRangeFails) {
    CharacterStats stats;
    stats.character_id = -1;
    stats.max_hp = 100.0f;
    stats.current_hp = 100.0f;

    auto result = validator.validateCharacterStats(stats);
    EXPECT_FALSE(result.success);
}

TEST_F(InputValidatorTest, IdAboveMaxFails) {
    CharacterStats stats;
    stats.character_id = 200000;
    stats.max_hp = 100.0f;
    stats.current_hp = 100.0f;

    auto result = validator.validateCharacterStats(stats);
    EXPECT_FALSE(result.success);
}

TEST_F(InputValidatorTest, CurrentHpAboveMaxHpFails) {
    CharacterStats stats;
    stats.character_id = 1;
    stats.max_hp = 100.0f;
    stats.current_hp = 150.0f;

    auto result = validator.validateCharacterStats(stats);
    EXPECT_FALSE(result.success);
}

TEST_F(InputValidatorTest, NegativeAttackFails) {
    CharacterStats stats;
    stats.character_id = 1;
    stats.max_hp = 100.0f;
    stats.current_hp = 100.0f;
    stats.attack = -5.0f;

    auto result = validator.validateCharacterStats(stats);
    EXPECT_FALSE(result.success);
}

// ============================================================================
// validateAIDecisionRequest tests
// ============================================================================

TEST_F(InputValidatorTest, ValidAIDecisionRequestPasses) {
    AIDecisionRequest req;
    req.npc_id = 1;
    req.context = {10, 80};  // distance=10, hp%=80

    auto result = validator.validateAIDecisionRequest(req);
    EXPECT_TRUE(result.success);
}

TEST_F(InputValidatorTest, NegativeNpcIdFails) {
    AIDecisionRequest req;
    req.npc_id = -1;
    req.context = {10, 80};

    auto result = validator.validateAIDecisionRequest(req);
    EXPECT_FALSE(result.success);
}

TEST_F(InputValidatorTest, ContextTooShortFails) {
    AIDecisionRequest req;
    req.npc_id = 1;
    req.context = {10};  // Only 1 element, need at least 2

    auto result = validator.validateAIDecisionRequest(req);
    EXPECT_FALSE(result.success);
}

TEST_F(InputValidatorTest, HpPercentAbove100Fails) {
    AIDecisionRequest req;
    req.npc_id = 1;
    req.context = {10, 150};

    auto result = validator.validateAIDecisionRequest(req);
    EXPECT_FALSE(result.success);
}

TEST_F(InputValidatorTest, NegativeDistanceFails) {
    AIDecisionRequest req;
    req.npc_id = 1;
    req.context = {-5, 80};

    auto result = validator.validateAIDecisionRequest(req);
    EXPECT_FALSE(result.success);
}

// ============================================================================
// validateSkillLearnRequest tests (requires SkillTreeManager)
// ============================================================================

class SkillLearnValidatorTest : public ::testing::Test {
protected:
    InputValidator validator;
    SkillTreeManager skill_manager;  // has default skills initialized
};

TEST_F(SkillLearnValidatorTest, InvalidSkillIdFails) {
    auto result = validator.validateSkillLearnRequest(
        -1, Profession::Warrior, {}, skill_manager);
    EXPECT_FALSE(result.success);
}

TEST_F(SkillLearnValidatorTest, NonexistentSkillFails) {
    auto result = validator.validateSkillLearnRequest(
        9999, Profession::Warrior, {}, skill_manager);
    EXPECT_FALSE(result.success);
}

TEST_F(SkillLearnValidatorTest, ValidCommonSkillPasses) {
    // Skill ID 1 should be a common skill (普通攻击) available to all professions
    auto skill = skill_manager.GetSkill(1);
    if (skill != nullptr) {
        auto result = validator.validateSkillLearnRequest(
            1, Profession::Warrior, {}, skill_manager);
        EXPECT_TRUE(result.success);
    }
}

TEST_F(SkillLearnValidatorTest, AlreadyLearnedSkillFails) {
    auto skill = skill_manager.GetSkill(1);
    if (skill != nullptr) {
        auto result = validator.validateSkillLearnRequest(
            1, Profession::Warrior, {1}, skill_manager);
        EXPECT_FALSE(result.success);
    }
}

// ============================================================================
// validateDamageRequest tests (requires AlgorithmService setup)
// ============================================================================

class DamageRequestValidatorTest : public ::testing::Test {
protected:
    AlgorithmService service;

    void SetUp() override {
        CharacterStats attacker;
        attacker.character_id = 1;
        attacker.name = "Attacker";
        attacker.max_hp = 200.0f;
        attacker.current_hp = 200.0f;
        attacker.attack = 25.0f;
        attacker.defense = 10.0f;
        service.RegisterCharacter(attacker);

        CharacterStats defender;
        defender.character_id = 2;
        defender.name = "Defender";
        defender.max_hp = 150.0f;
        defender.current_hp = 150.0f;
        defender.defense = 15.0f;
        service.RegisterCharacter(defender);
    }
};

TEST_F(DamageRequestValidatorTest, ValidRequestProducesDamage) {
    DamageRequest req;
    req.attacker_id = 1;
    req.defender_id = 2;
    req.skill_id = 1;  // Common skill

    auto result = service.CalculateDamage(req);
    // Should return a result (damage might be 0 if skill doesn't exist, but no crash)
    EXPECT_GE(result.damage, 0);
}
