#pragma once
#include "bignum.h"

struct MontgomeryContext {
    BigNum m;
    BigNum m_prime;
    BigNum r2_mod_m;
    int k;
};

MontgomeryContext montgomeryInit(const BigNum& m);
BigNum montgomeryReduce(const BigNum& T, const MontgomeryContext& ctx);
BigNum montgomeryMul(BigNum a, BigNum b, const MontgomeryContext& ctx);
BigNum modPowFast(BigNum a, BigNum b, const BigNum& m);
BigNum modPowFastWithCtx(BigNum a, BigNum b, const MontgomeryContext& ctx);
