/*
 * 文件名: SHA256.h
 * 说明: 纯C++ SHA-256哈希实现 (FIPS 180-4)
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#ifndef STRATEGY_SECURITY_SHA256_H
#define STRATEGY_SECURITY_SHA256_H

#include <string>
#include <array>
#include <cstdint>

namespace strategy {

/**
 * @brief SHA-256哈希算法实现
 *
 * 遵循FIPS 180-4标准的纯C++实现，无外部依赖。
 */
class SHA256 {
public:
    /**
     * @brief 计算输入数据的SHA-256哈希值
     * @param input 输入字符串
     * @return 64字符十六进制哈希字符串
     */
    static std::string hash(const std::string& input);

private:
    static constexpr size_t BLOCK_SIZE = 64;
    static constexpr size_t HASH_SIZE = 32;

    static const std::array<uint32_t, 64> K;

    static uint32_t rotr(uint32_t x, uint32_t n);
    static uint32_t ch(uint32_t x, uint32_t y, uint32_t z);
    static uint32_t maj(uint32_t x, uint32_t y, uint32_t z);
    static uint32_t sigma0(uint32_t x);
    static uint32_t sigma1(uint32_t x);
    static uint32_t gamma0(uint32_t x);
    static uint32_t gamma1(uint32_t x);

    static void processBlock(const uint8_t* block, std::array<uint32_t, 8>& state);
    static std::string toHex(const std::array<uint32_t, 8>& state);
};

} // namespace strategy

#endif // STRATEGY_SECURITY_SHA256_H
