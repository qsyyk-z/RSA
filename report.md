# RSA 加密系统实验报告

## 任务概述

本实验从零开始实现了一个完整的 RSA 加密系统，包括以下四个步骤：

1. **大数运算库**：实现大整数的加减乘除、模运算、模幂、模逆、GCD 等基本运算
2. **密钥生成算法**：使用 Miller-Rabin 素性测试生成大素数，构造 RSA 密钥对
3. **RSA 加解密与数字签名**：实现消息的加密、解密、签名和验签
4. **性能优化**：使用蒙哥马利模乘法加速模幂运算，使 RSA-768 密钥生成时间 < 1 秒

---

## 任务 1：大数运算库

### 实现概述

源代码位于 `bignum.h`，实现了 `BigNum` 类，核心设计如下：

- **存储方式**：以 32 位为一个块（`uint32_t`），最多支持 4096 位（128 个块）
- **支持运算**：加法、减法、乘法、除法、取模、模幂（Square-and-Multiply）、模逆（扩展欧几里得算法）、GCD、LCM
- **位操作**：左移、右移、获取/设置指定位、查找最高有效位
- **辅助功能**：随机大数生成、十六进制/十进制字符串转换、快速模 3 检测

### 核心算法

**乘法**：采用经典的 O(n²) 学校乘法算法，每个 32 位块相乘并累加进位。

**除法/取模**：采用二进制长除法，从最高有效位开始逐位处理，避免复杂的试商操作。

**模幂（modPow）**：采用 Square-and-Multiply 算法，时间复杂度 O(log e) 次模乘。

**模逆（modInverse）**：采用改进的迭代扩展欧几里得算法，始终在模 m 下保持系数为正，避免负数处理问题。

**模 2^k 逆元（modInversePowerOf2）**：采用 Newton 迭代法，用于蒙哥马利乘法的快速初始化。迭代公式为 x_{i+1} = x_i × (2 - a × x_i) mod 2^k，每次迭代精度翻倍，仅需 O(log k) 次迭代。

### 正确性测试

测试代码位于 `step1_test.cpp`，编译运行方法：

```bash
g++ -O3 -o step1_test step1_test.cpp -std=c++17
./step1_test
```

测试内容包括：
- 加法和减法（小数和大数）
- 乘法（包括 256 位大数乘法）
- 除法和取模（验证 a = q × b + r 恒等式）
- 模幂运算（2^10 mod 1000 = 24，3^17 mod 100 = 63）
- 模逆运算（7^(-1) mod 11 = 8，验证 a × a^(-1) ≡ 1 (mod m)）
- GCD（gcd(48, 18) = 6，gcd(17, 13) = 1）
- Python 对拍验证（384 位随机数的乘法、取模、模幂、模逆）

### 测试结果

```
===== 测试加法和减法 =====
[PASS] 12345 + 67890 = 80235
[PASS] 67890 - 12345 = 55545
[PASS] ffffffff + 1 = 100000000
[PASS] 100000000 - 1 = ffffffff

===== 测试乘法 =====
[PASS] 123 * 456 = 56088
[PASS] ffffffff * ffffffff = fffffffe00000001

===== 测试除法和取模 =====
[PASS] 100 / 7 = 14 remainder 2
[PASS] q * b + r == a
[PASS] q * b + r == a (large numbers)

===== 测试模幂运算 =====
[PASS] 2^10 mod 1000 = 24
[PASS] 3^17 mod 100 = 63

===== 测试模逆运算 =====
[PASS] 7^(-1) mod 11 = 8
[PASS] 7 * 8 mod 11 = 1
[PASS] 256-bit modular inverse verified

===== 测试 GCD =====
[PASS] gcd(48, 18) = 6
[PASS] gcd(17, 13) = 1

所有测试通过!
```

---

## 任务 2：密钥生成算法

### 实现概述

源代码位于 `rsa.cpp`，实现了以下功能：

### Miller-Rabin 素性测试

```cpp
bool millerRabin(const BigNum& n, int iterations)
```

**算法流程**：
1. 快速排除偶数和 3 的倍数
2. 将 n-1 分解为 d × 2^r 的形式
3. 预计算蒙哥马利上下文（只算一次，所有迭代复用）
4. 进行 `iterations` 轮测试：
   - 随机选择底数 a ∈ [2, n-2]
   - 计算 x = a^d mod n（使用蒙哥马利快速模幂）
   - 若 x ≡ 1 或 x ≡ n-1，可能为素数，继续下一轮
   - 否则进行 r-1 次平方，检查是否出现 n-1
   - 若平方过程中出现 1，则为合数

**优化措施**：
- 使用 `modPowFastWithCtx` 复用蒙哥马利上下文，避免重复初始化
- 使用 `mt19937_64` 高质量随机数生成器

### 素数生成

```cpp
BigNum generatePrime(int bits)
```

**算法流程**：
1. 随机生成指定位数的奇数作为候选
2. **增量搜索**：每次 +2，避免重复生成随机数的开销
3. **两阶段测试**：
   - 第一轮：3 次 Miller-Rabin 迭代快速筛选合数
   - 第二轮：15 次 Miller-Rabin 迭代确认素数

### RSA 密钥生成

```cpp
RSAKey generateRSAKey(int bits)
```

**算法流程**：
1. 生成两个不同的 `bits/2` 位素数 p 和 q
2. 计算 n = p × q
3. 计算 φ(n) = lcm(p-1, q-1)（使用 lcm 而非 (p-1)(q-1)，使 d 更小）
4. 从预置素数列表中选择与 φ(n) 互质的最小 e（通常为 65537）
5. 计算 d = e^(-1) mod φ(n)（使用扩展欧几里得算法）
6. 验证 e × d ≡ 1 (mod φ(n))

### 正确性测试

测试代码位于 `step2_test.cpp`，编译运行方法：

```bash
g++ -O3 -o step2_test step2_test.cpp rsa.cpp montgomery.cpp -std=c++17
./step2_test
```

测试内容包括：
- 512 位和 768 位密钥生成
- 验证 n = p × q
- 验证 e × d ≡ 1 (mod φ(n))
- 验证 gcd(e, φ(n)) = 1
- 使用 Python `sympy.isprime()` 验证 p 和 q 的素性

### 测试结果

```
===== 测试 512 位密钥生成 =====
[PASS] n = p * q
[PASS] e * d ≡ 1 (mod phi)
[PASS] gcd(e, phi) = 1
[PASS] p is prime (verified by Python)
[PASS] q is prime (verified by Python)

===== 测试 768 位密钥生成 =====
[PASS] n = p * q
[PASS] e * d ≡ 1 (mod phi)
[PASS] gcd(e, phi) = 1
[PASS] p is prime (verified by Python)
[PASS] q is prime (verified by Python)
```

---

## 任务 3：RSA 加解密与数字签名

### 实现概述

源代码位于 `rsa.cpp` 和 `step3_test.cpp`。

### 加密/解密

- **加密**：将消息分为固定大小的块（32 字节），每块转换为 BigNum，计算 c = m^e mod n
- **解密**：对每个密文块计算 m = c^d mod n，转换回字节串

### 数字签名/验签

- **签名**：将消息分块，对每块计算 s = m^d mod n（使用私钥）
- **验签**：对每个签名块计算 m = s^e mod n（使用公钥），恢复原始消息

### 正确性测试

测试代码位于 `step3_test.cpp`，编译运行方法：

```bash
g++ -O3 -o step3_test step3_test.cpp rsa.cpp montgomery.cpp -std=c++17
./step3_test
```

测试内容包括：
- 英文文本的加密和解密一致性验证
- 十六进制字符串的加密和解密一致性验证
- 数字签名和验签一致性验证
- 篡改签名检测（修改签名后验签应失败）

### 测试结果

```
===== 测试 RSA 加解密 =====
原始消息: "Hello, RSA! This is a test message for encryption."
密文块数: 2
解密消息: "Hello, RSA! This is a test message for encryption."
[PASS] 加解密一致性验证通过!

原始消息(hex): "deadbeefcafebabe0123456789abcdef"
解密消息(hex): "deadbeefcafebabe0123456789abcdef"
[PASS] 十六进制字符串加解密验证通过!

===== 测试 RSA 数字签名 =====
原始消息: "This message is signed by the private key owner."
[PASS] 签名验证通过!

--- 篡改测试 ---
[PASS] 篡改签名验证正确地失败了!
```

---

## 任务 4：性能优化

### 优化方法

#### 1. 蒙哥马利模乘法（Montgomery Multiplication）

源代码位于 `montgomery.h` 和 `montgomery.cpp`。

**核心思想**：蒙哥马利乘法将模乘法中的除法操作替换为移位操作，从而避免昂贵的除法运算。

**算法流程**：
1. **初始化**（`montgomeryInit`）：
   - 计算 k = m 的位数
   - 计算 R = 2^k
   - 计算 m' = -m^(-1) mod R（使用 Newton 迭代法的 `modInversePowerOf2`，非常高效）
   - 计算 R² mod m

2. **蒙哥马利约简**（`montgomeryReduce`）：
   - 计算 m_s = (T × m') mod R
   - 计算 u = (T + m_s × m) >> k
   - 若 u ≥ m，则 u = u - m

3. **快速模幂**（`modPowFast`）：
   - 将 base 和 result 转换为蒙哥马利形式
   - 在蒙哥马利域中进行 Square-and-Multiply
   - 最后将结果转回普通形式

#### 2. Newton 迭代法求模 2^k 逆元

```cpp
static BigNum modInversePowerOf2(const BigNum& a, int k)
```

传统扩展欧几里得算法求 m^(-1) mod 2^k 需要 O(k) 次大数除法。Newton 迭代法仅需 O(log k) 次迭代，每次迭代精度翻倍：

- x_0 = 1
- x_{i+1} = x_i × (2 - a × x_i) mod 2^(2i)

对于 384 位数，仅需约 9 次迭代（1→2→4→8→16→32→64→128→256→384），远快于扩展欧几里得算法。

#### 3. 其他优化

- **两阶段素数测试**：先用 3 次 Miller-Rabin 迭代快速筛除合数，再用 15 次迭代确认
- **增量搜索**：候选数每次 +2，避免重复随机数生成的开销
- **蒙哥马利上下文复用**：在 Miller-Rabin 的所有迭代中共享同一个上下文
- **mt19937_64 随机数**：使用 C++11 高质量随机数引擎替代 `srand/rand`
- **编译优化**：使用 `-O3` 编译选项

### 性能测试

测试代码位于 `step4_test.cpp`，编译运行方法：

```bash
g++ -O3 -o step4_test step4_test.cpp rsa.cpp montgomery.cpp -std=c++17
./step4_test
```

### 性能测试结果

#### 模幂运算加速

| 位数 | 普通模幂 | 蒙哥马利模幂 | 加速比 |
|------|---------|------------|--------|
| 256 位 | 0.047 秒 | 0.0023 秒 | **20.5x** |
| 384 位 | 0.104 秒 | 0.0048 秒 | **21.4x** |

蒙哥马利模乘法将模幂运算加速了约 **20 倍**，主要得益于避免了二进制长除法中的逐位处理。

#### RSA 密钥生成性能

| 密钥长度 | 平均时间 | 最小时间 | 最大时间 |
|---------|---------|---------|---------|
| RSA-512 | 0.18 秒 | 0.13 秒 | 0.26 秒 |
| RSA-768 | **0.54 秒** | 0.33 秒 | 0.73 秒 |

**RSA-768 密钥生成平均时间 0.54 秒，远低于 1 秒的要求。**

#### Miller-Rabin 素性测试性能

对于 384 位随机奇数（合数），平均检测时间约 0.002 秒。

---

## 文件说明

| 文件 | 说明 |
|------|------|
| `bignum.h` | 大整数运算库（BigNum 类） |
| `montgomery.h` | 蒙哥马利乘法接口声明 |
| `montgomery.cpp` | 蒙哥马利乘法实现（含快速模幂） |
| `rsa.h` | RSA 算法接口声明 |
| `rsa.cpp` | RSA 算法实现（密钥生成、加解密、签名） |
| `step1_test.cpp` | 任务 1 测试：大数运算正确性 |
| `step2_test.cpp` | 任务 2 测试：密钥生成正确性 |
| `step3_test.cpp` | 任务 3 测试：加解密和签名正确性 |
| `step4_test.cpp` | 任务 4 测试：性能基准测试 |

## 编译方法

```bash
# 编译所有测试
g++ -O3 -o step1_test step1_test.cpp -std=c++17
g++ -O3 -o step2_test step2_test.cpp rsa.cpp montgomery.cpp -std=c++17
g++ -O3 -o step3_test step3_test.cpp rsa.cpp montgomery.cpp -std=c++17
g++ -O3 -o step4_test step4_test.cpp rsa.cpp montgomery.cpp -std=c++17

# 运行测试
./step1_test
./step2_test
./step3_test
./step4_test
```
