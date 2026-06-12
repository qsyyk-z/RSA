#pragma once
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <random>
using namespace std;

// ============================================================
// BigNum: 自定义大整数类
// 以 32 位为一个块存储，最多支持 4096 位（128 个块）
// ============================================================

#define BLOCK_BITS 32
#define BLOCK_MASK 0xFFFFFFFFULL
#define MAX_BITNUM 4096
#define ARRAY_SIZE 128  // 4096 / 32

class BigNum {
public:
    uint32_t num[ARRAY_SIZE]; // 每个元素存储 32 位
    int bit_len;              // 有效位数（用于辅助，不严格维护）

    // ---- 构造函数 ----
    BigNum() {
        bit_len = 0;
        memset(num, 0, sizeof(num));
    }

    BigNum(int val) {
        bit_len = 0;
        memset(num, 0, sizeof(num));
        if (val >= 0) {
            num[0] = (uint32_t)val;
        } else {
            // 负数暂不处理
            num[0] = (uint32_t)(-val);
        }
        bit_len = 32;
    }

    BigNum(uint64_t val) {
        bit_len = 0;
        memset(num, 0, sizeof(num));
        num[0] = (uint32_t)(val & BLOCK_MASK);
        num[1] = (uint32_t)(val >> 32);
        bit_len = 64;
    }

    // 从十六进制字符串构造
    BigNum(const string& hex_str) {
        bit_len = 0;
        memset(num, 0, sizeof(num));
        // 去掉前导 0x
        string s = hex_str;
        if (s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            s = s.substr(2);
        }
        // 去掉前导零
        while (!s.empty() && s[0] == '0') s.erase(s.begin());
        if (s.empty()) return;

        int total_bits = (int)(s.size() * 4);
        int block_idx = 0;
        // 从低位到高位填充
        for (int i = (int)s.size(); i > 0; i -= 8) {
            int start = (i - 8 >= 0) ? i - 8 : 0;
            int len = i - start;
            string block_str = s.substr(start, len);
            uint32_t val = (uint32_t)stoul(block_str, nullptr, 16);
            if (block_idx < ARRAY_SIZE) {
                num[block_idx++] = val;
            }
        }
        bit_len = total_bits;
    }

    // 从字符串（原始字节）构造，用于 RSA 加密
    BigNum(const char* s, int chars_num) {
        bit_len = 0;
        memset(num, 0, sizeof(num));
        // 每 4 个字符（32位）一组
        for (int i = 0; i < chars_num; i++) {
            int block_idx = i / 4;
            int byte_pos = i % 4;
            if (block_idx < ARRAY_SIZE) {
                num[block_idx] |= ((uint32_t)(unsigned char)s[i]) << (byte_pos * 8);
            }
        }
    }

    // ---- 比较操作 ----
    // 返回 -1 (this < b), 0 (this == b), 1 (this > b)
    int compare(const BigNum& b) const {
        for (int i = ARRAY_SIZE - 1; i >= 0; i--) {
            if (num[i] > b.num[i]) return 1;
            if (num[i] < b.num[i]) return -1;
        }
        return 0;
    }

    bool isZero() const {
        for (int i = 0; i < ARRAY_SIZE; i++) {
            if (num[i] != 0) return false;
        }
        return true;
    }

    bool operator==(const BigNum& b) const { return compare(b) == 0; }
    bool operator!=(const BigNum& b) const { return compare(b) != 0; }
    bool operator<(const BigNum& b) const { return compare(b) < 0; }
    bool operator>(const BigNum& b) const { return compare(b) > 0; }
    bool operator<=(const BigNum& b) const { return compare(b) <= 0; }
    bool operator>=(const BigNum& b) const { return compare(b) >= 0; }

    // ---- 加法 ----
    BigNum operator+(const BigNum& b) const {
        BigNum result;
        uint64_t carry = 0;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            uint64_t sum = (uint64_t)num[i] + (uint64_t)b.num[i] + carry;
            result.num[i] = (uint32_t)(sum & BLOCK_MASK);
            carry = sum >> 32;
        }
        return result;
    }

    BigNum& operator+=(const BigNum& b) {
        uint64_t carry = 0;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            uint64_t sum = (uint64_t)num[i] + (uint64_t)b.num[i] + carry;
            num[i] = (uint32_t)(sum & BLOCK_MASK);
            carry = sum >> 32;
        }
        return *this;
    }

    // ---- 减法（假设 this >= b）----
    BigNum operator-(const BigNum& b) const {
        BigNum result;
        int64_t borrow = 0;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            int64_t diff = (int64_t)num[i] - (int64_t)b.num[i] - borrow;
            if (diff < 0) {
                diff += (int64_t)(1ULL << 32);
                borrow = 1;
            } else {
                borrow = 0;
            }
            result.num[i] = (uint32_t)diff;
        }
        return result;
    }

    // ---- 乘法 ----
    BigNum operator*(const BigNum& b) const {
        BigNum result;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            if (num[i] == 0) continue;
            uint64_t carry = 0;
            for (int j = 0; j < ARRAY_SIZE && i + j < ARRAY_SIZE; j++) {
                if (b.num[j] == 0 && carry == 0) continue;
                uint64_t prod = (uint64_t)num[i] * (uint64_t)b.num[j] + (uint64_t)result.num[i + j] + carry;
                result.num[i + j] = (uint32_t)(prod & BLOCK_MASK);
                carry = prod >> 32;
            }
        }
        return result;
    }

    // ---- 除法和取模 ----
    // 二进制长除法
    void divmod(const BigNum& b, BigNum& q, BigNum& r) const {
        memset(q.num, 0, sizeof(q.num));
        memset(r.num, 0, sizeof(r.num));
        q.bit_len = 0;
        r.bit_len = 0;

        if (compare(b) < 0) {
            r = *this;
            return;
        }
        if (b.isZero()) return;

        int msb = findMSB();
        for (int i = msb; i >= 0; i--) {
            // r 左移 1 位
            leftShift1(r);
            // 设置 r 的最低位为 a 的第 i 位
            if (getBit(i)) {
                r.num[0] |= 1;
            }
            if (r.compare(b) >= 0) {
                r = r - b;
                q.setBit(i);
            }
        }
    }

    BigNum operator/(const BigNum& b) const {
        BigNum q, r;
        divmod(b, q, r);
        return q;
    }

    BigNum operator%(const BigNum& b) const {
        BigNum q, r;
        divmod(b, q, r);
        return r;
    }

    // ---- 快速模幂 a^b mod m（平方-乘法算法）----
    static BigNum modPow(BigNum a, BigNum b, const BigNum& m) {
        a = a % m;
        BigNum result(1);
        BigNum zero;
        while (!(b == zero)) {
            if (b.getBit(0)) {
                result = (result * a) % m;
            }
            a = (a * a) % m;
            b = b >> 1;
        }
        return result;
    }

    // ---- 扩展欧几里得算法求模逆 ----
    // 求 x 使得 a*x ≡ 1 (mod m)
    // 使用改进的迭代算法，始终在模 m 下保持系数为正
    static BigNum modInverse(BigNum a, BigNum m) {
        if (a == BigNum(0)) return BigNum(0);

        BigNum orig_m = m;
        a = a % m;

        // 使用二元扩展欧几里得
        // 我们维护: t * old_r + s * old_s = gcd
        // 但始终让 s 和 t 在 [0, m) 范围内
        BigNum old_r = a, r = m;
        BigNum old_s(1), s(0);

        while (!(r == BigNum())) {
            BigNum q = old_r / r;

            // 更新 r
            BigNum temp = old_r - q * r;
            old_r = r;
            r = temp;

            // 更新 s: new_s = old_s - q * s (mod orig_m)
            // 为了避免负数，我们计算 (old_s + orig_m - (q*s) % orig_m) % orig_m
            BigNum qs_mod = (q * s) % orig_m;
            BigNum new_s;
            if (old_s >= qs_mod) {
                new_s = old_s - qs_mod;
            } else {
                new_s = orig_m - (qs_mod - old_s);
            }
            old_s = s;
            s = new_s;
        }

        // 如果 gcd != 1，则不存在模逆
        if (!(old_r == BigNum(1))) return BigNum(0);

        return old_s;
    }

    // ---- 模 2^k 的逆元（用于蒙哥马利乘法优化）----
    // 计算 a^(-1) mod 2^k，其中 a 是奇数
    // 使用 Newton 迭代法: x_{i+1} = x_i * (2 - a * x_i) mod 2^k
    static BigNum modInversePowerOf2(const BigNum& a, int k) {
        BigNum x(1); // x_0 = 1
        int current_bits = 1;

        while (current_bits < k) {
            int next_bits = current_bits * 2;
            if (next_bits > k) next_bits = k;

            // 计算 ax = a * x
            BigNum ax = a * x;

            // 计算 2 - ax mod 2^next_bits
            // 由于 a*x ≡ 1 (mod 2^current_bits)，ax 的低 current_bits 位是 ...0001
            // 所以 2 - ax 的低 current_bits 位是 ...0001（不会下溢到高位）
            // 但高位可能有借位，导致结果不正确
            // 正确方法：取 ax 的低 next_bits 位，然后计算 2^next_bits + 2 - ax_low
            BigNum ax_low = ax.truncate(next_bits);
            BigNum threshold = BigNum(1) << next_bits;
            BigNum neg_ax = threshold - ax_low;  // -ax mod 2^next_bits
            BigNum two_minus_ax = neg_ax.inc();  // -ax + 1 mod 2^next_bits
            two_minus_ax = two_minus_ax.inc();   // -ax + 2 mod 2^next_bits = (2 - ax) mod 2^next_bits

            x = (x * two_minus_ax).truncate(next_bits);
            current_bits = next_bits;
        }

        return x;
    }

    // ---- GCD ----
    static BigNum gcd(BigNum a, BigNum b) {
        BigNum zero;
        while (!(b == zero)) {
            BigNum t = a % b;
            a = b;
            b = t;
        }
        return a;
    }

    // ---- LCM ----
    static BigNum lcm(const BigNum& a, const BigNum& b) {
        return (a * b) / gcd(a, b);
    }

    // ---- 右移 1 位 ----
    BigNum operator>>(int k) const {
        if (k == 0) return *this;
        if (k == 1) return rsh1();
        return rshk(k);
    }

    BigNum rsh1() const {
        BigNum result;
        result.bit_len = bit_len;
        for (int i = 0; i < ARRAY_SIZE - 1; i++) {
            result.num[i] = (num[i] >> 1) | ((num[i + 1] & 1) << 31);
        }
        result.num[ARRAY_SIZE - 1] = num[ARRAY_SIZE - 1] >> 1;
        return result;
    }

    BigNum rshk(int k) const {
        BigNum result;
        result.bit_len = bit_len;
        int block_shift = k / 32;
        int bit_shift = k % 32;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            int src = i + block_shift;
            if (src < ARRAY_SIZE) {
                result.num[i] = num[src] >> bit_shift;
                if (bit_shift > 0 && src + 1 < ARRAY_SIZE) {
                    result.num[i] |= num[src + 1] << (32 - bit_shift);
                }
            }
        }
        return result;
    }

    // ---- 左移 k 位 ----
    BigNum operator<<(int k) const {
        BigNum result;
        result.bit_len = bit_len + k;
        int block_shift = k / 32;
        int bit_shift = k % 32;
        for (int i = ARRAY_SIZE - 1; i >= 0; i--) {
            int dst = i + block_shift;
            if (dst < ARRAY_SIZE) {
                result.num[dst] = num[i] << bit_shift;
                if (bit_shift > 0 && i > 0) {
                    result.num[dst] |= num[i - 1] >> (32 - bit_shift);
                }
            }
        }
        return result;
    }

    // ---- 位操作 ----
    bool getBit(int i) const {
        int block_idx = i / 32;
        int bit_idx = i % 32;
        if (block_idx >= ARRAY_SIZE) return false;
        return (num[block_idx] & (1U << bit_idx)) != 0;
    }

    void setBit(int i) {
        int block_idx = i / 32;
        int bit_idx = i % 32;
        if (block_idx < ARRAY_SIZE) {
            num[block_idx] |= (1U << bit_idx);
        }
    }

    // ---- 找最高有效位 ----
    int findMSB() const {
        for (int i = ARRAY_SIZE - 1; i >= 0; i--) {
            if (num[i] != 0) {
                uint32_t val = num[i];
                int bit_pos = i * 32;
                for (int j = 31; j >= 0; j--) {
                    if (val & (1U << j)) return bit_pos + j;
                }
            }
        }
        return -1;
    }

    // ---- 左移 1 位（静态）----
    static void leftShift1(BigNum& a) {
        uint32_t carry = 0;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            uint32_t new_carry = (a.num[i] >> 31) & 1;
            a.num[i] = (a.num[i] << 1) | carry;
            carry = new_carry;
        }
    }

    // ---- 自增/自减 ----
    BigNum inc() const {
        BigNum result = *this;
        uint64_t carry = 1;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            uint64_t sum = (uint64_t)result.num[i] + carry;
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
            if (diff < 0) {
                diff += (int64_t)(1ULL << 32);
                borrow = 1;
            } else {
                borrow = 0;
            }
            result.num[i] = (uint32_t)diff;
            if (borrow == 0) break;
        }
        return result;
    }

    // ---- 截断到 k 位 ----
    BigNum truncate(int k) const {
        BigNum result = *this;
        int block_idx = k / 32;
        int bit_idx = k % 32;
        for (int i = ARRAY_SIZE - 1; i > block_idx; i--) {
            result.num[i] = 0;
        }
        if (block_idx < ARRAY_SIZE) {
            if (bit_idx == 0) {
                result.num[block_idx] = 0;
            } else {
                result.num[block_idx] &= ((1U << bit_idx) - 1);
            }
        }
        return result;
    }

    // ---- 判断奇偶 ----
    bool isEven() const { return (num[0] & 1) == 0; }
    bool isOdd() const { return (num[0] & 1) == 1; }

    // ---- 快速模 3 检测 ----
    bool mod3equals0() const {
        uint64_t sum = 0;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            uint32_t val = num[i];
            for (int j = 0; j < 8; j++) {
                sum += val & 0xF;
                val >>= 4;
            }
        }
        return (sum % 3 == 0);
    }

    // ---- 随机大数生成 ----
    static BigNum randBits(int bits) {
        BigNum result;
        result.bit_len = bits;
        static mt19937_64 rng((unsigned long long)time(NULL) ^ (unsigned long long)clock());
        int num_blocks = (bits + 31) / 32;
        for (int i = 0; i < num_blocks && i < ARRAY_SIZE; i++) {
            uint64_t val = rng();
            result.num[i] = (uint32_t)(val & 0xFFFFFFFF);
            if (i + 1 < ARRAY_SIZE) {
                result.num[i + 1] = (uint32_t)(val >> 32);
                i++;
            }
        }
        // 确保最高位为 1
        int msb_block = (bits - 1) / 32;
        int msb_bit = (bits - 1) % 32;
        if (msb_block < ARRAY_SIZE) {
            result.num[msb_block] |= (1U << msb_bit);
        }
        return result;
    }

    // ---- 转十六进制字符串 ----
    string toHex() const {
        if (isZero()) return "0";
        string result;
        bool started = false;
        for (int i = ARRAY_SIZE - 1; i >= 0; i--) {
            if (!started && num[i] == 0) continue;
            started = true;
            char buf[16];
            if (i == ARRAY_SIZE - 1 || !started) {
                sprintf(buf, "%x", num[i]);
            } else {
                sprintf(buf, "%08x", num[i]);
            }
            result += buf;
        }
        return result;
    }

    // ---- 转十进制字符串 ----
    string toDec() const {
        if (isZero()) return "0";
        // 通过反复除以 10^9 来转换
        string result;
        BigNum a = *this;
        BigNum base((uint64_t)1000000000ULL);
        while (!a.isZero()) {
            BigNum q, r;
            a.divmod(base, q, r);
            uint32_t val = r.num[0];
            char buf[16];
            sprintf(buf, "%u", val);
            string part(buf);
            // 补零
            if (!result.empty()) {
                while (part.size() < 9) part = "0" + part;
            }
            result = part + result;
            a = q;
        }
        return result;
    }

    // ---- 从 BigNum 提取原始字节（用于解密）----
    string toBytes(int max_len) const {
        string result;
        for (int i = 0; i < max_len; i++) {
            int block_idx = i / 4;
            int byte_pos = i % 4;
            if (block_idx < ARRAY_SIZE) {
                char c = (char)((num[block_idx] >> (byte_pos * 8)) & 0xFF);
                result += c;
            }
        }
        return result;
    }

    // ---- 输出流 ----
    friend ostream& operator<<(ostream& os, const BigNum& n) {
        os << n.toHex();
        return os;
    }
};
