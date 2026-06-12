// Step1: 大数运算正确性测试
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "bignum.h"
using namespace std;

bool testAddSub() {
    cout << "===== 加法和减法 =====" << endl;
    bool pass = true;

    if ((BigNum(12345) + BigNum(67890)) == BigNum(80235))
        cout << "[PASS] 12345 + 67890 = 80235" << endl;
    else { cout << "[FAIL] 加法" << endl; pass = false; }

    if ((BigNum(67890) - BigNum(12345)) == BigNum(55545))
        cout << "[PASS] 67890 - 12345 = 55545" << endl;
    else { cout << "[FAIL] 减法" << endl; pass = false; }

    BigNum e("ffffffff"), g = e + BigNum(1);
    if (g == BigNum("100000000"))
        cout << "[PASS] ffffffff + 1 = 100000000" << endl;
    else { cout << "[FAIL] 大数加法" << endl; pass = false; }

    if ((g - BigNum(1)) == e)
        cout << "[PASS] 100000000 - 1 = ffffffff" << endl;
    else { cout << "[FAIL] 大数减法" << endl; pass = false; }

    return pass;
}

bool testMultiplication() {
    cout << "\n===== 乘法 =====" << endl;
    bool pass = true;

    if ((BigNum(123) * BigNum(456)) == BigNum(56088))
        cout << "[PASS] 123 * 456 = 56088" << endl;
    else { cout << "[FAIL] 小数乘法" << endl; pass = false; }

    if ((BigNum("ffffffff") * BigNum("ffffffff")) == BigNum("fffffffe00000001"))
        cout << "[PASS] ffffffff * ffffffff = fffffffe00000001" << endl;
    else { cout << "[FAIL] 大数乘法" << endl; pass = false; }

    return pass;
}

bool testDivision() {
    cout << "\n===== 除法和取模 =====" << endl;
    bool pass = true;

    BigNum q = BigNum(100) / BigNum(7), r = BigNum(100) % BigNum(7);
    if (q == BigNum(14) && r == BigNum(2))
        cout << "[PASS] 100 / 7 = 14 ... 2" << endl;
    else { cout << "[FAIL] 除法" << endl; pass = false; }

    BigNum d("10000000000000000"), e("100000001");
    BigNum q2 = d / e, r2 = d % e;
    if (q2 * e + r2 == d)
        cout << "[PASS] 大数除法恒等式 a = q*b + r" << endl;
    else { cout << "[FAIL] 大数除法" << endl; pass = false; }

    return pass;
}

bool testModPow() {
    cout << "\n===== 模幂 =====" << endl;
    bool pass = true;

    if (BigNum::modPow(BigNum(2), BigNum(10), BigNum(1000)) == BigNum(24))
        cout << "[PASS] 2^10 mod 1000 = 24" << endl;
    else { cout << "[FAIL] 2^10 mod 1000" << endl; pass = false; }

    if (BigNum::modPow(BigNum(3), BigNum(17), BigNum(100)) == BigNum(63))
        cout << "[PASS] 3^17 mod 100 = 63" << endl;
    else { cout << "[FAIL] 3^17 mod 100" << endl; pass = false; }

    return pass;
}

bool testModInverse() {
    cout << "\n===== 模逆 =====" << endl;
    bool pass = true;

    BigNum inv = BigNum::modInverse(BigNum(7), BigNum(11));
    if (inv == BigNum(8))
        cout << "[PASS] 7^(-1) mod 11 = 8" << endl;
    else { cout << "[FAIL] 模逆" << endl; pass = false; }

    if ((BigNum(7) * inv) % BigNum(11) == BigNum(1))
        cout << "[PASS] 7 * 8 mod 11 = 1" << endl;
    else { cout << "[FAIL] 模逆验证" << endl; pass = false; }

    BigNum a = BigNum::randBits(256); a.num[0] |= 1;
    BigNum m = BigNum::randBits(512); m.num[0] |= 1;
    BigNum inv2 = BigNum::modInverse(a, m);
    if ((a * inv2) % m == BigNum(1))
        cout << "[PASS] 256 位模逆验证" << endl;
    else { cout << "[FAIL] 256 位模逆" << endl; pass = false; }

    return pass;
}

bool testGCD() {
    cout << "\n===== GCD =====" << endl;
    bool pass = true;
    if (BigNum::gcd(BigNum(48), BigNum(18)) == BigNum(6))
        cout << "[PASS] gcd(48, 18) = 6" << endl;
    else { cout << "[FAIL] gcd(48,18)" << endl; pass = false; }
    if (BigNum::gcd(BigNum(17), BigNum(13)) == BigNum(1))
        cout << "[PASS] gcd(17, 13) = 1" << endl;
    else { cout << "[FAIL] gcd(17,13)" << endl; pass = false; }
    return pass;
}

static string extractHex(const string& line) {
    size_t pos = line.find('=');
    if (pos == string::npos) return "";
    string val = line.substr(pos + 1);
    if (val.size() >= 2 && val[0] == '0' && (val[1] == 'x' || val[1] == 'X'))
        return val.substr(2);
    if (val.size() >= 3 && val[0] == ' ' && val[1] == '0' && (val[2] == 'x' || val[2] == 'X'))
        return val.substr(3);
    return val;
}

bool testWithPython() {
    cout << "\n===== Python 对拍验证 =====" << endl;
    bool pass = true;

    ofstream out("test_data.py");
    out << "import random, math\nrandom.seed(42)\n\n";
    out << "for t in range(5):\n";
    out << "    m = random.getrandbits(512) | 1\n";
    out << "    a = random.getrandbits(384)\n";
    out << "    while math.gcd(a, m) != 1:\n";
    out << "        a = random.getrandbits(384)\n";
    out << "    b = random.getrandbits(384)\n";
    out << "    print(f'CASE_{t}')\n";
    out << "    print(f'A={hex(a)}')\n";
    out << "    print(f'B={hex(b)}')\n";
    out << "    print(f'M={hex(m)}')\n";
    out << "    print(f'MUL={hex(a * b)}')\n";
    out << "    print(f'MOD={hex(a % m)}')\n";
    out << "    print(f'POWM={hex(pow(a, b, m))}')\n";
    out << "    print(f'INV={hex(pow(a, -1, m))}')\n";
    out.close();

    ifstream in("test_results.txt");

    string line;
    int case_idx = 0;
    while (getline(in, line) && case_idx < 5) {
        if (line.find("CASE_") == string::npos) continue;

        string a_hex, b_hex, m_hex, py_mul, py_mod, py_powm, py_inv;
        getline(in, line); a_hex = extractHex(line);
        getline(in, line); b_hex = extractHex(line);
        getline(in, line); m_hex = extractHex(line);
        getline(in, line); py_mul = extractHex(line);
        getline(in, line); py_mod = extractHex(line);
        getline(in, line); py_powm = extractHex(line);
        getline(in, line); py_inv = extractHex(line);

        BigNum a(a_hex), b(b_hex), m(m_hex);
        cout << "--- 用例 " << case_idx << " ---" << endl;
        if ((a * b).toHex() == py_mul) cout << "[PASS] 乘法" << endl;
        else { cout << "[FAIL] 乘法" << endl; pass = false; }
        if ((a % m).toHex() == py_mod) cout << "[PASS] 取模" << endl;
        else { cout << "[FAIL] 取模" << endl; pass = false; }
        if (BigNum::modPow(a, b, m).toHex() == py_powm) cout << "[PASS] 模幂" << endl;
        else { cout << "[FAIL] 模幂" << endl; pass = false; }
        if (BigNum::modInverse(a, m).toHex() == py_inv) cout << "[PASS] 模逆" << endl;
        else { cout << "[FAIL] 模逆" << endl; pass = false; }
        case_idx++;
    }
    return pass;
}

int main() {
    cout << "========================================" << endl;
    cout << "  Step1: 大数运算正确性测试" << endl;
    cout << "========================================" << endl;

    bool all_pass = true;
    all_pass &= testAddSub();
    all_pass &= testMultiplication();
    all_pass &= testDivision();
    all_pass &= testModPow();
    all_pass &= testModInverse();
    all_pass &= testGCD();
    all_pass &= testWithPython();

    cout << "\n========================================" << endl;
    cout << (all_pass ? "  所有测试通过!" : "  部分测试失败!") << endl;
    cout << "========================================" << endl;
    return all_pass ? 0 : 1;
}
