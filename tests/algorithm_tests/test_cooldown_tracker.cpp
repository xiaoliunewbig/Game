/*
 * test_cooldown_tracker.cpp
 * CooldownTracker unit tests
 */
#include <gtest/gtest.h>
#include "CooldownTracker.h"

using namespace algorithm;

class CooldownTrackerTest : public ::testing::Test {
protected:
    CooldownTracker tracker;
};

TEST_F(CooldownTrackerTest, IsReadyForUnknownCharacterAndSkill) {
    EXPECT_TRUE(tracker.isReady(1, 100));
}

TEST_F(CooldownTrackerTest, NotReadyAfterStartCooldown) {
    tracker.startCooldown(1, 100, 5000);
    EXPECT_FALSE(tracker.isReady(1, 100));
}

TEST_F(CooldownTrackerTest, OtherSkillStillReady) {
    tracker.startCooldown(1, 100, 5000);
    EXPECT_TRUE(tracker.isReady(1, 200));
}

TEST_F(CooldownTrackerTest, OtherCharacterStillReady) {
    tracker.startCooldown(1, 100, 5000);
    EXPECT_TRUE(tracker.isReady(2, 100));
}

TEST_F(CooldownTrackerTest, TickReducesCooldown) {
    tracker.startCooldown(1, 100, 5000);
    tracker.tick(3000);
    EXPECT_FALSE(tracker.isReady(1, 100));
    EXPECT_EQ(tracker.getRemainingCooldown(1, 100), 2000);
}

TEST_F(CooldownTrackerTest, TickMakesReadyWhenExpired) {
    tracker.startCooldown(1, 100, 3000);
    tracker.tick(3000);
    EXPECT_TRUE(tracker.isReady(1, 100));
}

TEST_F(CooldownTrackerTest, TickOvershootMakesReady) {
    tracker.startCooldown(1, 100, 3000);
    tracker.tick(5000);
    EXPECT_TRUE(tracker.isReady(1, 100));
    EXPECT_EQ(tracker.getRemainingCooldown(1, 100), 0);
}

TEST_F(CooldownTrackerTest, GetRemainingCooldownForUnknown) {
    EXPECT_EQ(tracker.getRemainingCooldown(1, 100), 0);
}

TEST_F(CooldownTrackerTest, GetRemainingCooldownAfterStart) {
    tracker.startCooldown(1, 100, 5000);
    EXPECT_EQ(tracker.getRemainingCooldown(1, 100), 5000);
}

TEST_F(CooldownTrackerTest, ResetCharacterCooldowns) {
    tracker.startCooldown(1, 100, 5000);
    tracker.startCooldown(1, 200, 3000);
    tracker.startCooldown(2, 100, 4000);

    tracker.resetCharacterCooldowns(1);

    EXPECT_TRUE(tracker.isReady(1, 100));
    EXPECT_TRUE(tracker.isReady(1, 200));
    EXPECT_FALSE(tracker.isReady(2, 100));
}

TEST_F(CooldownTrackerTest, ResetAll) {
    tracker.startCooldown(1, 100, 5000);
    tracker.startCooldown(2, 200, 3000);

    tracker.resetAll();

    EXPECT_TRUE(tracker.isReady(1, 100));
    EXPECT_TRUE(tracker.isReady(2, 200));
}

TEST_F(CooldownTrackerTest, GetCooldownSkillsListsOnlyActiveCooldowns) {
    tracker.startCooldown(1, 100, 5000);
    tracker.startCooldown(1, 200, 3000);
    tracker.startCooldown(1, 300, 1000);

    // Expire skill 300
    tracker.tick(1000);

    auto on_cooldown = tracker.getCooldownSkills(1);
    // Skills 100 and 200 should still be on cooldown
    EXPECT_EQ(on_cooldown.size(), 2u);

    bool has_100 = false;
    bool has_200 = false;
    for (int skill_id : on_cooldown) {
        if (skill_id == 100) has_100 = true;
        if (skill_id == 200) has_200 = true;
    }
    EXPECT_TRUE(has_100);
    EXPECT_TRUE(has_200);
}

TEST_F(CooldownTrackerTest, GetCooldownSkillsEmptyForUnknownCharacter) {
    auto on_cooldown = tracker.getCooldownSkills(999);
    EXPECT_TRUE(on_cooldown.empty());
}

TEST_F(CooldownTrackerTest, MultipleTicks) {
    tracker.startCooldown(1, 100, 10000);
    tracker.tick(2000);
    EXPECT_EQ(tracker.getRemainingCooldown(1, 100), 8000);
    tracker.tick(3000);
    EXPECT_EQ(tracker.getRemainingCooldown(1, 100), 5000);
    tracker.tick(5000);
    EXPECT_TRUE(tracker.isReady(1, 100));
}
