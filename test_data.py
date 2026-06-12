import random
random.seed(42)

# Test case 1
a_0 = random.getrandbits(384)
b_0 = random.getrandbits(384)
m_0 = random.getrandbits(512) | 1

# Test case 2
a_1 = random.getrandbits(384)
b_1 = random.getrandbits(384)
m_1 = random.getrandbits(512) | 1

# Test case 3
a_2 = random.getrandbits(384)
b_2 = random.getrandbits(384)
m_2 = random.getrandbits(512) | 1

# Test case 4
a_3 = random.getrandbits(384)
b_3 = random.getrandbits(384)
m_3 = random.getrandbits(512) | 1

# Test case 5
a_4 = random.getrandbits(384)
b_4 = random.getrandbits(384)
m_4 = random.getrandbits(512) | 1

# Print results
print(f'CASE_0')
print(f'MUL={{hex(a_0 * b_0)}}')
print(f'MOD={{hex(a_0 % m_0)}}')
print(f'POWM={{hex(pow(a_0, b_0, m_0))}}')
a_inv_0 = pow(a_0, -1, m_0)
print(f'INV={{hex(a_inv_0)}}')
print(f'VERIFY={{hex((a_0 * a_inv_0) % m_0)}}')
print(f'CASE_1')
print(f'MUL={{hex(a_1 * b_1)}}')
print(f'MOD={{hex(a_1 % m_1)}}')
print(f'POWM={{hex(pow(a_1, b_1, m_1))}}')
a_inv_1 = pow(a_1, -1, m_1)
print(f'INV={{hex(a_inv_1)}}')
print(f'VERIFY={{hex((a_1 * a_inv_1) % m_1)}}')
print(f'CASE_2')
print(f'MUL={{hex(a_2 * b_2)}}')
print(f'MOD={{hex(a_2 % m_2)}}')
print(f'POWM={{hex(pow(a_2, b_2, m_2))}}')
a_inv_2 = pow(a_2, -1, m_2)
print(f'INV={{hex(a_inv_2)}}')
print(f'VERIFY={{hex((a_2 * a_inv_2) % m_2)}}')
print(f'CASE_3')
print(f'MUL={{hex(a_3 * b_3)}}')
print(f'MOD={{hex(a_3 % m_3)}}')
print(f'POWM={{hex(pow(a_3, b_3, m_3))}}')
a_inv_3 = pow(a_3, -1, m_3)
print(f'INV={{hex(a_inv_3)}}')
print(f'VERIFY={{hex((a_3 * a_inv_3) % m_3)}}')
print(f'CASE_4')
print(f'MUL={{hex(a_4 * b_4)}}')
print(f'MOD={{hex(a_4 % m_4)}}')
print(f'POWM={{hex(pow(a_4, b_4, m_4))}}')
a_inv_4 = pow(a_4, -1, m_4)
print(f'INV={{hex(a_inv_4)}}')
print(f'VERIFY={{hex((a_4 * a_inv_4) % m_4)}}')
