/*
 * test_character_stats.cpp
 * CharacterStats and CharacterStatsRegistry unit tests
 */
#include <gtest/gtest.h>
#include "CharacterStats.h"

using namespace algorithm;

// ============================================================================
// CharacterStats struct tests
// ============================================================================

TEST(CharacterStats, GetHpPercentFullHp) {
    CharacterStats stats;
    stats.max_hp = 200.0f;
    stats.current_hp = 200.0f;
    EXPECT_FLOAT_EQ(stats.getHpPercent(), 100.0f);
}

TEST(CharacterStats, GetHpPercentHalfHp) {
    CharacterStats stats;
    stats.max_hp = 200.0f;
    stats.current_hp = 100.0f;
    EXPECT_FLOAT_EQ(stats.getHpPercent(), 50.0f);
}

TEST(CharacterStats, GetHpPercentZeroMaxHp) {
    CharacterStats stats;
    stats.max_hp = 0.0f;
    stats.current_hp = 0.0f;
    EXPECT_FLOAT_EQ(stats.getHpPercent(), 0.0f);
}

TEST(CharacterStats, GetMpPercentFullMp) {
    CharacterStats stats;
    stats.max_mp = 100.0f;
    stats.current_mp = 100.0f;
    EXPECT_FLOAT_EQ(stats.getMpPercent(), 100.0f);
}

TEST(CharacterStats, GetMpPercentHalfMp) {
    CharacterStats stats;
    stats.max_mp = 100.0f;
    stats.current_mp = 50.0f;
    EXPECT_FLOAT_EQ(stats.getMpPercent(), 50.0f);
}

TEST(CharacterStats, GetMpPercentZeroMaxMp) {
    CharacterStats stats;
    stats.max_mp = 0.0f;
    stats.current_mp = 0.0f;
    EXPECT_FLOAT_EQ(stats.getMpPercent(), 0.0f);
}

// ============================================================================
// CharacterStatsRegistry tests
// ============================================================================

class CharacterStatsRegistryTest : public ::testing::Test {
protected:
    CharacterStatsRegistry registry;
};

TEST_F(CharacterStatsRegistryTest, GetDefaultStatsWarrior) {
    auto stats = registry.getDefaultStats(Profession::Warrior);
    EXPECT_EQ(stats.profession, Profession::Warrior);
    EXPECT_GT(stats.max_hp, 0.0f);
    EXPECT_GT(stats.attack, 0.0f);
    // Warrior should have high HP and high physical attack
    EXPECT_GE(stats.max_hp, 200.0f);
    EXPECT_GE(stats.attack, 25.0f);
}

TEST_F(CharacterStatsRegistryTest, GetDefaultStatsMage) {
    auto stats = registry.getDefaultStats(Profession::Mage);
    EXPECT_EQ(stats.profession, Profession::Mage);
    // Mage should have high magic attack and high MP
    EXPECT_GE(stats.magic_attack, 28.0f);
    EXPECT_GE(stats.max_mp, 100.0f);
}

TEST_F(CharacterStatsRegistryTest, GetDefaultStatsArcher) {
    auto stats = registry.getDefaultStats(Profession::Archer);
    EXPECT_EQ(stats.profession, Profession::Archer);
    // Archer should have high speed and high crit rate
    EXPECT_GE(stats.speed, 18.0f);
    EXPECT_GE(stats.crit_rate, 0.12f);
}

TEST_F(CharacterStatsRegistryTest, ElementMultiplierFireBeatsGrass) {
    float mult = registry.getElementMultiplier(Element::Fire, Element::Grass);
    EXPECT_FLOAT_EQ(mult, 2.0f);
}

TEST_F(CharacterStatsRegistryTest, ElementMultiplierWaterBeatsFire) {
    float mult = registry.getElementMultiplier(Element::Water, Element::Fire);
    EXPECT_FLOAT_EQ(mult, 2.0f);
}

TEST_F(CharacterStatsRegistryTest, ElementMultiplierGrassBeatsWater) {
    float mult = registry.getElementMultiplier(Element::Grass, Element::Water);
    EXPECT_FLOAT_EQ(mult, 2.0f);
}

TEST_F(CharacterStatsRegistryTest, ElementMultiplierSameElement) {
    EXPECT_FLOAT_EQ(registry.getElementMultiplier(Element::Fire, Element::Fire), 0.75f);
    EXPECT_FLOAT_EQ(registry.getElementMultiplier(Element::Water, Element::Water), 0.75f);
    EXPECT_FLOAT_EQ(registry.getElementMultiplier(Element::Grass, Element::Grass), 0.75f);
}

TEST_F(CharacterStatsRegistryTest, ElementMultiplierNoneIsNeutral) {
    EXPECT_FLOAT_EQ(registry.getElementMultiplier(Element::None, Element::Fire), 1.0f);
    EXPECT_FLOAT_EQ(registry.getElementMultiplier(Element::Fire, Element::None), 1.0f);
    EXPECT_FLOAT_EQ(registry.getElementMultiplier(Element::None, Element::None), 1.0f);
}

TEST_F(CharacterStatsRegistryTest, ElementMultiplierLightDarkMutual) {
    EXPECT_FLOAT_EQ(registry.getElementMultiplier(Element::Light, Element::Dark), 2.0f);
    EXPECT_FLOAT_EQ(registry.getElementMultiplier(Element::Dark, Element::Light), 2.0f);
}

TEST_F(CharacterStatsRegistryTest, RegisterAndRetrieveCharacter) {
    CharacterStats stats;
    stats.character_id = 42;
    stats.name = "TestHero";
    stats.max_hp = 500.0f;
    stats.current_hp = 500.0f;

    registry.registerCharacter(stats);

    const CharacterStats* retrieved = registry.getCharacterStats(42);
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->character_id, 42);
    EXPECT_EQ(retrieved->name, "TestHero");
    EXPECT_FLOAT_EQ(retrieved->max_hp, 500.0f);
}

TEST_F(CharacterStatsRegistryTest, GetCharacterStatsReturnsNullForUnknown) {
    EXPECT_EQ(registry.getCharacterStats(99999), nullptr);
}

TEST_F(CharacterStatsRegistryTest, ScaleByLevelIncreasesStats) {
    auto base = registry.getDefaultStats(Profession::Warrior, 1);
    auto scaled = CharacterStatsRegistry::scaleByLevel(base, 10);

    EXPECT_EQ(scaled.level, 10);
    EXPECT_GT(scaled.max_hp, base.max_hp);
    EXPECT_GT(scaled.attack, base.attack);
    EXPECT_GT(scaled.defense, base.defense);
    EXPECT_GT(scaled.magic_attack, base.magic_attack);
    EXPECT_GT(scaled.magic_defense, base.magic_defense);
    EXPECT_GT(scaled.speed, base.speed);
}

TEST_F(CharacterStatsRegistryTest, ScaleByLevelOneReturnsSame) {
    auto base = registry.getDefaultStats(Profession::Mage, 1);
    auto scaled = CharacterStatsRegistry::scaleByLevel(base, 1);

    EXPECT_FLOAT_EQ(scaled.max_hp, base.max_hp);
    EXPECT_FLOAT_EQ(scaled.attack, base.attack);
}
