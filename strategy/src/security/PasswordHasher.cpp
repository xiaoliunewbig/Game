/*
 * 文件名: PasswordHasher.cpp
 * 说明: 密码哈希工具实现
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#include "security/PasswordHasher.h"
#include "security/SHA256.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <cstdint>

namespace strategy {

std::string PasswordHasher::generateSalt() {
    std::random_device rd;
    std::ostringstream oss;
    for (int i = 0; i < 4; ++i) {
        uint32_t val = rd();
        oss << std::hex << std::setfill('0') << std::setw(8) << val;
    }
    return oss.str();
}

std::string PasswordHasher::hashWithSalt(const std::string& salt, const std::string& password) {
    return SHA256::hash(salt + password);
}

bool PasswordHasher::constantTimeCompare(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) {
        return false;
    }
    volatile uint8_t result = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        result |= static_cast<uint8_t>(a[i]) ^ static_cast<uint8_t>(b[i]);
    }
    return result == 0;
}

std::string PasswordHasher::hashPassword(const std::string& password) {
    std::string salt = generateSalt();
    std::string hash = hashWithSalt(salt, password);
    return salt + ":" + hash;
}

bool PasswordHasher::verifyPassword(const std::string& password, const std::string& stored) {
    size_t sep = stored.find(':');
    if (sep == std::string::npos) {
        return false;
    }
    std::string salt = stored.substr(0, sep);
    std::string expected_hash = stored.substr(sep + 1);
    std::string actual_hash = hashWithSalt(salt, password);
    return constantTimeCompare(expected_hash, actual_hash);
}

} // namespace strategy
