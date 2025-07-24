#include <random>
#include <stdexcept>
#include <string>

#include "../inc/rsa.hpp"
#include <gmp.h>
#include <gmpxx.h>

namespace rsa {

gmp_randclass &global_rng() {
  static gmp_randclass rng(gmp_randinit_default);
  static bool seeded = false;
  if (!seeded) {
    std::random_device rd;
    // Combine several 32-bit draws into a 64-bit seed
    unsigned long seed = ((unsigned long)rd() << 32) ^ rd();
    rng.seed(seed);
    seeded = true;
  }
  return rng;
}

mpz_class random_kbit_odd(size_t k) {
  gmp_randclass &rng = global_rng();
  mpz_class x = rng.get_z_bits(k);  // uniform k-bit integer
  mpz_setbit(x.get_mpz_t(), 0);     // ensure odd
  mpz_setbit(x.get_mpz_t(), k - 1); // ensure top bit set -> k bits
  return x;
}

mpz_class random_prime(size_t k) {
  mpz_class cand = random_kbit_odd(k);
  // Quick small-factor trial: divide by small primes if you like (optional).
  // Let GMP walk to the next probable prime:
  mpz_nextprime(cand.get_mpz_t(), cand.get_mpz_t());
  return cand;
}

mpz_class gen_probable_prime(size_t k, int reps) {
  gmp_randclass &rng = global_rng();
  while (true) {
    mpz_class cand = random_kbit_odd(k);
    // Miller-Rabin reps internal to GMP prob test
    int result = mpz_probab_prime_p(cand.get_mpz_t(), reps);
    if (result > 0) { // 1 = probably, 2 = definitely (for small)
      return cand;
    }
  }
}

rsaKey generate_rsa(size_t bits, const mpz_class &e) {
  size_t pb = bits / 2;
  size_t qb = bits - pb;

  mpz_class p, q, phi, lamda;

  // Generate p and q until gcd(p-1,e)==1
  while (true) {
    p = gen_probable_prime(pb);
    mpz_class pm1 = p - 1;
    if (mpz_gcd_ui(nullptr, pm1.get_mpz_t(), e.get_ui()) == 1)
      break;
  }
  while (true) {
    q = gen_probable_prime(qb);
    if (q == p)
      continue;
    mpz_class qm1 = q - 1;
    if (mpz_gcd_ui(nullptr, qm1.get_mpz_t(), e.get_ui()) == 1)
      break;
  }

  rsaKey key;
  key.p = p;
  key.q = q;
  key.n = p * q;

  phi = (p - 1) * (q - 1);

  if (mpz_invert(key.d.get_mpz_t(), e.get_mpz_t(), phi.get_mpz_t()) == 0) {
    throw std::runtime_error(
        "e not invertible mod phi(n) (shouldn't happen if gcd checks passed");
  }

  key.e = e;

  key.dp = key.d % (p - 1);
  key.dq = key.d % (q - 1);
  mpz_invert(key.qinv.get_mpz_t(), q.get_mpz_t(), p.get_mpz_t());

  return key;
}

mpz_class string_to_mpz(std::string &s) {
  mpz_class m;
  mpz_import(m.get_mpz_t(), s.size(), 1, 1, 0, 0, s.data());

  return m;
}

std::string mpz_to_string(mpz_class m) {
  std::string s;
  size_t count;
  s.resize((mpz_sizeinbase(m.get_mpz_t(), 2) + 7) / 8);
  mpz_export(s.data(), &count, 1, 1, 0, 0, m.get_mpz_t());
  s.resize(count);

  return s;
}

mpz_class rsa_encrypt(const rsaKey &key, const mpz_class &m) {
  mpz_class c;
  mpz_powm(c.get_mpz_t(), m.get_mpz_t(), key.e.get_mpz_t(), key.n.get_mpz_t());

  return c;
}

mpz_class rsa_decrypt(const rsaKey &k, const mpz_class &c) {
  // m1 = c^dp mod p
  mpz_class m1, m2, h, m;
  mpz_powm(m1.get_mpz_t(), c.get_mpz_t(), k.dp.get_mpz_t(), k.p.get_mpz_t());
  mpz_powm(m2.get_mpz_t(), c.get_mpz_t(), k.dq.get_mpz_t(), k.q.get_mpz_t());

  // h = (qinv * (m1 - m2)) mod p
  h = m1 - m2;
  h %= k.p;
  if (h < 0)
    h += k.p;
  h *= k.qinv;
  h %= k.p;

  // m = m2 + q * h
  m = m2 + k.q * h;
  m %= k.n;
  return m;
}
} // namespace rsa
