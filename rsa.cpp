#include "rsa.h"
using namespace std;

static const int PRIME_CANDIDATES[] = {
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
static const int NUM_PRIME_CANDIDATES = sizeof(PRIME_CANDIDATES) / sizeof(PRIME_CANDIDATES[0]);

static const int SMALL_PRIMES[] = {
    5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43,
    47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101,
    103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163,
    167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229,
    233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293,
    307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373,
    379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443,
    449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521,
    523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601,
    607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673,
    677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757,
    761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829, 839,
    853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929,
    937, 941, 947, 953, 967, 971, 977, 983, 991, 997,
};
static const int NUM_SMALL_PRIMES = sizeof(SMALL_PRIMES) / sizeof(SMALL_PRIMES[0]);

static int g_optim_level = 0;

void setRSAOptimLevel(int level) {
    if (level < 0) level = 0;
    if (level > 2) level = 2;
    g_optim_level = level;
}

int getRSAOptimLevel() { return g_optim_level; }

const char* rsaOptimLevelName() {
    switch (g_optim_level) {
        case 0: return "Level 0: 基础版 (Step2)";
        case 1: return "Level 1: 小素数试除预筛";
        case 2: return "Level 2: 小素数试除 + 蒙哥马利";
        default: return "未知";
    }
}

bool rsaUseOptimization() { return g_optim_level >= 2; }

static bool useMontgomery() { return g_optim_level >= 2; }
static bool useSmallPrimeScreen() { return g_optim_level >= 1; }

static bool passesSmallPrimeTrial(const BigNum& n) {
    for (int i = 0; i < NUM_SMALL_PRIMES; i++) {
        if (n % BigNum(SMALL_PRIMES[i]) == BigNum(0))
            return false;
    }
    return true;
}

static void add2(BigNum& x) {
    uint64_t carry = 2;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        uint64_t sum = (uint64_t)x.num[i] + carry;
        x.num[i] = (uint32_t)(sum & BLOCK_MASK);
        carry = sum >> 32;
        if (carry == 0) break;
    }
}

static BigNum modPowForRSA(BigNum a, BigNum b, const BigNum& m) {
    if (useMontgomery()) return modPowFast(a, b, m);
    return BigNum::modPow(a, b, m);
}

bool millerRabin(const BigNum& n, int iterations) {
    if (n.isEven() || n.mod3equals0()) return false;

    BigNum one(1), two(2);
    if (n == one || n == two) return false;

    BigNum d = n.dec();
    int r = 0;
    while (d.isEven()) { d = d >> 1; r++; }

    BigNum n_minus_1 = n.dec();
    MontgomeryContext ctx;
    if (useMontgomery()) ctx = montgomeryInit(n);

    for (int i = 0; i < iterations; i++) {
        BigNum a = BigNum::randBits(n.findMSB());
        a = a % n_minus_1;
        if (a < two) a = a + two;

        BigNum x = useMontgomery()
            ? modPowFastWithCtx(a, d, ctx)
            : BigNum::modPow(a, d, n);

        if (x == one || x == n_minus_1) continue;

        bool witness = false;
        for (int j = 0; j < r - 1; j++) {
            x = (x * x) % n;
            if (x == n_minus_1) { witness = true; break; }
            if (x == one) return false;
        }
        if (!witness) return false;
    }
    return true;
}

static bool isProbablyPrime(const BigNum& candidate) {
    if (useMontgomery())
        return millerRabin(candidate, 3) && millerRabin(candidate, 15);
    return millerRabin(candidate, 20);
}

BigNum generatePrime(int bits) {
    BigNum candidate = BigNum::randBits(bits);
    candidate.num[0] |= 1;
    int msb_block = (bits - 1) / 32, msb_bit = (bits - 1) % 32;
    candidate.num[msb_block] |= (1U << msb_bit);

    for (int attempt = 0; attempt < 100000; attempt++) {
        if (!candidate.isEven() && !candidate.mod3equals0()) {
            if (useSmallPrimeScreen() && !passesSmallPrimeTrial(candidate)) {
                add2(candidate);
                continue;
            }
            if (isProbablyPrime(candidate))
                return candidate;
        }
        add2(candidate);
    }
    return generatePrime(bits);
}

static BigNum chooseE(const BigNum& phi) {
    BigNum one(1);
    for (int i = 0; i < NUM_PRIME_CANDIDATES; i++) {
        BigNum e(PRIME_CANDIDATES[i]);
        if (BigNum::gcd(e, phi) == one) return e;
    }
    return BigNum(65537);
}

static BigNum computePhi(const BigNum& p, const BigNum& q) {
    if (useMontgomery()) return BigNum::lcm(p.dec(), q.dec());
    return (p.dec()) * (q.dec());
}

RSAKey generateRSAKey(int bits) {
    RSAKey key;
    key.bits = bits;
    int half = bits / 2;

    key.p = generatePrime(half);
    do { key.q = generatePrime(half); } while (key.q == key.p);

    key.n = key.p * key.q;
    BigNum phi = computePhi(key.p, key.q);
    key.e = chooseE(phi);
    key.d = BigNum::modInverse(key.e, phi);
    return key;
}

BigNum rsaEncryptBlock(const BigNum& m, const BigNum& e, const BigNum& n) {
    return modPowForRSA(m, e, n);
}

BigNum rsaDecryptBlock(const BigNum& c, const BigNum& d, const BigNum& n) {
    return modPowForRSA(c, d, n);
}

BigNum rsaSignBlock(const BigNum& m, const BigNum& d, const BigNum& n) {
    return modPowForRSA(m, d, n);
}

BigNum rsaVerifyBlock(const BigNum& s, const BigNum& e, const BigNum& n) {
    return modPowForRSA(s, e, n);
}
