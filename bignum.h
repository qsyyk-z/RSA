#pragma once
#include <cstdint>
#include <cstring>
#include <random>
#include <sstream>
#include <string>
#include <iostream>
using namespace std;

#define BLOCK_BITS 32
#define BLOCK_MASK 0xFFFFFFFFULL
#define ARRAY_SIZE 128  // 最多 4096 位

class BigNum {
public:
    uint32_t num[ARRAY_SIZE];
    int bit_len;

    BigNum() {
        bit_len = 0;
        memset(num, 0, sizeof(num));
    }

    BigNum(int val) {
        bit_len = 0;
        memset(num, 0, sizeof(num));
        num[0] = (uint32_t)(val >= 0 ? val : -val);
        bit_len = 32;
    }

    BigNum(uint64_t val) {
        bit_len = 0;
        memset(num, 0, sizeof(num));
        num[0] = (uint32_t)(val & BLOCK_MASK);
        num[1] = (uint32_t)(val >> 32);
        bit_len = 64;
    }

    BigNum(const string& hex_str) {
        bit_len = 0;
        memset(num, 0, sizeof(num));
        string s = hex_str;
        if (s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
            s = s.substr(2);
        while (!s.empty() && s[0] == '0') s.erase(s.begin());
        if (s.empty()) return;

        int block_idx = 0;
        for (int i = (int)s.size(); i > 0; i -= 8) {
            int start = max(0, i - 8);
            string block_str = s.substr(start, i - start);
            uint32_t val = (uint32_t)stoul(block_str, nullptr, 16);
            if (block_idx < ARRAY_SIZE)
                num[block_idx++] = val;
        }
        bit_len = (int)s.size() * 4;
    }

    BigNum(const char* s, int chars_num) {
        bit_len = 0;
        memset(num, 0, sizeof(num));
        for (int i = 0; i < chars_num; i++) {
            int block_idx = i / 4;
            int byte_pos = i % 4;
            if (block_idx < ARRAY_SIZE)
                num[block_idx] |= ((uint32_t)(unsigned char)s[i]) << (byte_pos * 8);
        }
    }

    static mt19937_64& getRng() {
        static mt19937_64 gen(42);
        return gen;
    }

    static void setRandSeed(uint64_t seed) { getRng().seed(seed); }

    int compare(const BigNum& b) const {
        for (int i = ARRAY_SIZE - 1; i >= 0; i--) {
            if (num[i] > b.num[i]) return 1;
            if (num[i] < b.num[i]) return -1;
        }
        return 0;
    }

    bool isZero() const {
        for (int i = 0; i < ARRAY_SIZE; i++)
            if (num[i] != 0) return false;
        return true;
    }

    bool operator==(const BigNum& b) const { return compare(b) == 0; }
    bool operator!=(const BigNum& b) const { return compare(b) != 0; }
    bool operator<(const BigNum& b) const { return compare(b) < 0; }
    bool operator>(const BigNum& b) const { return compare(b) > 0; }
    bool operator<=(const BigNum& b) const { return compare(b) <= 0; }
    bool operator>=(const BigNum& b) const { return compare(b) >= 0; }

    BigNum operator+(const BigNum& b) const {
        BigNum result;
        uint64_t carry = 0; // 进位
        for (int i = 0; i < ARRAY_SIZE; i++) {
            uint64_t sum = (uint64_t)num[i] + (uint64_t)b.num[i] + carry;
            result.num[i] = (uint32_t)(sum & BLOCK_MASK);
            carry = sum >> 32;
        }
        return result;
    }

    BigNum operator-(const BigNum& b) const {
        BigNum result;
        int64_t borrow = 0; // 借位
        for (int i = 0; i < ARRAY_SIZE; i++) {
            int64_t diff = (int64_t)num[i] - (int64_t)b.num[i] - borrow;
            if (diff < 0) { diff += (int64_t)(1ULL << 32); borrow = 1; }
            else borrow = 0;
            result.num[i] = (uint32_t)diff;
        }
        return result;
    }

    BigNum operator*(const BigNum& b) const {
        BigNum result;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            if (num[i] == 0) continue;
            uint64_t carry = 0;
            for (int j = 0; j < ARRAY_SIZE && i + j < ARRAY_SIZE; j++) {
                uint64_t prod = (uint64_t)num[i] * b.num[j] + result.num[i + j] + carry;
                result.num[i + j] = (uint32_t)(prod & BLOCK_MASK);
                carry = prod >> 32;
            }
        }
        return result;
    }

    void divmod(const BigNum& b, BigNum& q, BigNum& r) const { // 带余除法：a / b = q ... r （结果写入q，r）
        memset(q.num, 0, sizeof(q.num));
        memset(r.num, 0, sizeof(r.num));
        if (compare(b) < 0) { r = *this; return; }
        if (b.isZero()) return;

        int msb = findMSB();
        for (int i = msb; i >= 0; i--) {
            leftShift1(r);
            if (getBit(i)) r.num[0] |= 1;
            if (r.compare(b) >= 0) {
                r = r - b;
                q.setBit(i);
            }
        }
    }

    BigNum operator/(const BigNum& b) const {
        BigNum q, r; divmod(b, q, r); return q;
    }

    BigNum operator%(const BigNum& b) const {
        BigNum q, r; divmod(b, q, r); return r;
    }

    static BigNum modPow(BigNum a, BigNum b, const BigNum& m) { // a^b mod m
        a = a % m;
        BigNum result(1);
        BigNum zero;
        while (!(b == zero)) {
            if (b.getBit(0)) result = (result * a) % m;
            a = (a * a) % m;
            b = b >> 1;
        }
        return result;
    }

    static BigNum modInverse(BigNum a, BigNum m) { // a^(-1) mod m
        if (a == BigNum(0)) return BigNum(0);
        BigNum orig_m = m;
        a = a % m;
        BigNum old_r = a, r = m;
        BigNum old_s(1), s(0);

        while (!(r == BigNum())) {
            BigNum q = old_r / r;
            BigNum temp = old_r - q * r;
            old_r = r; r = temp;

            BigNum qs_mod = (q * s) % orig_m;
            BigNum new_s = (old_s >= qs_mod) ? old_s - qs_mod : orig_m - (qs_mod - old_s);
            old_s = s; s = new_s;
        }
        if (!(old_r == BigNum(1))) return BigNum(0);
        return old_s;
    }

    static BigNum modInversePowerOf2(const BigNum& a, int k) { // a^(-1) mod 2^k
        BigNum x(1);
        int current_bits = 1;
        while (current_bits < k) {
            int next_bits = min(current_bits * 2, k);
            BigNum ax_low = (a * x).truncate(next_bits);
            BigNum threshold = BigNum(1) << next_bits;
            BigNum two_minus_ax = (threshold - ax_low).inc().inc();
            x = (x * two_minus_ax).truncate(next_bits);
            current_bits = next_bits;
        }
        return x;
    }

    static BigNum gcd(BigNum a, BigNum b) {
        BigNum zero;
        while (!(b == zero)) { BigNum t = a % b; a = b; b = t; }
        return a;
    }

    static BigNum lcm(const BigNum& a, const BigNum& b) {
        return (a * b) / gcd(a, b);
    }

    BigNum operator>>(int k) const {
        if (k <= 0) return *this;
        if (k == 1) return rsh1();
        BigNum result;
        int block_shift = k / 32, bit_shift = k % 32;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            int src = i + block_shift;
            if (src < ARRAY_SIZE) {
                result.num[i] = num[src] >> bit_shift;
                if (bit_shift > 0 && src + 1 < ARRAY_SIZE)
                    result.num[i] |= num[src + 1] << (32 - bit_shift);
            }
        }
        return result;
    }

    BigNum rsh1() const {
        BigNum result;
        for (int i = 0; i < ARRAY_SIZE - 1; i++)
            result.num[i] = (num[i] >> 1) | ((num[i + 1] & 1) << 31);
        result.num[ARRAY_SIZE - 1] = num[ARRAY_SIZE - 1] >> 1;
        return result;
    }

    BigNum operator<<(int k) const {
        BigNum result;
        int block_shift = k / 32, bit_shift = k % 32;
        for (int i = ARRAY_SIZE - 1; i >= 0; i--) {
            int dst = i + block_shift;
            if (dst < ARRAY_SIZE) {
                result.num[dst] = num[i] << bit_shift;
                if (bit_shift > 0 && i > 0)
                    result.num[dst] |= num[i - 1] >> (32 - bit_shift);
            }
        }
        return result;
    }

    bool getBit(int i) const {
        int block_idx = i / 32, bit_idx = i % 32;
        if (block_idx >= ARRAY_SIZE) return false;
        return (num[block_idx] & (1U << bit_idx)) != 0;
    }

    void setBit(int i) {
        int block_idx = i / 32, bit_idx = i % 32;
        if (block_idx < ARRAY_SIZE) num[block_idx] |= (1U << bit_idx);
    }

    int findMSB() const {
        for (int i = ARRAY_SIZE - 1; i >= 0; i--) {
            if (num[i] != 0) {
                uint32_t val = num[i];
                for (int j = 31; j >= 0; j--)
                    if (val & (1U << j)) return i * 32 + j;
            }
        }
        return -1;
    }

    static void leftShift1(BigNum& a) {
        uint32_t carry = 0;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            uint32_t new_carry = (a.num[i] >> 31) & 1;
            a.num[i] = (a.num[i] << 1) | carry;
            carry = new_carry;
        }
    }

    BigNum inc() const {
        BigNum result = *this;
        uint64_t carry = 1;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            uint64_t sum = result.num[i] + carry;
            result.num[i] = (uint32_t)(sum & BLOCK_MASK);
            carry = sum >> 32;
            if (carry == 0) break;
        }
        return result;
    }

    BigNum dec() const {
        BigNum result = *this;
        int64_t borrow = 1;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            int64_t diff = (int64_t)result.num[i] - borrow;
            if (diff < 0) { diff += (int64_t)(1ULL << 32); borrow = 1; }
            else borrow = 0;
            result.num[i] = (uint32_t)diff;
            if (borrow == 0) break;
        }
        return result;
    }

    BigNum truncate(int k) const {
        BigNum result = *this;
        int block_idx = k / 32, bit_idx = k % 32;
        for (int i = ARRAY_SIZE - 1; i > block_idx; i--) result.num[i] = 0;
        if (block_idx < ARRAY_SIZE) {
            if (bit_idx == 0) result.num[block_idx] = 0;
            else result.num[block_idx] &= ((1U << bit_idx) - 1);
        }
        return result;
    }

    bool isEven() const { return (num[0] & 1) == 0; }
    bool isOdd() const { return (num[0] & 1) == 1; }

    bool mod3equals0() const {
        uint64_t sum = 0;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            uint32_t val = num[i];
            for (int j = 0; j < 8; j++) { sum += val & 0xF; val >>= 4; }
        }
        return sum % 3 == 0;
    }

    static BigNum randBits(int bits) {
        BigNum result;
        result.bit_len = bits;
        int num_blocks = (bits + 31) / 32;
        for (int i = 0; i < num_blocks && i < ARRAY_SIZE; i++) {
            uint64_t val = getRng()();
            result.num[i] = (uint32_t)(val & 0xFFFFFFFF);
            if (++i < num_blocks && i < ARRAY_SIZE)
                result.num[i] = (uint32_t)(val >> 32);
        }
        int msb_block = (bits - 1) / 32, msb_bit = (bits - 1) % 32;
        if (msb_block < ARRAY_SIZE)
            result.num[msb_block] |= (1U << msb_bit);
        return result;
    }

    string toHex() const {
        if (isZero()) return "0";
        string result;
        bool started = false;
        for (int i = ARRAY_SIZE - 1; i >= 0; i--) {
            if (!started && num[i] == 0) continue;
            started = true;
            char buf[16];
            if (result.empty()) sprintf(buf, "%x", num[i]);
            else sprintf(buf, "%08x", num[i]);
            result += buf;
        }
        return result;
    }

    string toBytes(int max_len) const {
        string result;
        for (int i = 0; i < max_len; i++) {
            int block_idx = i / 4, byte_pos = i % 4;
            if (block_idx < ARRAY_SIZE)
                result += (char)((num[block_idx] >> (byte_pos * 8)) & 0xFF);
        }
        return result;
    }

    friend ostream& operator<<(ostream& os, const BigNum& n) {
        os << n.toHex();
        return os;
    }
};
