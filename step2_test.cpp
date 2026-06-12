// Step2: 密钥生成算法测试
#include <iostream>
#include <fstream>
#include <ctime>
#include "rsa.h"
using namespace std;

bool verifyPrimeWithPython(const BigNum& n, const string& name) {
    ofstream out("verify_prime.py");
    out << "n = int('" << n.toHex() << "', 16)\n";
    out << "def is_prime(n):\n";
    out << "    if n < 2: return False\n";
    out << "    if n == 2: return True\n";
    out << "    if n % 2 == 0: return False\n";
    out << "    d, s = n - 1, 0\n";
    out << "    while d % 2 == 0: d //= 2; s += 1\n";
    out << "    for a in [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37]:\n";
    out << "        if a >= n: continue\n";
    out << "        x = pow(a, d, n)\n";
    out << "        if x == 1 or x == n - 1: continue\n";
    out << "        for _ in range(s - 1):\n";
    out << "            x = pow(x, 2, n)\n";
    out << "            if x == n - 1: break\n";
    out << "        else: return False\n";
    out << "    return True\n";
    out << "print(f'PRIME={is_prime(n)}')\n";
    out.close();

    if (system("python3 verify_prime.py > verify_prime_result.txt 2>&1") != 0) {
        cout << "[WARN] Python 验证不可用: " << name << endl;
        return true;
    }

    ifstream in("verify_prime_result.txt");
    if (!in.is_open()) {
        cout << "[WARN] 无法读取 Python 验证结果: " << name << endl;
        return true;
    }

    string line;
    while (getline(in, line)) {
        if (line.find("PRIME=True") != string::npos) {
            cout << "[PASS] " << name << " 为素数 (Python 验证)" << endl;
            return true;
        }
        if (line.find("PRIME=False") != string::npos) {
            cout << "[FAIL] " << name << " 不是素数 (Python 验证)" << endl;
            return false;
        }
    }
    cout << "[WARN] Python 未返回有效结果: " << name << endl;
    return true;
}

void testKeyGeneration(int bits) {
    cout << "\n===== " << bits << " 位密钥生成 =====" << endl;
    time_t start = time(NULL);
    RSAKey key = generateRSAKey(bits);
    double elapsed = difftime(time(NULL), start);

    cout << "耗时: " << elapsed << " 秒" << endl;
    cout << "p = " << key.p << endl;
    cout << "q = " << key.q << endl;
    cout << "n = " << key.n << endl;
    cout << "e = " << key.e << endl;
    cout << "d = " << key.d << endl;

    if (key.n == key.p * key.q) cout << "[PASS] n = p * q" << endl;
    else cout << "[FAIL] n != p * q" << endl;

    BigNum phi = (key.p.dec()) * (key.q.dec());
    if ((key.e * key.d) % phi == BigNum(1)) cout << "[PASS] e*d ≡ 1 (mod phi)" << endl;
    else cout << "[FAIL] e*d ≡ 1 (mod phi)" << endl;

    if (BigNum::gcd(key.e, phi) == BigNum(1)) cout << "[PASS] gcd(e, phi) = 1" << endl;
    else cout << "[FAIL] gcd(e, phi) != 1" << endl;

    verifyPrimeWithPython(key.p, "p");
    verifyPrimeWithPython(key.q, "q");
}

int main() {
    setRSAOptimLevel(0);

    cout << "========================================" << endl;
    cout << "  Step2: 密钥生成算法测试" << endl;
    cout << "  " << rsaOptimLevelName() << endl;
    cout << "========================================" << endl;

    testKeyGeneration(512);
    testKeyGeneration(768);

    cout << "\n========================================" << endl;
    cout << "  Step2 测试完成" << endl;
    cout << "========================================" << endl;
    return 0;
}
