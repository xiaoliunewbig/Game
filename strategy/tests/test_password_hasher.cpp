/*
 * test_password_hasher.cpp
 * PasswordHasher unit tests
 */
#include <gtest/gtest.h>
#include "security/PasswordHasher.h"

using namespace strategy;

TEST(PasswordHasher, HashPasswordReturns97CharString) {
    std::string hashed = PasswordHasher::hashPassword("mypassword");
    // Format: 32-char hex salt + ":" + 64-char hex hash = 97 chars
    EXPECT_EQ(hashed.size(), 97u);
}

TEST(PasswordHasher, HashPasswordContainsSaltSeparator) {
    std::string hashed = PasswordHasher::hashPassword("mypassword");
    size_t sep = hashed.find(':');
    EXPECT_NE(sep, std::string::npos);
    EXPECT_EQ(sep, 32u);  // Salt is 32 hex chars
}

TEST(PasswordHasher, VerifyCorrectPassword) {
    std::string password = "correcthorse";
    std::string hashed = PasswordHasher::hashPassword(password);
    EXPECT_TRUE(PasswordHasher::verifyPassword(password, hashed));
}

TEST(PasswordHasher, VerifyWrongPassword) {
    std::string hashed = PasswordHasher::hashPassword("correcthorse");
    EXPECT_FALSE(PasswordHasher::verifyPassword("wrongpassword", hashed));
}

TEST(PasswordHasher, DifferentSaltsPerCall) {
    std::string hash1 = PasswordHasher::hashPassword("samepassword");
    std::string hash2 = PasswordHasher::hashPassword("samepassword");
    // Same password should produce different stored hashes due to random salt
    EXPECT_NE(hash1, hash2);
    // But both should verify correctly
    EXPECT_TRUE(PasswordHasher::verifyPassword("samepassword", hash1));
    EXPECT_TRUE(PasswordHasher::verifyPassword("samepassword", hash2));
}

TEST(PasswordHasher, VerifyMalformedStoredHashReturnsFalse) {
    // No colon separator
    EXPECT_FALSE(PasswordHasher::verifyPassword("password", "nocolonhere"));
}

TEST(PasswordHasher, VerifyEmptyStoredHashReturnsFalse) {
    EXPECT_FALSE(PasswordHasher::verifyPassword("password", ""));
}

TEST(PasswordHasher, EmptyPasswordCanBeHashedAndVerified) {
    std::string hashed = PasswordHasher::hashPassword("");
    EXPECT_EQ(hashed.size(), 97u);
    EXPECT_TRUE(PasswordHasher::verifyPassword("", hashed));
    EXPECT_FALSE(PasswordHasher::verifyPassword("notempty", hashed));
}

TEST(PasswordHasher, HashOutputIsHexOnly) {
    std::string hashed = PasswordHasher::hashPassword("test");
    // Check salt part (first 32 chars)
    std::string salt = hashed.substr(0, 32);
    for (char c : salt) {
        EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
            << "Non-hex char in salt: " << c;
    }
    // Check hash part (after colon, 64 chars)
    std::string hash = hashed.substr(33, 64);
    for (char c : hash) {
        EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
            << "Non-hex char in hash: " << c;
    }
}
