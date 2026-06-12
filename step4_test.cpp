// ============================================================
// Step4 测试: 性能优化测试
// ============================================================
#include <iostream>
#include <fstream>
#include <ctime>
#include <chrono>
#include "bignum.h"
#include "montgomery.h"
#include "rsa.h"
using namespace std;
using namespace std::chrono;

// 性能计时辅助
class Timer {
    high_resolution_clock::time_point start_time;
public:
    void start() { start_time = high_resolution_clock::now(); }
    double elapsed_ms() {
        auto end = high_resolution_clock::now();
        return duration_cast<milliseconds>(end - start_time).count();
    }
    double elapsed_sec() {
        auto end = high_resolution_clock::now();
        return duration_cast<duration<double>>(end - start_time).count();
    }
};

void benchmarkKeyGeneration(int bits, int trials) {
    cout << "\n===== RSA-" << bits << " 密钥生成性能测试 =====" << endl;
    cout << "测试次数: " << trials << endl;

    double total_time = 0;
    double min_time = 1e9, max_time = 0;

    for (int i = 0; i < trials; i++) {
        Timer timer;
        timer.start();
        RSAKey key = generateRSAKey(bits);
        double elapsed = timer.elapsed_sec();
        total_time += elapsed;
        if (elapsed < min_time) min_time = elapsed;
        if (elapsed > max_time) max_time = elapsed;
        cout << "  Trial " << i + 1 << ": " << elapsed << " 秒" << endl;
    }

    double avg_time = total_time / trials;
    cout << "\n--- 结果 ---" << endl;
    cout << "  平均时间: " << avg_time << " 秒" << endl;
    cout << "  最小时间: " << min_time << " 秒" << endl;
    cout << "  最大时间: " << max_time << " 秒" << endl;

    if (bits == 768) {
        if (avg_time <= 1.0) {
            cout << "  [PASS] RSA-768 密钥生成时间 <= 1 秒!" << endl;
        } else {
            cout << "  [INFO] RSA-768 密钥生成时间 > 1 秒 (需要进一步优化)" << endl;
        }
    }
}

void benchmarkModPow(int bits, int trials) {
    cout << "\n===== 模幂运算性能测试 (" << bits << " 位) =====" << endl;

    BigNum base = BigNum::randBits(bits);
    BigNum exp = BigNum::randBits(bits);
    BigNum mod = BigNum::randBits(bits * 2);
    mod.num[0] |= 1;

    // 普通模幂
    Timer timer;
    timer.start();
    for (int i = 0; i < trials; i++) {
        BigNum result = BigNum::modPow(base, exp, mod);
    }
    double normal_time = timer.elapsed_sec() / trials;

    // 蒙哥马利模幂
    timer.start();
    for (int i = 0; i < trials; i++) {
        BigNum result = modPowFast(base, exp, mod);
    }
    double fast_time = timer.elapsed_sec() / trials;

    cout << "  普通模幂平均时间: " << normal_time << " 秒" << endl;
    cout << "  蒙哥马利模幂平均时间: " << fast_time << " 秒" << endl;
    if (normal_time > 0) {
        cout << "  加速比: " << normal_time / fast_time << "x" << endl;
    }

    // 验证结果一致
    BigNum r1 = BigNum::modPow(base, exp, mod);
    BigNum r2 = modPowFast(base, exp, mod);
    if (r1 == r2) {
        cout << "  [PASS] 两种方法结果一致" << endl;
    } else {
        cout << "  [FAIL] 两种方法结果不一致!" << endl;
    }
}

void benchmarkMillerRabin(int bits, int trials) {
    cout << "\n===== Miller-Rabin 素性测试性能 (" << bits << " 位) =====" << endl;

    double total_composite = 0;
    double total_prime = 0;
    int composite_count = 0;
    int prime_count = 0;

    for (int i = 0; i < trials; i++) {
        BigNum candidate = BigNum::randBits(bits);
        candidate.num[0] |= 1; // 确保奇数

        Timer timer;
        timer.start();
        bool is_prime = millerRabin(candidate, 20);
        double elapsed = timer.elapsed_sec();

        if (is_prime) {
            total_prime += elapsed;
            prime_count++;
        } else {
            total_composite += elapsed;
            composite_count++;
        }
    }

    cout << "  测试 " << trials << " 个 " << bits << " 位随机奇数" << endl;
    cout << "  合数: " << composite_count << " 个, 平均检测时间: "
         << (composite_count > 0 ? total_composite / composite_count : 0) << " 秒" << endl;
    cout << "  素数: " << prime_count << " 个, 平均检测时间: "
         << (prime_count > 0 ? total_prime / prime_count : 0) << " 秒" << endl;
}

int main() {
    cout << "========================================" << endl;
    cout << "  Step4: 性能优化测试" << endl;
    cout << "========================================" << endl;

    // 模幂性能对比
    benchmarkModPow(256, 10);
    benchmarkModPow(384, 5);

    // Miller-Rabin 性能
    benchmarkMillerRabin(384, 20);

    // 密钥生成性能
    benchmarkKeyGeneration(512, 3);
    benchmarkKeyGeneration(768, 3);

    cout << "\n========================================" << endl;
    cout << "  Step4 性能测试完成" << endl;
    cout << "========================================" << endl;

    return 0;
}
