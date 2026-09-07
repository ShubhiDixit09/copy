#ifndef DHARTI_UTILS_SHA256_HPP
#define DHARTI_UTILS_SHA256_HPP

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <cstring>

namespace dharti {
namespace utils {

/**
 * @brief Pure C++17 Standard SHA-256 Implementation (RFC 6234 compliant).
 * Completely self-contained with zero external dependencies.
 */
class SHA256 {
public:
    SHA256() { reset(); }

    void reset() {
        m_len = 0;
        m_tot_len = 0;
        m_h[0] = 0x6a09e667;
        m_h[1] = 0xbb67ae85;
        m_h[2] = 0x3c6ef372;
        m_h[3] = 0xa54ff53a;
        m_h[4] = 0x510e527f;
        m_h[5] = 0x9b05688c;
        m_h[6] = 0x1f83d9ab;
        m_h[7] = 0x5be0cd19;
    }

    void update(const uint8_t* message, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            m_block[m_len++] = message[i];
            if (m_len == 64) {
                transform();
                m_tot_len += 64;
                m_len = 0;
            }
        }
    }

    void update(const std::string& str) {
        update(reinterpret_cast<const uint8_t*>(str.data()), str.size());
    }

    std::string finalize() {
        uint8_t padded[64];
        memset(padded, 0, 64);
        m_tot_len += m_len;

        m_block[m_len++] = 0x80;
        if (m_len > 56) {
            while (m_len < 64) m_block[m_len++] = 0x00;
            transform();
            memset(m_block, 0, 56);
        } else {
            while (m_len < 56) m_block[m_len++] = 0x00;
        }

        uint64_t total_bits = m_tot_len * 8;
        for (int i = 7; i >= 0; --i) {
            m_block[56 + (7 - i)] = static_cast<uint8_t>((total_bits >> (i * 8)) & 0xFF);
        }
        transform();

        std::stringstream ss;
        for (int i = 0; i < 8; ++i) {
            ss << std::hex << std::setfill('0') << std::setw(8) << m_h[i];
        }
        reset();
        return ss.str();
    }

    static std::string hash_string(const std::string& input) {
        SHA256 hasher;
        hasher.update(input);
        return hasher.finalize();
    }

private:
    static inline uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
    static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
    static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
    static inline uint32_t sig0(uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
    static inline uint32_t sig1(uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
    static inline uint32_t theta0(uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
    static inline uint32_t theta1(uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

    void transform() {
        static const uint32_t K[64] = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
        };

        uint32_t w[64];
        for (int i = 0; i < 16; ++i) {
            w[i] = (static_cast<uint32_t>(m_block[i * 4]) << 24) |
                   (static_cast<uint32_t>(m_block[i * 4 + 1]) << 16) |
                   (static_cast<uint32_t>(m_block[i * 4 + 2]) << 8) |
                   (static_cast<uint32_t>(m_block[i * 4 + 3]));
        }
        for (int i = 16; i < 64; ++i) {
            w[i] = theta1(w[i - 2]) + w[i - 7] + theta0(w[i - 15]) + w[i - 16];
        }

        uint32_t a = m_h[0], b = m_h[1], c = m_h[2], d = m_h[3];
        uint32_t e = m_h[4], f = m_h[5], g = m_h[6], h = m_h[7];

        for (int i = 0; i < 64; ++i) {
            uint32_t t1 = h + sig1(e) + ch(e, f, g) + K[i] + w[i];
            uint32_t t2 = sig0(a) + maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }

        m_h[0] += a; m_h[1] += b; m_h[2] += c; m_h[3] += d;
        m_h[4] += e; m_h[5] += f; m_h[6] += g; m_h[7] += h;
    }

    uint32_t m_h[8];
    uint8_t m_block[64];
    size_t m_len{0};
    uint64_t m_tot_len{0};
};

} // namespace utils
} // namespace dharti

#endif // DHARTI_UTILS_SHA256_HPP
