#include <cstddef>
#include <string>

#include <gmp.h>
#include <gmpxx.h>

namespace aes {
// stuff
//
void keyScheduling();
std::byte cipher128(std::byte in[16], std::byte out[16], std::byte w[4 * 11]);
} // namespace aes
