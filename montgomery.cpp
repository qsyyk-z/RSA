#include "montgomery.h"

MontgomeryContext montgomeryInit(const BigNum& m) {
    MontgomeryContext ctx;
    ctx.m = m;
    ctx.k = m.findMSB() + 1;

    BigNum R = BigNum(1) << ctx.k;
    BigNum m_inv = BigNum::modInversePowerOf2(m, ctx.k);
    ctx.m_prime = R - m_inv;

    BigNum R2 = BigNum(1) << (ctx.k * 2);
    ctx.r2_mod_m = R2 % m;
    return ctx;
}

BigNum montgomeryReduce(const BigNum& T, const MontgomeryContext& ctx) {
    BigNum m_s = (T * ctx.m_prime).truncate(ctx.k);
    BigNum u = (T + m_s * ctx.m) >> ctx.k;
    if (u >= ctx.m) u = u - ctx.m;
    return u;
}

BigNum montgomeryMul(BigNum a, BigNum b, const MontgomeryContext& ctx) {
    BigNum a_mont = montgomeryReduce(a * ctx.r2_mod_m, ctx);
    BigNum b_mont = montgomeryReduce(b * ctx.r2_mod_m, ctx);
    return montgomeryReduce(a_mont * b_mont, ctx);
}

static BigNum modPowMontgomery(BigNum a, BigNum b, const MontgomeryContext& ctx) {
    BigNum result_mont = montgomeryReduce(BigNum(1) * ctx.r2_mod_m, ctx);
    BigNum base_mont = montgomeryReduce((a % ctx.m) * ctx.r2_mod_m, ctx);
    BigNum zero;
    while (!(b == zero)) {
        if (b.getBit(0))
            result_mont = montgomeryReduce(result_mont * base_mont, ctx);
        base_mont = montgomeryReduce(base_mont * base_mont, ctx);
        b = b >> 1;
    }
    return montgomeryReduce(result_mont, ctx);
}

BigNum modPowFast(BigNum a, BigNum b, const BigNum& m) {
    if (m.isZero()) return BigNum();
    return modPowMontgomery(a, b, montgomeryInit(m));
}

BigNum modPowFastWithCtx(BigNum a, BigNum b, const MontgomeryContext& ctx) {
    return modPowMontgomery(a, b, ctx);
}
