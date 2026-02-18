/*
 * 文件名: SHA256.cpp
 * 说明: SHA-256哈希算法实现 (FIPS 180-4)
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#include "security/SHA256.h"
#include <cstring>
#include <sstream>
#include <iomanip>

namespace strategy {

const std::array<uint32_t, 64> SHA256::K = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

uint32_t SHA256::rotr(uint32_t x, uint32_t n) {
    return (x >> n) | (x << (32 - n));
}

uint32_t SHA256::ch(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (~x & z);
}

uint32_t SHA256::maj(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

uint32_t SHA256::sigma0(uint32_t x) {
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

uint32_t SHA256::sigma1(uint32_t x) {
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

uint32_t SHA256::gamma0(uint32_t x) {
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

uint32_t SHA256::gamma1(uint32_t x) {
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

void SHA256::processBlock(const uint8_t* block, std::array<uint32_t, 8>& state) {
    std::array<uint32_t, 64> w{};

    for (int i = 0; i < 16; ++i) {
        w[i] = (static_cast<uint32_t>(block[i * 4]) << 24) |
               (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
               (static_cast<uint32_t>(block[i * 4 + 2]) << 8) |
               (static_cast<uint32_t>(block[i * 4 + 3]));
    }

    for (int i = 16; i < 64; ++i) {
        w[i] = gamma1(w[i - 2]) + w[i - 7] + gamma0(w[i - 15]) + w[i - 16];
    }

    uint32_t a = state[0];
    uint32_t b = state[1];
    uint32_t c = state[2];
    uint32_t d = state[3];
    uint32_t e = state[4];
    uint32_t f = state[5];
    uint32_t g = state[6];
    uint32_t h = state[7];

    for (int i = 0; i < 64; ++i) {
        uint32_t t1 = h + sigma1(e) + ch(e, f, g) + K[i] + w[i];
        uint32_t t2 = sigma0(a) + maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    state[5] += f;
    state[6] += g;
    state[7] += h;
}

std::string SHA256::toHex(const std::array<uint32_t, 8>& state) {
    std::ostringstream oss;
    for (uint32_t val : state) {
        oss << std::hex << std::setfill('0') << std::setw(8) << val;
    }
    return oss.str();
}

std::string SHA256::hash(const std::string& input) {
    std::array<uint32_t, 8> state = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    const auto* data = reinterpret_cast<const uint8_t*>(input.data());
    uint64_t bit_len = static_cast<uint64_t>(input.size()) * 8;
    size_t offset = 0;

    // Process complete blocks
    while (offset + BLOCK_SIZE <= input.size()) {
        processBlock(data + offset, state);
        offset += BLOCK_SIZE;
    }

    // Padding
    std::array<uint8_t, 128> padding{};
    size_t remaining = input.size() - offset;
    std::memcpy(padding.data(), data + offset, remaining);
    padding[remaining] = 0x80;

    size_t pad_len = (remaining < 56) ? 64 : 128;

    // Append length in bits (big-endian)
    for (int i = 0; i < 8; ++i) {
        padding[pad_len - 1 - i] = static_cast<uint8_t>(bit_len >> (i * 8));
    }

    // Process remaining blocks
    processBlock(padding.data(), state);
    if (pad_len == 128) {
        processBlock(padding.data() + 64, state);
    }

    return toHex(state);
}

} // namespace strategy
