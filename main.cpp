#include <iostream>
#include <string>

#include "inc/rsa.hpp"
#include <gmp.h>
#include <gmpxx.h>

int main() {
  rsa::rsaKey key = rsa::generate_rsa();
  std::string message = "This is a message.:)";
  mpz_class mpz_message = rsa::string_to_mpz(message);

  mpz_class cypher = rsa::rsa_encrypt(key, mpz_message);
  std::cout << "cipher: " << cypher.get_mpz_t() << std::endl;

  mpz_class decrypted = rsa::rsa_decrypt(key, cypher);
  std::string decrypted_string = rsa::mpz_to_string(decrypted);
  std::cout << "clear: " << decrypted_string << std::endl;
}
