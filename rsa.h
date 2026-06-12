#pragma once
#include "bignum.h"
#include "montgomery.h"

struct RSAKey {
    int bits;
    BigNum n, e, d, p, q;
};

// 优化级别:
//   0 = Step2 基础版（普通模幂，20 轮 MR，无小素数试除）
//   1 = 小素数试除预筛 + 基础版 MR/模幂
//   2 = 小素数试除 + 蒙哥马利模乘 + 两阶段 MR + lcm(phi)
void setRSAOptimLevel(int level);
int getRSAOptimLevel();
const char* rsaOptimLevelName();
bool rsaUseOptimization();

bool millerRabin(const BigNum& n, int iterations = 20);
BigNum generatePrime(int bits);
RSAKey generateRSAKey(int bits);

BigNum rsaEncryptBlock(const BigNum& m, const BigNum& e, const BigNum& n);
BigNum rsaDecryptBlock(const BigNum& c, const BigNum& d, const BigNum& n);
BigNum rsaSignBlock(const BigNum& m, const BigNum& d, const BigNum& n);
BigNum rsaVerifyBlock(const BigNum& s, const BigNum& e, const BigNum& n);
