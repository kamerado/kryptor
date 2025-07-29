#include <cstddef>
#include <string>

#include <gmp.h>
#include <gmpxx.h>

namespace aes {
// stuff
//
void keyScheduling();
std::byte *cipher128(std::byte in[16], std::byte out[16], std::byte w[4 * 11]) {
  std::byte state[4][4];

  // state = in; // Implement method to handle assinment

  // AddRoundKey(state, w[0, Nb-1]);

  for (int round = 0; round < 10; round++) {
    // Subbytes(state);
    // ShiftRows(state);
    // MixColumns(state);
    // AddRoundKey(state, w[round*Nb], (round + 1) *Nb-1);
  }

  // Subbytes(state);
  // ShiftRows(state);
  // AddRoundKey(state, w[round*Nb, (Nr+1)*Nb-1]);

  // out = state; // Again, implement method to handle assignment

  return out;
}
} // namespace aes
