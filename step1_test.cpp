// ============================================================
// Step1 测试: 大数运算正确性测试
// 使用 Python 生成已知答案进行对拍验证
// ============================================================
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include "bignum.h"
using namespace std;

// 从 Python 脚本读取测试数据
struct TestCase {
    string name;
    BigNum a, b, m;
    BigNum expected_mul;
    BigNum expected_mod;
    BigNum expected_powm;
    BigNum expected_inv_a;  // a^(-1) mod m
    BigNum expected_inv_b;  // b^(-1) mod m
};

bool testAddSub() {
    cout << "===== 测试加法和减法 =====" << endl;
    bool pass = true;

    // 测试 1: 简单加法
    BigNum a(12345);
    BigNum b(67890);
    BigNum c = a + b;
    if (c == BigNum(80235)) {
        cout << "[PASS] 12345 + 67890 = 80235" << endl;
    } else {
        cout << "[FAIL] 12345 + 67890 = " << c << " (expected 80235)" << endl;
        pass = false;
    }

    // 测试 2: 简单减法
    BigNum d = b - a;
    if (d == BigNum(55545)) {
        cout << "[PASS] 67890 - 12345 = 55545" << endl;
    } else {
        cout << "[FAIL] 67890 - 12345 = " << d << " (expected 55545)" << endl;
        pass = false;
    }

    // 测试 3: 大数加法
    BigNum e("ffffffff");
    BigNum f(1);
    BigNum g = e + f;
    if (g == BigNum("100000000")) {
        cout << "[PASS] ffffffff + 1 = 100000000" << endl;
    } else {
        cout << "[FAIL] ffffffff + 1 = " << g << " (expected 100000000)" << endl;
        pass = false;
    }

    // 测试 4: 大数减法
    BigNum h = g - f;
    if (h == e) {
        cout << "[PASS] 100000000 - 1 = ffffffff" << endl;
    } else {
        cout << "[FAIL] 100000000 - 1 = " << h << " (expected ffffffff)" << endl;
        pass = false;
    }

    return pass;
}

bool testMultiplication() {
    cout << "\n===== 测试乘法 =====" << endl;
    bool pass = true;

    // 测试 1: 简单乘法
    BigNum a(123);
    BigNum b(456);
    BigNum c = a * b;
    if (c == BigNum(56088)) {
        cout << "[PASS] 123 * 456 = 56088" << endl;
    } else {
        cout << "[FAIL] 123 * 456 = " << c << " (expected 56088)" << endl;
        pass = false;
    }

    // 测试 2: 大数乘法
    BigNum d("ffffffff");
    BigNum e("ffffffff");
    BigNum f = d * e;
    if (f == BigNum("fffffffe00000001")) {
        cout << "[PASS] ffffffff * ffffffff = fffffffe00000001" << endl;
    } else {
        cout << "[FAIL] ffffffff * ffffffff = " << f << " (expected fffffffe00000001)" << endl;
        pass = false;
    }

    // 测试 3: 256位乘法
    BigNum x = BigNum::randBits(256);
    BigNum y = BigNum::randBits(256);
    BigNum z = x * y;
    // 用 Python 验证
    cout << "[INFO] 256-bit multiplication test:" << endl;
    cout << "  x = " << x << endl;
    cout << "  y = " << y << endl;
    cout << "  x*y = " << z << endl;
    cout << "  (verify with Python: int('" << x.toHex() << "', 16) * int('" << y.toHex() << "', 16))" << endl;

    return pass;
}

bool testDivision() {
    cout << "\n===== 测试除法和取模 =====" << endl;
    bool pass = true;

    // 测试 1: 简单除法
    BigNum a(100);
    BigNum b(7);
    BigNum q = a / b;
    BigNum r = a % b;
    if (q == BigNum(14) && r == BigNum(2)) {
        cout << "[PASS] 100 / 7 = 14 remainder 2" << endl;
    } else {
        cout << "[FAIL] 100 / 7 = " << q << " remainder " << r << " (expected 14 remainder 2)" << endl;
        pass = false;
    }

    // 测试 2: 验证 a = q * b + r
    BigNum c = q * b + r;
    if (c == a) {
        cout << "[PASS] q * b + r == a" << endl;
    } else {
        cout << "[FAIL] q * b + r != a" << endl;
        pass = false;
    }

    // 测试 3: 大数除法
    BigNum d("10000000000000000");
    BigNum e("100000001");
    BigNum q2 = d / e;
    BigNum r2 = d % e;
    cout << "[INFO] 10000000000000000 / 100000001 = " << q2 << " remainder " << r2 << endl;
    BigNum check = q2 * e + r2;
    if (check == d) {
        cout << "[PASS] q * b + r == a (large numbers)" << endl;
    } else {
        cout << "[FAIL] q * b + r != a (large numbers)" << endl;
        pass = false;
    }

    return pass;
}

bool testModPow() {
    cout << "\n===== 测试模幂运算 =====" << endl;
    bool pass = true;

    // 测试 1: 小数模幂
    BigNum result = BigNum::modPow(BigNum(2), BigNum(10), BigNum(1000));
    if (result == BigNum(24)) {
        cout << "[PASS] 2^10 mod 1000 = 24" << endl;
    } else {
        cout << "[FAIL] 2^10 mod 1000 = " << result << " (expected 24)" << endl;
        pass = false;
    }

    // 测试 2: 3^17 mod 100 = 63 (0x3f)
    BigNum result2 = BigNum::modPow(BigNum(3), BigNum(17), BigNum(100));
    if (result2 == BigNum(63)) {
        cout << "[PASS] 3^17 mod 100 = 63" << endl;
    } else {
        cout << "[FAIL] 3^17 mod 100 = " << result2 << " (expected 63)" << endl;
        pass = false;
    }

    // 测试 3: 大数模幂
    BigNum base = BigNum::randBits(128);
    BigNum exp = BigNum::randBits(128);
    BigNum mod = BigNum::randBits(256);
    mod.num[0] |= 1; // 确保奇数
    BigNum result3 = BigNum::modPow(base, exp, mod);
    cout << "[INFO] 128-bit modPow test:" << endl;
    cout << "  base = " << base << endl;
    cout << "  exp  = " << exp << endl;
    cout << "  mod  = " << mod << endl;
    cout << "  result = " << result3 << endl;

    return pass;
}

bool testModInverse() {
    cout << "\n===== 测试模逆运算 =====" << endl;
    bool pass = true;

    // 测试 1: 7^(-1) mod 11 = 8
    BigNum inv1 = BigNum::modInverse(BigNum(7), BigNum(11));
    if (inv1 == BigNum(8)) {
        cout << "[PASS] 7^(-1) mod 11 = 8" << endl;
    } else {
        cout << "[FAIL] 7^(-1) mod 11 = " << inv1 << " (expected 8)" << endl;
        pass = false;
    }

    // 测试 2: 验证 a * a^(-1) ≡ 1 (mod m)
    BigNum check1 = (BigNum(7) * inv1) % BigNum(11);
    if (check1 == BigNum(1)) {
        cout << "[PASS] 7 * 8 mod 11 = 1" << endl;
    } else {
        cout << "[FAIL] 7 * 8 mod 11 = " << check1 << " (expected 1)" << endl;
        pass = false;
    }

    // 测试 3: 大数模逆
    BigNum a = BigNum::randBits(256);
    a.num[0] |= 1; // 确保奇数
    BigNum m = BigNum::randBits(512);
    m.num[0] |= 1; // 确保奇数
    BigNum inv2 = BigNum::modInverse(a, m);
    BigNum check2 = (a * inv2) % m;
    if (check2 == BigNum(1)) {
        cout << "[PASS] 256-bit modular inverse verified" << endl;
    } else {
        cout << "[FAIL] 256-bit modular inverse FAILED" << endl;
        cout << "  a * a^(-1) mod m = " << check2 << " (expected 1)" << endl;
        pass = false;
    }

    return pass;
}

bool testGCD() {
    cout << "\n===== 测试 GCD =====" << endl;
    bool pass = true;

    BigNum g1 = BigNum::gcd(BigNum(48), BigNum(18));
    if (g1 == BigNum(6)) {
        cout << "[PASS] gcd(48, 18) = 6" << endl;
    } else {
        cout << "[FAIL] gcd(48, 18) = " << g1 << " (expected 6)" << endl;
        pass = false;
    }

    BigNum g2 = BigNum::gcd(BigNum(17), BigNum(13));
    if (g2 == BigNum(1)) {
        cout << "[PASS] gcd(17, 13) = 1" << endl;
    } else {
        cout << "[FAIL] gcd(17, 13) = " << g2 << " (expected 1)" << endl;
        pass = false;
    }

    return pass;
}

// 使用 Python 对拍验证大数运算
bool testWithPython() {
    cout << "\n===== 使用 Python 对拍验证 =====" << endl;
    bool pass = true;

    // 生成测试数据文件
    ofstream out("test_data.py");
    out << "import random\n";
    out << "random.seed(42)\n\n";

    // 生成 5 组随机大数
    for (int t = 0; t < 5; t++) {
        out << "# Test case " << t + 1 << "\n";
        out << "a_" << t << " = random.getrandbits(384)\n";
        out << "b_" << t << " = random.getrandbits(384)\n";
        out << "m_" << t << " = random.getrandbits(512) | 1\n\n";
    }

    out << "# Print results\n";
    for (int t = 0; t < 5; t++) {
        out << "print(f'CASE_" << t << "')" << endl;
        out << "print(f'MUL={{hex(a_" << t << " * b_" << t << ")}}')" << endl;
        out << "print(f'MOD={{hex(a_" << t << " % m_" << t << ")}}')" << endl;
        out << "print(f'POWM={{hex(pow(a_" << t << ", b_" << t << ", m_" << t << "))}}')" << endl;
        out << "a_inv_" << t << " = pow(a_" << t << ", -1, m_" << t << ")\n";
        out << "print(f'INV={{hex(a_inv_" << t << ")}}')" << endl;
        out << "print(f'VERIFY={{hex((a_" << t << " * a_inv_" << t << ") % m_" << t << ")}}')" << endl;
    }
    out.close();

    // 运行 Python 脚本获取结果
    cout << "[INFO] Running Python verification script..." << endl;
    int ret = system("python test_data.py > test_results.txt 2>&1");
    if (ret != 0) {
        cout << "[WARN] Python not available, skipping cross-validation" << endl;
        return true;
    }

    // 读取 Python 结果
    ifstream in("test_results.txt");
    if (!in.is_open()) {
        cout << "[WARN] Could not read Python results" << endl;
        return true;
    }

    // 设置相同的随机种子生成相同的数
    srand(42);
    string line;
    int case_idx = 0;
    while (getline(in, line) && case_idx < 5) {
        if (line.find("CASE_") != string::npos) {
            cout << "\n--- Test case " << case_idx << " ---" << endl;
            // 生成相同的随机数
            BigNum a = BigNum::randBits(384);
            BigNum b = BigNum::randBits(384);
            BigNum m = BigNum::randBits(512);
            m.num[0] |= 1;

            // 读取 Python 结果
            string py_mul, py_mod, py_powm, py_inv, py_verify;
            getline(in, line); // MUL
            py_mul = line.substr(line.find("= {") + 3, line.find("}") - line.find("= {") - 3);
            getline(in, line); // MOD
            py_mod = line.substr(line.find("= {") + 3, line.find("}") - line.find("= {") - 3);
            getline(in, line); // POWM
            py_powm = line.substr(line.find("= {") + 3, line.find("}") - line.find("= {") - 3);
            getline(in, line); // INV
            py_inv = line.substr(line.find("= {") + 3, line.find("}") - line.find("= {") - 3);
            getline(in, line); // VERIFY

            // 比较乘法
            BigNum my_mul = a * b;
            if (my_mul.toHex() == py_mul) {
                cout << "[PASS] Multiplication matches Python" << endl;
            } else {
                cout << "[FAIL] Multiplication mismatch!" << endl;
                cout << "  Ours:   " << my_mul << endl;
                cout << "  Python: " << py_mul << endl;
                pass = false;
            }

            // 比较取模
            BigNum my_mod = a % m;
            if (my_mod.toHex() == py_mod) {
                cout << "[PASS] Modulo matches Python" << endl;
            } else {
                cout << "[FAIL] Modulo mismatch!" << endl;
                cout << "  Ours:   " << my_mod << endl;
                cout << "  Python: " << py_mod << endl;
                pass = false;
            }

            // 比较模幂
            BigNum my_powm = BigNum::modPow(a, b, m);
            if (my_powm.toHex() == py_powm) {
                cout << "[PASS] ModPow matches Python" << endl;
            } else {
                cout << "[FAIL] ModPow mismatch!" << endl;
                cout << "  Ours:   " << my_powm << endl;
                cout << "  Python: " << py_powm << endl;
                pass = false;
            }

            // 比较模逆
            BigNum my_inv = BigNum::modInverse(a, m);
            if (my_inv.toHex() == py_inv) {
                cout << "[PASS] ModInverse matches Python" << endl;
            } else {
                cout << "[FAIL] ModInverse mismatch!" << endl;
                cout << "  Ours:   " << my_inv << endl;
                cout << "  Python: " << py_inv << endl;
                pass = false;
            }

            case_idx++;
        }
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
    if (all_pass) {
        cout << "  所有测试通过!" << endl;
    } else {
        cout << "  部分测试失败!" << endl;
    }
    cout << "========================================" << endl;

    return all_pass ? 0 : 1;
}
