#include <iostream>
#include <string>

#include "inc/rsa.hpp"
#include <gmp.h>
#include <gmpxx.h>

int main() {
  std::string filePath = "/home/noname/projects/kryptor/test/test.txt";

  rsa::rsaKey key = rsa::generate_rsa();
  std::string message = "This is a message.:)";
  // mpz_class mpz_message = rsa::string_to_mpz(message);
  mpz_class mpz_message;
  mpz_class c;
  rsa::rsa_encrypt(key, message, c);
  gmp_printf("ciphertext: \n%Zd\n", c.get_mpz_t());
  std::cout << "ciphertext: \n" << c.get_str(10) << std::endl;

  mpz_class decrypted = rsa::rsa_decrypt(key, c);
  std::string decrypted_string = rsa::mpz_to_string(decrypted);
  std::cout << "clear: " << decrypted_string << std::endl;

  rsa::rsa_encrypt_file(key, "/home/noname/projects/kryptor/test/test.txt",
                        "/home/noname/projects/kryptor/test/test.enc");
  rsa::rsa_decrypt_file(key, "/home/noname/projects/kryptor/test/test.enc",
                        "/home/noname/projects/kryptor/test/test2.txt");
  std::cout << "\n\nReadFileBinary:" << std::endl;
  rsa::printFileBin(filePath);
}
