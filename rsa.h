#pragma once
#include "bignum.h"
#include "montgomery.h"
#include <string>
#include <vector>

// RSA 密钥结构
struct RSAKey {
    int bits;
    BigNum n;   // 模数
    BigNum e;   // 公钥指数
    BigNum d;   // 私钥指数
    BigNum p;   // 素数 p
    BigNum q;   // 素数 q
};

// Miller-Rabin 素性测试
bool millerRabin(const BigNum& n, int iterations = 20);

// 生成指定位数的素数
BigNum generatePrime(int bits);

// 生成 RSA 密钥
RSAKey generateRSAKey(int bits);

// RSA 加密（单块）
BigNum rsaEncryptBlock(const BigNum& m, const BigNum& e, const BigNum& n);

// RSA 解密（单块）
BigNum rsaDecryptBlock(const BigNum& c, const BigNum& d, const BigNum& n);

// RSA 签名（单块）
BigNum rsaSignBlock(const BigNum& m, const BigNum& d, const BigNum& n);

// RSA 验签（单块）
BigNum rsaVerifyBlock(const BigNum& s, const BigNum& e, const BigNum& n);
