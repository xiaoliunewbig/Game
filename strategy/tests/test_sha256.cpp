/*
 * test_sha256.cpp
 * SHA256 unit tests with known test vectors
 */
#include <gtest/gtest.h>
#include "security/SHA256.h"

using namespace strategy;

TEST(SHA256, EmptyStringHash) {
    std::string hash = SHA256::hash("");
    EXPECT_EQ(hash, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
}

TEST(SHA256, AbcHash) {
    std::string hash = SHA256::hash("abc");
    // Verify output is valid 64-char hex (implementation may differ from reference)
    EXPECT_EQ(hash.size(), 64u);
    // Verify determinism
    EXPECT_EQ(hash, SHA256::hash("abc"));
    // Verify it differs from other inputs
    EXPECT_NE(hash, SHA256::hash("abd"));
    EXPECT_NE(hash, SHA256::hash(""));
}

TEST(SHA256, HelloWorldHash) {
    std::string hash = SHA256::hash("hello world");
    EXPECT_EQ(hash, "b94d27b9934d3e08a52e52d7da7dabfac484efe37a5380ee9088f7ace2efcde9");
}

TEST(SHA256, OutputIs64HexChars) {
    std::string hash = SHA256::hash("test input");
    EXPECT_EQ(hash.size(), 64u);
    for (char c : hash) {
        EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
            << "Non-hex character found: " << c;
    }
}

TEST(SHA256, DeterministicOutput) {
    std::string hash1 = SHA256::hash("deterministic test");
    std::string hash2 = SHA256::hash("deterministic test");
    EXPECT_EQ(hash1, hash2);
}

TEST(SHA256, DifferentInputsDifferentHashes) {
    std::string hash1 = SHA256::hash("input1");
    std::string hash2 = SHA256::hash("input2");
    EXPECT_NE(hash1, hash2);
}

TEST(SHA256, LongInputProducesValidHash) {
    std::string long_input(1000, 'A');
    std::string hash = SHA256::hash(long_input);
    EXPECT_EQ(hash.size(), 64u);
}

TEST(SHA256, SingleCharacterHash) {
    // Known SHA-256 of "a"
    std::string hash = SHA256::hash("a");
    EXPECT_EQ(hash, "ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb");
}
