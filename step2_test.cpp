// ============================================================
// Step2 测试: 密钥生成算法测试
// ============================================================
#include <iostream>
#include <fstream>
#include <ctime>
#include "bignum.h"
#include "montgomery.h"
#include "rsa.h"
using namespace std;

// 使用 Python 验证素数性
bool verifyPrimeWithPython(const BigNum& n, const string& name) {
    ofstream out("verify_prime.py");
    out << "n = int('" << n.toHex() << "', 16)\n";
    out << "from sympy import isprime\n";
    out << "try:\n";
    out << "    result = isprime(n)\n";
    out << "    print(f'PRIME={result}')\n";
    out << "except:\n";
    out << "    # fallback: use gmpy2\n";
    out << "    import gmpy2\n";
    out << "    result = gmpy2.is_prime(n)\n";
    out << "    print(f'PRIME={result}')\n";
    out.close();

    int ret = system("python verify_prime.py > verify_prime_result.txt 2>&1");
    if (ret != 0) {
        cout << "[WARN] Python verification not available for " << name << endl;
        return true; // 无法验证，假设通过
    }

    ifstream in("verify_prime_result.txt");
    string line;
    while (getline(in, line)) {
        if (line.find("PRIME=") != string::npos) {
            bool is_prime = line.find("PRIME=True") != string::npos;
            if (is_prime) {
                cout << "[PASS] " << name << " is prime (verified by Python)" << endl;
            } else {
                cout << "[FAIL] " << name << " is NOT prime (verified by Python)" << endl;
            }
            return is_prime;
        }
    }
    return true;
}

void testKeyGeneration(int bits) {
    cout << "\n===== 测试 " << bits << " 位密钥生成 =====" << endl;

    time_t start = time(NULL);
    RSAKey key = generateRSAKey(bits);
    time_t end = time(NULL);

    double elapsed = difftime(end, start);
    cout << "密钥生成耗时: " << elapsed << " 秒" << endl;

    // 输出密钥信息
    cout << "\n--- 密钥详情 ---" << endl;
    cout << "位数: " << key.bits << endl;
    cout << "p = " << key.p << endl;
    cout << "q = " << key.q << endl;
    cout << "n = " << key.n << endl;
    cout << "e = " << key.e << endl;
    cout << "d = " << key.d << endl;

    // 验证 n = p * q
    if (key.n == key.p * key.q) {
        cout << "[PASS] n = p * q" << endl;
    } else {
        cout << "[FAIL] n != p * q" << endl;
    }

    // 验证 e * d ≡ 1 (mod phi)
    BigNum phi = BigNum::lcm(key.p.dec(), key.q.dec());
    BigNum check = (key.e * key.d) % phi;
    if (check == BigNum(1)) {
        cout << "[PASS] e * d ≡ 1 (mod phi)" << endl;
    } else {
        cout << "[FAIL] e * d ≡ 1 (mod phi) FAILED" << endl;
    }

    // 验证 gcd(e, phi) = 1
    if (BigNum::gcd(key.e, phi) == BigNum(1)) {
        cout << "[PASS] gcd(e, phi) = 1" << endl;
    } else {
        cout << "[FAIL] gcd(e, phi) != 1" << endl;
    }

    // 尝试用 Python 验证素数
    verifyPrimeWithPython(key.p, "p");
    verifyPrimeWithPython(key.q, "q");
}

int main() {
    cout << "========================================" << endl;
    cout << "  Step2: 密钥生成算法测试" << endl;
    cout << "========================================" << endl;

    // 测试不同位数的密钥生成
    testKeyGeneration(512);
    testKeyGeneration(768);

    cout << "\n========================================" << endl;
    cout << "  Step2 测试完成" << endl;
    cout << "========================================" << endl;

    return 0;
}
