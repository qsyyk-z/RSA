import random, math
random.seed(42)

for t in range(5):
    m = random.getrandbits(512) | 1
    a = random.getrandbits(384)
    while math.gcd(a, m) != 1:
        a = random.getrandbits(384)
    b = random.getrandbits(384)
    print(f'CASE_{t}')
    print(f'A={hex(a)}')
    print(f'B={hex(b)}')
    print(f'M={hex(m)}')
    print(f'MUL={hex(a * b)}')
    print(f'MOD={hex(a % m)}')
    print(f'POWM={hex(pow(a, b, m))}')
    print(f'INV={hex(pow(a, -1, m))}')
