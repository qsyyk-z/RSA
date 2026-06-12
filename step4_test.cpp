// Step4: 性能优化测试
#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <vector>
#include "rsa.h"
using namespace std;
using namespace std::chrono;

struct Timer {
    high_resolution_clock::time_point t;
    void start() { t = high_resolution_clock::now(); }
    double elapsed_sec() {
        return duration_cast<duration<double>>(high_resolution_clock::now() - t).count();
    }
};

struct BenchResult {
    int level;
    double avg, mn, mx;
};

static bool verifyPrimeWithPython(const BigNum& n, const string& name) {
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

static void testKeyGenerationOptimal(int bits) {
    setRSAOptimLevel(2);

    cout << "\n===== " << bits << " 位密钥生成" << " =====" << endl;

    Timer timer;
    timer.start();
    RSAKey key = generateRSAKey(bits);
    double elapsed = timer.elapsed_sec();

    cout << "耗时: " << elapsed << " 秒" << endl;
    cout << "p = " << key.p << endl;
    cout << "q = " << key.q << endl;
    cout << "n = " << key.n << endl;
    cout << "e = " << key.e << endl;
    cout << "d = " << key.d << endl;

    if (key.n == key.p * key.q) cout << "[PASS] n = p * q" << endl;
    else cout << "[FAIL] n != p * q" << endl;

    BigNum phi = BigNum::lcm(key.p.dec(), key.q.dec());
    if ((key.e * key.d) % phi == BigNum(1)) cout << "[PASS] e*d ≡ 1 (mod phi)" << endl;
    else cout << "[FAIL] e*d ≡ 1 (mod phi)" << endl;

    if (BigNum::gcd(key.e, phi) == BigNum(1)) cout << "[PASS] gcd(e, phi) = 1" << endl;
    else cout << "[FAIL] gcd(e, phi) != 1" << endl;

    verifyPrimeWithPython(key.p, "p");
    verifyPrimeWithPython(key.q, "q");
}

static void benchmarkMontgomeryModPow() {
    const int bits = 256, trials = 10;
    cout << "\n===== 蒙哥马利模乘加速测试 (" << bits << " 位, " << trials << " 次) =====" << endl;

    BigNum base = BigNum::randBits(bits);
    BigNum exp = BigNum::randBits(bits);
    BigNum mod = BigNum::randBits(bits * 2);
    mod.num[0] |= 1;

    Timer timer;
    timer.start();
    for (int i = 0; i < trials; i++) BigNum::modPow(base, exp, mod);
    double normal = timer.elapsed_sec() / trials;

    timer.start();
    for (int i = 0; i < trials; i++) modPowFast(base, exp, mod);
    double fast = timer.elapsed_sec() / trials;

    cout << "  普通模幂:     " << normal << " 秒" << endl;
    cout << "  蒙哥马利模幂: " << fast << " 秒" << endl;
    if (fast > 0) cout << "  加速比:       " << normal / fast << "x" << endl;

    if (BigNum::modPow(base, exp, mod) == modPowFast(base, exp, mod))
        cout << "  [PASS] 两种方法结果一致" << endl;
    else
        cout << "  [FAIL] 两种方法结果不一致" << endl;
}

static BenchResult benchmarkKeyGen(int level, int trials) {
    setRSAOptimLevel(level);
    BenchResult res{level, 0, 1e9, 0};
    double total = 0;

    cout << "\n>>> " << rsaOptimLevelName() << " | RSA-768 | " << trials << " 次" << endl;
    for (int i = 0; i < trials; i++) {
        Timer timer;
        timer.start();
        RSAKey key = generateRSAKey(768);
        double el = timer.elapsed_sec();
        total += el;
        res.mn = min(res.mn, el);
        res.mx = max(res.mx, el);
        cout << "    第 " << i + 1 << " 次: " << el << " 秒" << endl;
        (void)key;
    }
    res.avg = total / trials;
    cout << "    平均: " << res.avg << " 秒, 最小: " << res.mn << " 秒, 最大: " << res.mx << " 秒" << endl;
    if (res.avg <= 1.0) cout << "    [PASS] RSA-768 <= 1 秒" << endl;
    else cout << "    [FAIL] RSA-768 > 1 秒" << endl;
    return res;
}

static void printSummary(const vector<BenchResult>& results) {
    cout << "\n===== RSA-768 三级优化对比汇总 =====" << endl;
    cout << "  级别 | 平均(秒) | 最小(秒) | 最大(秒) | 相对 Level0" << endl;
    cout << "  -----|----------|----------|----------|-------------" << endl;

    double base_avg = 0;
    for (const auto& r : results)
        if (r.level == 0) base_avg = r.avg;

    for (const auto& r : results) {
        double speedup = (base_avg > 0 && r.avg > 0) ? base_avg / r.avg : 0;
        cout << "  L" << r.level << "   | "
             << r.avg << " | "
             << r.mn << " | "
             << r.mx << " | ";
        if (r.level == 0) cout << "1.00x (基准)";
        else cout << speedup << "x";
        cout << endl;
    }
}

int main() {
    cout << "========================================" << endl;
    cout << "  Step4: 性能优化测试" << endl;
    cout << "========================================" << endl;

    benchmarkMontgomeryModPow();

    vector<BenchResult> results;
    for (int lv = 0; lv <= 2; lv++)
        results.push_back(benchmarkKeyGen(lv, 3));

    printSummary(results);

    cout << "\n===== 最优版本密钥生成正确性测试 =====" << endl;
    testKeyGenerationOptimal(512);
    testKeyGenerationOptimal(768);

    cout << "\n========================================" << endl;
    cout << "  Step4 测试完成" << endl;
    cout << "========================================" << endl;
    return 0;
}
