/*
 * 文件名: PasswordHasher.h
 * 说明: 密码哈希工具，使用SHA-256和随机盐值
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#ifndef STRATEGY_SECURITY_PASSWORDHASHER_H
#define STRATEGY_SECURITY_PASSWORDHASHER_H

#include <string>

namespace strategy {

/**
 * @brief 密码哈希工具类
 *
 * 使用SHA-256算法和随机盐值对密码进行安全哈希。
 * 存储格式: "salt:hash" (32位hex盐 + ":" + 64位hex哈希 = 97字符)
 */
class PasswordHasher {
public:
    /**
     * @brief 对密码进行哈希
     * @param password 明文密码
     * @return "salt:hash" 格式的哈希字符串
     */
    static std::string hashPassword(const std::string& password);

    /**
     * @brief 验证密码是否匹配
     * @param password 明文密码
     * @param stored 存储的 "salt:hash" 格式字符串
     * @return 密码是否匹配
     */
    static bool verifyPassword(const std::string& password, const std::string& stored);

private:
    static std::string generateSalt();
    static std::string hashWithSalt(const std::string& salt, const std::string& password);
    static bool constantTimeCompare(const std::string& a, const std::string& b);
};

} // namespace strategy

#endif // STRATEGY_SECURITY_PASSWORDHASHER_H
