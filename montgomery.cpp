#include "montgomery.h"
#include <iostream>
using namespace std;

// 初始化蒙哥马利上下文
MontgomeryContext montgomeryInit(const BigNum& m) {
    MontgomeryContext ctx;
    ctx.m = m;
    ctx.k = m.findMSB() + 1;

    // 计算 R = 2^k
    BigNum R = BigNum(1) << ctx.k;

    // 计算 m' = -m^(-1) mod R（使用 Newton 迭代法，非常高效）
    BigNum m_inv = BigNum::modInversePowerOf2(m, ctx.k);
    ctx.m_prime = R - m_inv;

    // 计算 R^2 mod m
    BigNum R2 = BigNum(1) << (ctx.k * 2);
    ctx.r2_mod_m = R2 % m;

    return ctx;
}

// 蒙哥马利约简
BigNum montgomeryReduce(const BigNum& T, const MontgomeryContext& ctx) {
    BigNum m_s = (T * ctx.m_prime).truncate(ctx.k);
    BigNum u = (T + m_s * ctx.m) >> ctx.k;
    if (u >= ctx.m) {
        u = u - ctx.m;
    }
    return u;
}

// 蒙哥马利乘法
BigNum montgomeryMul(BigNum a, BigNum b, const MontgomeryContext& ctx) {
    BigNum a_mont = montgomeryReduce(a * ctx.r2_mod_m, ctx);
    BigNum b_mont = montgomeryReduce(b * ctx.r2_mod_m, ctx);
    BigNum T = a_mont * b_mont;
    return montgomeryReduce(T, ctx);
}

// 快速模幂（蒙哥马利版本）
BigNum modPowFast(BigNum a, BigNum b, const BigNum& m) {
    if (m.isZero()) return BigNum();

    MontgomeryContext ctx = montgomeryInit(m);

    BigNum result_mont = montgomeryReduce(BigNum(1) * ctx.r2_mod_m, ctx);
    BigNum base_mont = montgomeryReduce((a % m) * ctx.r2_mod_m, ctx);

    BigNum zero;
    while (!(b == zero)) {
        if (b.getBit(0)) {
            BigNum T = result_mont * base_mont;
            result_mont = montgomeryReduce(T, ctx);
        }
        BigNum T = base_mont * base_mont;
        base_mont = montgomeryReduce(T, ctx);
        b = b >> 1;
    }

    return montgomeryReduce(result_mont, ctx);
}

// 快速模幂（使用预计算的蒙哥马利上下文）
BigNum modPowFastWithCtx(BigNum a, BigNum b, const MontgomeryContext& ctx) {
    BigNum result_mont = montgomeryReduce(BigNum(1) * ctx.r2_mod_m, ctx);
    BigNum base_mont = montgomeryReduce((a % ctx.m) * ctx.r2_mod_m, ctx);

    BigNum zero;
    while (!(b == zero)) {
        if (b.getBit(0)) {
            BigNum T = result_mont * base_mont;
            result_mont = montgomeryReduce(T, ctx);
        }
        BigNum T = base_mont * base_mont;
        base_mont = montgomeryReduce(T, ctx);
        b = b >> 1;
    }

    return montgomeryReduce(result_mont, ctx);
}
