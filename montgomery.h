#pragma once
#include "bignum.h"

// 蒙哥马利乘法上下文
struct MontgomeryContext {
    BigNum m;        // 模数
    BigNum m_prime;  // -m^(-1) mod R
    BigNum r2_mod_m; // R^2 mod m
    int k;           // m 的位数
};

// 初始化蒙哥马利上下文
MontgomeryContext montgomeryInit(const BigNum& m);

// 蒙哥马利约简: 计算 TR^(-1) mod m
BigNum montgomeryReduce(const BigNum& T, const MontgomeryContext& ctx);

// 蒙哥马利乘法: 计算 a*b mod m
BigNum montgomeryMul(BigNum a, BigNum b, const MontgomeryContext& ctx);

// 快速模幂 a^b mod m（使用蒙哥马利乘法，内部初始化上下文）
BigNum modPowFast(BigNum a, BigNum b, const BigNum& m);

// 快速模幂 a^b mod m（使用预计算的蒙哥马利上下文）
BigNum modPowFastWithCtx(BigNum a, BigNum b, const MontgomeryContext& ctx);
