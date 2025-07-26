#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

#include "../inc/rsa.hpp"
#include <gmp.h>
#include <gmpxx.h>

namespace rsa {

namespace {
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

mpz_class gen_probable_prime(size_t k, int reps = 40) {
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

void writeMpzToFile(const mpz_class &value, const std::string &path,
                    int base = 10) // 2–36 possible
{
  std::ofstream file(path, std::ios::trunc);
  if (!file)
    throw std::runtime_error("Cannot open file for writing: " + path);

  file << value.get_str(base); // textual form
  if (!file)
    throw std::runtime_error("Failed while writing: " + path);
}

/* ---------- 2. Compact binary representation ---------- */
void writeMpzToBinaryFile(const mpz_class &value, const std::string &path) {
  // Allocate exactly as many bytes as needed
  size_t byteCount = (mpz_sizeinbase(value.get_mpz_t(), 2) + 7) / 8;
  std::vector<unsigned char> buf(byteCount);

  // Export big‑endian, one byte per limb, most‑significant limb first
  mpz_export(buf.data(), &byteCount, 1 /*order*/, 1 /*size*/, 1 /*endian*/,
             0 /*nail*/, value.get_mpz_t());

  std::ofstream file(path, std::ios::binary | std::ios::trunc);
  if (!file)
    throw std::runtime_error("Cannot open file for writing: " + path);

  file.write(reinterpret_cast<const char *>(buf.data()), byteCount);
  if (!file)
    throw std::runtime_error("Failed while writing (binary): " + path);
}

std::string readFileToString(const std::string &path) {
  std::ifstream file(path,
                     std::ios::ate); // open and seek to end
  if (!file)
    throw std::runtime_error("Could not open file: " + path);

  const std::streamsize size = file.tellg(); // file size in bytes
  file.seekg(0);                             // rewind to beginning

  std::string buffer(static_cast<std::size_t>(size), '\0'); // allocate string
  if (!file.read(&buffer[0], size))                         // read raw bytes
    throw std::runtime_error("Error reading file: " + path);

  std::cout << "Here is file to string: \n" << buffer << std::endl;

  return buffer;
}

void printFile(const std::string &path) {
  std::ifstream file(path,
                     std::ios::binary | std::ios::ate); // open and seek to end
  if (!file)
    throw std::runtime_error("Could not open file: " + path);

  const std::streamsize size = file.tellg(); // file size in bytes
  file.seekg(0);                             // rewind to beginning

  std::string buffer(static_cast<std::size_t>(size), '\0'); // allocate string
  if (!file.read(&buffer[0], size))                         // read raw bytes
    throw std::runtime_error("Error reading file: " + path);

  std::cout << "Here is file to string: \n" << buffer << std::endl;
}

void writeStringToFile(const std::string &path, const std::string &content) {
  std::ofstream file(path, std::ios::trunc); // overwrite existing file
  if (!file)
    throw std::runtime_error("Cannot open file for writing: " + path);

  file << content;
  if (!file)
    throw std::runtime_error("Failed while writing to file: " + path);
}

} // namespace
mpz_class string_to_mpz(const std::string &s) {
  mpz_class m;
  mpz_import(m.get_mpz_t(), s.size(), 1, 1, 0, 0, s.data());

  return m;
}
void printFileBin(const std::string &path) {
  std::ifstream file(path,
                     std::ios::binary | std::ios::ate); // open and seek to end
  if (!file)
    throw std::runtime_error("Could not open file: " + path);

  const std::streamsize size = file.tellg(); // file size in bytes
  file.seekg(0);                             // rewind to beginning

  std::string buffer(static_cast<std::size_t>(size), '\0'); // allocate string
  if (!file.read(&buffer[0], size))                         // read raw bytes
    throw std::runtime_error("Error reading file: " + path);

  std::cout << "Here is file to string: \n" << buffer << std::endl;
}

std::string mpz_to_string(mpz_class m) {
  std::string s;
  size_t count;
  s.resize((mpz_sizeinbase(m.get_mpz_t(), 2) + 7) / 8);
  mpz_export(s.data(), &count, 1, 1, 0, 0, m.get_mpz_t());
  s.resize(count);

  return s;
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

void rsa_encrypt(const rsaKey &key, const std::string &message, mpz_class &c) {
  std::cout << "rsa_encrypt message value as string: \n"
            << message << std::endl;
  mpz_class m = string_to_mpz(message);
  // m.set_str(message, 10);
  gmp_printf("rsa_encrypt mpz_class vlaue: \n%Zd\n", m.get_mpz_t());
  mpz_powm(c.get_mpz_t(), m.get_mpz_t(), key.e.get_mpz_t(), key.n.get_mpz_t());
  gmp_printf("rsa_encrypt after enc:\n%Zd\n", c.get_mpz_t());
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

void rsa_encrypt_file(const rsaKey &key, const std::string &inPath,
                      const std::string &outPath) {
  mpz_class c;
  std::string plaintext;

  // std::ifstream file(inPath, std::ios::binary | std::ios::ate);  // open and
  // seek to end if (!file)
  //     throw std::runtime_error("Could not open file: " + inPath);
  //
  // const std::streamsize size = file.tellg();   // file size in bytes
  // file.seekg(0);                               // rewind to beginning
  //
  // std::string buffer(static_cast<std::size_t>(size), '\0'); // allocate
  // string if (!file.read(&buffer[0], size))            // read raw bytes
  //     throw std::runtime_error("Error reading file: " + path);
  try {
    plaintext = readFileToString(inPath);
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }
  std::cout << "right before encryption: \n" << plaintext << std::endl;

  rsa_encrypt(key, plaintext, c);
  gmp_printf("right after encryption %Zd\n", c.get_mpz_t());
  std::string str = c.get_str(10);
  std::cout << "after encryption:" << str << std::endl;

  try {
    writeStringToFile(outPath, str);
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }

  printFile(outPath);

  // std::ofstream ofile(outPath, std::ios::trunc);
  // if (!ofile)
  //     throw std::runtime_error("Cannot open file for writing: " + outPath);
  //
  // ofile << c.get_str(10);             // textual form
  // if (!ofile)
  //     throw std::runtime_error("Failed while writing: " + outPath);

  return;
}

void rsa_decrypt_file(const rsaKey &k, const std::string &inPath,
                      const std::string &outPath) {
  std::string ciphertext = readFileToString(inPath);
  std::cout << "Start of rsa_decrypt_file: \n" << ciphertext << std::endl;

  mpz_class c(ciphertext, 10);
  // mpz_import(c.get_mpz_t(), ciphertext.size(), 1, 1, 0, 0,
  // ciphertext.data()); m1 = c^dp mod p
  // mpz_class m1, m2, h, m;
  // mpz_powm(m1.get_mpz_t(), c.get_mpz_t(), k.dp.get_mpz_t(), k.p.get_mpz_t());
  // mpz_powm(m2.get_mpz_t(), c.get_mpz_t(), k.dq.get_mpz_t(), k.q.get_mpz_t());
  //
  // // h = (qinv * (m1 - m2)) mod p
  // h = m1 - m2;
  // h %= k.p;
  // if (h < 0)
  //   h += k.p;
  // h *= k.qinv;
  // h %= k.p;
  //
  // // m = m2 + q * h
  // m = m2 + k.q * h;
  // m %= k.n;
  mpz_class m = rsa_decrypt(k, c);

  std::string plaintext = mpz_to_string(m);
  try {
    writeStringToFile(outPath, plaintext);
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }

  return;
}
} // namespace rsa
