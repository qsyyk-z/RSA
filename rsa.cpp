#include "rsa.h"
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <random>
using namespace std;

// 预定义的素数列表（用于选择 e）
static int prime_candidates[] = {
    65537, 65539, 65543, 65551, 65557, 65563, 65579, 65581,
    65587, 65599, 65609, 65617, 65629, 65633, 65647, 65651,
    65657, 65677, 65687, 65699, 65701, 65707, 65713, 65717,
    65719, 65729, 65731, 65761, 65777, 65789, 65809, 65827,
    65831, 65837, 65839, 65843, 65851, 65867, 65881, 65899,
    65921, 65927, 65929, 65951, 65957, 65963, 65981, 65983,
    65993, 66029, 66037, 66041, 66047, 66067, 66071, 66083,
    66089, 66103, 66107, 66109, 66137, 66161, 66169, 66173,
    66179, 66191, 66221, 66239, 66271, 66293, 66301, 66337,
    66343, 66347, 66359, 66361, 66373, 66377, 66383, 66403,
    66413, 66431, 66449, 66457, 66463, 66467, 66491, 66499,
    66509, 66523, 66529, 66533, 66541, 66553, 66569, 66571,
    66587, 66593, 66601, 66617, 66629
};
static int num_prime_candidates = sizeof(prime_candidates) / sizeof(prime_candidates[0]);

// Miller-Rabin 素性测试（优化版：预计算蒙哥马利上下文，复用于所有迭代）
bool millerRabin(const BigNum& n, int iterations) {
    if (n.isEven()) return false;
    if (n.mod3equals0()) return false;

    BigNum one(1);
    BigNum two(2);
    if (n == one || n == two) return false;

    BigNum d = n.dec();
    int r = 0;
    while (d.isEven()) {
        d = d >> 1;
        r++;
    }

    BigNum n_minus_1 = n.dec();

    // 预计算蒙哥马利上下文（只算一次，所有迭代复用）
    MontgomeryContext ctx = montgomeryInit(n);

    for (int i = 0; i < iterations; i++) {
        BigNum a = BigNum::randBits(n.findMSB() + 1);
        a = a % n_minus_1;
        if (a < two) a = a + two;

        // 使用预计算上下文进行模幂
        BigNum x = modPowFastWithCtx(a, d, ctx);

        if (x == one || x == n_minus_1) continue;

        bool found = false;
        for (int j = 0; j < r - 1; j++) {
            x = (x * x) % n;
            if (x == n_minus_1) { found = true; break; }
            if (x == one) return false;
        }
        if (!found) return false;
    }
    return true;
}

// 生成指定位数的素数（增量搜索 + 两阶段测试）
BigNum generatePrime(int bits) {
    BigNum candidate = BigNum::randBits(bits);
    candidate.num[0] |= 1;
    int msb_block = (bits - 1) / 32;
    int msb_bit = (bits - 1) % 32;
    candidate.num[msb_block] |= (1U << msb_bit);

    int max_attempts = 100000;
    for (int attempt = 0; attempt < max_attempts; attempt++) {
        if (!candidate.isEven() && !candidate.mod3equals0()) {
            // 第一轮：少量迭代快速筛选
            if (millerRabin(candidate, 3)) {
                // 第二轮：更多迭代确认
                if (millerRabin(candidate, 15)) {
                    return candidate;
                }
            }
        }
        // candidate += 2
        uint64_t carry = 2;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            uint64_t sum = (uint64_t)candidate.num[i] + carry;
            candidate.num[i] = (uint32_t)(sum & 0xFFFFFFFF);
            carry = sum >> 32;
            if (carry == 0) break;
        }
    }
    return generatePrime(bits);
}

// 选择公钥指数 e
BigNum chooseE(const BigNum& phi) {
    BigNum one(1);
    for (int i = 0; i < num_prime_candidates; i++) {
        BigNum e(prime_candidates[i]);
        if (BigNum::gcd(e, phi) == one) return e;
    }
    cerr << "No suitable e found!" << endl;
    return BigNum(65537);
}

// 生成 RSA 密钥
RSAKey generateRSAKey(int bits) {
    RSAKey key;
    key.bits = bits;
    int half_bits = bits / 2;

    key.p = generatePrime(half_bits);
    cout << "p = " << key.p << endl;

    do { key.q = generatePrime(half_bits); } while (key.q == key.p);
    cout << "q = " << key.q << endl;

    key.n = key.p * key.q;
    cout << "n = " << key.n << endl;

    BigNum phi = BigNum::lcm(key.p.dec(), key.q.dec());
    key.e = chooseE(phi);
    cout << "e = " << key.e << endl;

    key.d = BigNum::modInverse(key.e, phi);
    cout << "d = " << key.d << endl;

    BigNum check = (key.e * key.d) % phi;
    if (check == BigNum(1)) {
        cout << "[OK] e * d ≡ 1 (mod phi) verified!" << endl;
    } else {
        cerr << "[ERROR] e * d ≡ 1 (mod phi) FAILED!" << endl;
    }
    return key;
}

BigNum rsaEncryptBlock(const BigNum& m, const BigNum& e, const BigNum& n) { return modPowFast(m, e, n); }
BigNum rsaDecryptBlock(const BigNum& c, const BigNum& d, const BigNum& n) { return modPowFast(c, d, n); }
BigNum rsaSignBlock(const BigNum& m, const BigNum& d, const BigNum& n) { return modPowFast(m, d, n); }
BigNum rsaVerifyBlock(const BigNum& s, const BigNum& e, const BigNum& n) { return modPowFast(s, e, n); }
