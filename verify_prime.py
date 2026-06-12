n = int('e8becab607c34b74bf1d0eb4cb01167ad5e433596be8caf6604aae099e5730eab1fb97e99b7e897519db8eaa3db3ed49', 16)
from sympy import isprime
try:
    result = isprime(n)
    print(f'PRIME={result}')
except:
    # fallback: use gmpy2
    import gmpy2
    result = gmpy2.is_prime(n)
    print(f'PRIME={result}')
