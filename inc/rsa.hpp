#include <string>

#include <gmp.h>
#include <gmpxx.h>

namespace rsa {
struct rsaKey {
  mpz_class n, e, d, p, q, dp, dq, qinv;
};
//
// gmp_randclass &global_rng();
//
// mpz_class random_kbit_odd(size_t k);
// mpz_class random_prime(size_t k);
// mpz_class gen_probable_prime(size_t k, int reps = 40);
rsaKey generate_rsa(size_t bits = 2048, const mpz_class &e = 65537);

mpz_class string_to_mpz(std::string &s);
std::string mpz_to_string(mpz_class m);
mpz_class rsa_encrypt(const rsaKey &key, const mpz_class &m);
mpz_class rsa_decrypt(const rsaKey &k, const mpz_class &c);

void rsa_encrypt_file(const rsaKey &key, const std::ifstream &m);
void rsa_decrypt_file(const rsaKey &k, const std::ofstream &c);
} // namespace rsa
