// ============================================================
// Step3 测试: RSA 加解密和数字签名测试
// ============================================================
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "bignum.h"
#include "montgomery.h"
#include "rsa.h"
using namespace std;

// 每个加密块的字节数（根据密钥位数调整）
// 对于 512 位密钥，n 最多 64 字节，我们用 32 字节一块比较安全
#define BLOCK_SIZE 32

// 将字符串分块加密
vector<BigNum> encryptString(const string& plaintext, const RSAKey& key) {
    vector<BigNum> ciphertext;
    string padded = plaintext;
    // 填充到 BLOCK_SIZE 的倍数
    while (padded.size() % BLOCK_SIZE != 0) {
        padded += ' ';
    }

    for (size_t i = 0; i < padded.size(); i += BLOCK_SIZE) {
        string block = padded.substr(i, BLOCK_SIZE);
        BigNum m(block.c_str(), block.size());
        BigNum c = rsaEncryptBlock(m, key.e, key.n);
        ciphertext.push_back(c);
    }
    return ciphertext;
}

// 将密文分块解密
string decryptBlocks(const vector<BigNum>& ciphertext, const RSAKey& key) {
    string result;
    for (const BigNum& c : ciphertext) {
        BigNum m = rsaDecryptBlock(c, key.d, key.n);
        result += m.toBytes(BLOCK_SIZE);
    }
    return result;
}

// 签名
vector<BigNum> signString(const string& message, const RSAKey& key) {
    vector<BigNum> signature;
    string padded = message;
    while (padded.size() % BLOCK_SIZE != 0) {
        padded += ' ';
    }

    for (size_t i = 0; i < padded.size(); i += BLOCK_SIZE) {
        string block = padded.substr(i, BLOCK_SIZE);
        BigNum m(block.c_str(), block.size());
        BigNum s = rsaSignBlock(m, key.d, key.n);
        signature.push_back(s);
    }
    return signature;
}

// 验签
string verifySignature(const vector<BigNum>& signature, const RSAKey& key) {
    string result;
    for (const BigNum& s : signature) {
        BigNum m = rsaVerifyBlock(s, key.e, key.n);
        result += m.toBytes(BLOCK_SIZE);
    }
    return result;
}

void testEncryptDecrypt(const RSAKey& key) {
    cout << "\n===== 测试 RSA 加解密 =====" << endl;

    // 测试样例 1: 英文文本
    string msg1 = "Hello, RSA! This is a test message for encryption.";
    cout << "原始消息: \"" << msg1 << "\"" << endl;

    vector<BigNum> cipher = encryptString(msg1, key);
    cout << "密文块数: " << cipher.size() << endl;
    for (size_t i = 0; i < cipher.size(); i++) {
        cout << "  Block[" << i << "] = " << cipher[i] << endl;
    }

    string decrypted = decryptBlocks(cipher, key);
    cout << "解密消息: \"" << decrypted << "\"" << endl;

    // 去掉尾部空格比较
    string msg1_trimmed = msg1;
    string dec_trimmed = decrypted;
    while (!dec_trimmed.empty() && dec_trimmed.back() == ' ') dec_trimmed.pop_back();
    if (msg1_trimmed == dec_trimmed) {
        cout << "[PASS] 加解密一致性验证通过!" << endl;
    } else {
        cout << "[FAIL] 加解密不一致!" << endl;
    }

    // 测试样例 2: 十六进制字符串
    string msg2 = "deadbeefcafebabe0123456789abcdef";
    cout << "\n原始消息(hex): \"" << msg2 << "\"" << endl;

    vector<BigNum> cipher2 = encryptString(msg2, key);
    string decrypted2 = decryptBlocks(cipher2, key);
    cout << "解密消息(hex): \"" << decrypted2 << "\"" << endl;

    string msg2_trimmed = msg2;
    string dec2_trimmed = decrypted2;
    while (!dec2_trimmed.empty() && dec2_trimmed.back() == ' ') dec2_trimmed.pop_back();
    if (msg2_trimmed == dec2_trimmed) {
        cout << "[PASS] 十六进制字符串加解密验证通过!" << endl;
    } else {
        cout << "[FAIL] 十六进制字符串加解密不一致!" << endl;
    }
}

void testSignVerify(const RSAKey& key) {
    cout << "\n===== 测试 RSA 数字签名 =====" << endl;

    string message = "This message is signed by the private key owner.";
    cout << "原始消息: \"" << message << "\"" << endl;

    // 签名
    vector<BigNum> signature = signString(message, key);
    cout << "签名块数: " << signature.size() << endl;
    for (size_t i = 0; i < signature.size(); i++) {
        cout << "  Sig[" << i << "] = " << signature[i] << endl;
    }

    // 验签
    string verified = verifySignature(signature, key);
    cout << "验签消息: \"" << verified << "\"" << endl;

    string msg_trimmed = message;
    string ver_trimmed = verified;
    while (!ver_trimmed.empty() && ver_trimmed.back() == ' ') ver_trimmed.pop_back();
    if (msg_trimmed == ver_trimmed) {
        cout << "[PASS] 签名验证通过!" << endl;
    } else {
        cout << "[FAIL] 签名验证失败!" << endl;
    }

    // 篡改测试：修改签名后验签应失败
    cout << "\n--- 篡改测试 ---" << endl;
    vector<BigNum> tampered_sig = signature;
    if (!tampered_sig.empty()) {
        tampered_sig[0] = tampered_sig[0].inc(); // 篡改第一个块
        string tampered_verified = verifySignature(tampered_sig, key);
        string tam_trimmed = tampered_verified;
        while (!tam_trimmed.empty() && tam_trimmed.back() == ' ') tam_trimmed.pop_back();
        if (msg_trimmed != tam_trimmed) {
            cout << "[PASS] 篡改签名验证正确地失败了!" << endl;
        } else {
            cout << "[FAIL] 篡改签名竟然验证通过了!" << endl;
        }
    }
}

int main() {
    cout << "========================================" << endl;
    cout << "  Step3: RSA 加解密和数字签名测试" << endl;
    cout << "========================================" << endl;

    // 生成 512 位密钥用于测试
    cout << "\n生成 512 位 RSA 密钥..." << endl;
    RSAKey key = generateRSAKey(512);

    testEncryptDecrypt(key);
    testSignVerify(key);

    cout << "\n========================================" << endl;
    cout << "  Step3 测试完成" << endl;
    cout << "========================================" << endl;

    return 0;
}
