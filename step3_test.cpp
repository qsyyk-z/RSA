// Step3: RSA 加解密与数字签名测试
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include "rsa.h"
using namespace std;

const int BLOCK_SIZE = 32;

static string blockToHex(const string& block) {
    ostringstream oss;
    for (unsigned char c : block)
        oss << hex << setw(2) << setfill('0') << (int)c;
    return oss.str();
}

static string blockToPrintable(const string& block) {
    string result;
    for (unsigned char c : block) {
        if (c >= 32 && c < 127) result += (char)c;
        else result += '.';
    }
    return result;
}

static string trimSpaces(const string& s) {
    string r = s;
    while (!r.empty() && r.back() == ' ') r.pop_back();
    return r;
}

static string padMessage(const string& msg) {
    string padded = msg;
    while (padded.size() % BLOCK_SIZE != 0) padded += ' ';
    return padded;
}

static void printKeyInfo(const RSAKey& key) {
    cout << "\n--- 所用 RSA 密钥 ---" << endl;
    cout << "  位数: " << key.bits << endl;
    cout << "  n = " << key.n << endl;
    cout << "  e = " << key.e << " (十六进制 0x" << key.e << ", 即十进制 65537)" << endl;
    cout << "  d = " << key.d << endl;
    cout << "  分块大小: " << BLOCK_SIZE << " 字节" << endl;
}

static string verifySignature(const vector<BigNum>& signature, const RSAKey& key) {
    string result;
    for (const BigNum& s : signature)
        result += rsaVerifyBlock(s, key.e, key.n).toBytes(BLOCK_SIZE);
    return result;
}

static void printEncryptProcess(const string& label, const string& plaintext, const RSAKey& key) {
    cout << "\n--- " << label << " ---" << endl;
    cout << "原文: \"" << plaintext << "\"" << endl;
    cout << "原文长度: " << plaintext.size() << " 字节" << endl;

    string padded = padMessage(plaintext);
    int num_blocks = (int)(padded.size() / BLOCK_SIZE);
    cout << "填充后长度: " << padded.size() << " 字节 (空格填充至 "
         << BLOCK_SIZE << " 的倍数, 共 " << num_blocks << " 块)" << endl;

    cout << "\n[加密过程] c = m^e mod n" << endl;
    vector<BigNum> ciphertext;
    for (int i = 0; i < num_blocks; i++) {
        string block = padded.substr(i * BLOCK_SIZE, BLOCK_SIZE);
        BigNum m(block.c_str(), (int)block.size());
        BigNum c = rsaEncryptBlock(m, key.e, key.n);
        ciphertext.push_back(c);

        cout << "  块[" << i << "] 明文(可读): \"" << blockToPrintable(block) << "\"" << endl;
        cout << "  块[" << i << "] 明文(hex):  " << blockToHex(block) << endl;
        cout << "  块[" << i << "] 明文(BigNum): " << m << endl;
        cout << "  块[" << i << "] 密文 c:      " << c << endl;
        if (i + 1 < num_blocks) cout << endl;
    }

    cout << "\n密文汇总 (共 " << ciphertext.size() << " 块):" << endl;
    for (size_t i = 0; i < ciphertext.size(); i++)
        cout << "  Cipher[" << i << "] = " << ciphertext[i] << endl;

    cout << "\n[解密过程] m = c^d mod n" << endl;
    string decrypted;
    for (size_t i = 0; i < ciphertext.size(); i++) {
        BigNum m = rsaDecryptBlock(ciphertext[i], key.d, key.n);
        string block = m.toBytes(BLOCK_SIZE);
        decrypted += block;

        cout << "  块[" << i << "] 密文 c:      " << ciphertext[i] << endl;
        cout << "  块[" << i << "] 明文(BigNum): " << m << endl;
        cout << "  块[" << i << "] 明文(可读): \"" << blockToPrintable(block) << "\"" << endl;
        cout << "  块[" << i << "] 明文(hex):  " << blockToHex(block) << endl;
        if (i + 1 < ciphertext.size()) cout << endl;
    }

    cout << "\n解密结果: \"" << decrypted << "\"" << endl;
    if (trimSpaces(decrypted) == plaintext)
        cout << "[PASS] 加解密一致性验证通过" << endl;
    else
        cout << "[FAIL] 加解密不一致" << endl;
}

static void printSignProcess(const string& message, const RSAKey& key) {
    cout << "\n--- 数字签名 ---" << endl;
    cout << "原文: \"" << message << "\"" << endl;
    cout << "原文长度: " << message.size() << " 字节" << endl;

    string padded = padMessage(message);
    int num_blocks = (int)(padded.size() / BLOCK_SIZE);
    cout << "填充后长度: " << padded.size() << " 字节, 共 " << num_blocks << " 块" << endl;

    cout << "\n[签名过程] s = m^d mod n (使用私钥)" << endl;
    vector<BigNum> signature;
    for (int i = 0; i < num_blocks; i++) {
        string block = padded.substr(i * BLOCK_SIZE, BLOCK_SIZE);
        BigNum m(block.c_str(), (int)block.size());
        BigNum s = rsaSignBlock(m, key.d, key.n);
        signature.push_back(s);

        cout << "  块[" << i << "] 消息块(可读): \"" << blockToPrintable(block) << "\"" << endl;
        cout << "  块[" << i << "] 消息块(BigNum): " << m << endl;
        cout << "  块[" << i << "] 签名 s:        " << s << endl;
        if (i + 1 < num_blocks) cout << endl;
    }

    cout << "\n签名汇总 (共 " << signature.size() << " 块):" << endl;
    for (size_t i = 0; i < signature.size(); i++)
        cout << "  Sig[" << i << "] = " << signature[i] << endl;

    cout << "\n[验签过程] m = s^e mod n (使用公钥)" << endl;
    string verified;
    for (size_t i = 0; i < signature.size(); i++) {
        BigNum m = rsaVerifyBlock(signature[i], key.e, key.n);
        string block = m.toBytes(BLOCK_SIZE);
        verified += block;

        cout << "  块[" << i << "] 签名 s:        " << signature[i] << endl;
        cout << "  块[" << i << "] 恢复(BigNum):  " << m << endl;
        cout << "  块[" << i << "] 恢复(可读): \"" << blockToPrintable(block) << "\"" << endl;
        if (i + 1 < signature.size()) cout << endl;
    }

    cout << "\n验签结果: \"" << verified << "\"" << endl;
    if (trimSpaces(verified) == message)
        cout << "[PASS] 签名验证通过" << endl;
    else
        cout << "[FAIL] 签名验证失败" << endl;

    cout << "\n--- 篡改测试 ---" << endl;
    if (!signature.empty()) {
        BigNum tampered = signature[0].inc();
        cout << "篡改 Sig[0]: " << signature[0] << endl;
        cout << "         ->   " << tampered << endl;
        vector<BigNum> bad_sig = signature;
        bad_sig[0] = tampered;
        string bad_verified = verifySignature(bad_sig, key);
        cout << "篡改后验签恢复: \"" << bad_verified << "\"" << endl;
        if (trimSpaces(bad_verified) != message)
            cout << "[PASS] 篡改签名被正确拒绝" << endl;
        else
            cout << "[FAIL] 篡改签名竟然通过" << endl;
    }
}

void testEncryptDecrypt(const RSAKey& key) {
    cout << "\n===== RSA 加解密 =====" << endl;
    printEncryptProcess("样例1: 英文文本",
        "Hello, RSA! This is a test message for encryption.", key);
    printEncryptProcess("样例2: 十六进制字符串",
        "deadbeefcafebabe0123456789abcdef", key);
}

void testSignVerify(const RSAKey& key) {
    cout << "\n===== RSA 数字签名 =====" << endl;
    printSignProcess("This message is signed by the private key owner.", key);
}

int main() {
    cout << "========================================" << endl;
    cout << "  Step3: RSA 加解密和数字签名测试" << endl;
    cout << "========================================" << endl;

    setRSAOptimLevel(0);
    cout << "\n生成 512 位 RSA 密钥 (" << rsaOptimLevelName() << ")..." << endl;
    RSAKey key = generateRSAKey(512);
    printKeyInfo(key);
    testEncryptDecrypt(key);
    testSignVerify(key);

    cout << "\n========================================" << endl;
    cout << "  Step3 测试完成" << endl;
    cout << "========================================" << endl;
    return 0;
}
