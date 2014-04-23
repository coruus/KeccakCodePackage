/*
The Keccak sponge function, designed by Guido Bertoni, Joan Daemen,
Michaël Peeters and Gilles Van Assche. For more information, feedback or
questions, please refer to our website: http://keccak.noekeon.org/

Implementation by the designers,
hereby denoted as "the implementer".

To the extent possible under law, the implementer has waived all copyright
and related or neighboring rights to the source code in this file.
http://creativecommons.org/publicdomain/zero/1.0/
*/

#include "KeccakF-1600/Reference/KeccakF-1600-reference.h"
#include "Tests/displayIntermediateValues.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define nrRounds 24
uint64_t KeccakRoundConstants[nrRounds];
#define nrLanes 25
unsigned int KeccakRhoOffsets[nrLanes];

/* ---------------------------------------------------------------- */

void KeccakF1600_InitializeRoundConstants();
void KeccakF1600_InitializeRhoOffsets();
int LFSR86540(uint8_t* LFSR);

void KeccakF1600_Initialize() {
  KeccakF1600_InitializeRoundConstants();
  KeccakF1600_InitializeRhoOffsets();
}

void KeccakF1600_InitializeRoundConstants() {
  uint8_t LFSRstate = 0x01;
  unsigned int i, j, bitPosition;

  for (i = 0; i < nrRounds; i++) {
    KeccakRoundConstants[i] = 0;
    for (j = 0; j < 7; j++) {
      bitPosition = (1 << j) - 1;  // 2^j-1
      if (LFSR86540(&LFSRstate))
        KeccakRoundConstants[i] ^= (uint64_t)1 << bitPosition;
    }
  }
}

#define index(x, y) (((x) % 5) + 5 * ((y) % 5))

void KeccakF1600_InitializeRhoOffsets() {
  unsigned int x, y, t, newX, newY;

  KeccakRhoOffsets[index(0, 0)] = 0;
  x = 1;
  y = 0;
  for (t = 0; t < 24; t++) {
    KeccakRhoOffsets[index(x, y)] = ((t + 1) * (t + 2) / 2) % 64;
    newX = (0 * x + 1 * y) % 5;
    newY = (2 * x + 3 * y) % 5;
    x = newX;
    y = newY;
  }
}

int LFSR86540(uint8_t* LFSR) {
  int result = ((*LFSR) & 0x01) != 0;
  if (((*LFSR) & 0x80) != 0)
    // Primitive polynomial over GF(2): x^8+x^6+x^5+x^4+1
    (*LFSR) = ((*LFSR) << 1) ^ 0x71;
  else
    (*LFSR) <<= 1;
  return result;
}

/* ---------------------------------------------------------------- */

void KeccakF1600_StateInitialize(void* state) {
  memset(state, 0, KeccakF_width / 8);
}

/* ---------------------------------------------------------------- */

void KeccakF1600_StateXORBytesInLane(void* state,
                                     unsigned int lanePosition,
                                     const unsigned char* data,
                                     unsigned int offset,
                                     unsigned int length) {
  unsigned int i;

  for (i = 0; i < length; i++)
    ((unsigned char*)state)[lanePosition * 8 + offset + i] ^= data[i];
}

/* ---------------------------------------------------------------- */

void KeccakF1600_StateXORLanes(void* state,
                               const unsigned char* data,
                               unsigned int laneCount) {
  unsigned int i;

  for (i = 0; i < laneCount * 8; i++)
    ((unsigned char*)state)[i] ^= data[i];
}

/* ---------------------------------------------------------------- */

void KeccakF1600_StateComplementBit(void* state, unsigned int position) {
  if (position < 1600) {
    unsigned int bytePosition = position / 8;
    unsigned int bitPosition = position % 8;

    ((unsigned char*)state)[bytePosition] ^= (uint8_t)1 << bitPosition;
  }
}

/* ---------------------------------------------------------------- */

void fromBytesToWords(uint64_t* stateAsWords, const unsigned char* state);
void fromWordsToBytes(unsigned char* state, const uint64_t* stateAsWords);
void KeccakF1600OnWords(uint64_t* state);
void theta(uint64_t* A);
void rho(uint64_t* A);
void pi(uint64_t* A);
void chi(uint64_t* A);
void iota(uint64_t* A, unsigned int indexRound);

void KeccakF1600_StatePermute(void* state) {

  displayStateAsBytes(1, "Input of permutation", (const unsigned char*)state);
  KeccakF1600OnWords((uint64_t*)state);
  displayStateAsBytes(
      1, "State after permutation", (const unsigned char*)state);
}

void fromBytesToWords(uint64_t* stateAsWords, const unsigned char* state) {
  unsigned int i, j;

  for (i = 0; i < nrLanes; i++) {
    stateAsWords[i] = 0;
    for (j = 0; j < (64 / 8); j++)
      stateAsWords[i] |= (uint64_t)(state[i * (64 / 8) + j]) << (8 * j);
  }
}

void fromWordsToBytes(unsigned char* state, const uint64_t* stateAsWords) {
  unsigned int i, j;

  for (i = 0; i < nrLanes; i++)
    for (j = 0; j < (64 / 8); j++)
      state[i * (64 / 8) + j] = (stateAsWords[i] >> (8 * j)) & 0xFF;
}

void KeccakF1600OnWords(uint64_t* state) {
  unsigned int i;

  displayStateAs64bitWords(3, "Same, with lanes as 64-bit words", state);

  for (i = 0; i < nrRounds; i++) {
    displayRoundNumber(3, i);

    theta(state);
    displayStateAs64bitWords(3, "After theta", state);

    rho(state);
    displayStateAs64bitWords(3, "After rho", state);

    pi(state);
    displayStateAs64bitWords(3, "After pi", state);

    chi(state);
    displayStateAs64bitWords(3, "After chi", state);

    iota(state, i);
    displayStateAs64bitWords(3, "After iota", state);
  }
}

#define ROL64(a, offset)                                                      \
  ((offset != 0) ? ((((uint64_t)a) << offset) ^ (((uint64_t)a) >> (64 - offset))) \
                 : a)

void theta(uint64_t* A) {
  unsigned int x, y;
  uint64_t C[5], D[5];

  for (x = 0; x < 5; x++) {
    C[x] = 0;
    for (y = 0; y < 5; y++)
      C[x] ^= A[index(x, y)];
  }
  for (x = 0; x < 5; x++)
    D[x] = ROL64(C[(x + 1) % 5], 1) ^ C[(x + 4) % 5];
  for (x = 0; x < 5; x++)
    for (y = 0; y < 5; y++)
      A[index(x, y)] ^= D[x];
}

void rho(uint64_t* A) {
  unsigned int x, y;

  for (x = 0; x < 5; x++)
    for (y = 0; y < 5; y++)
      A[index(x, y)] = ROL64(A[index(x, y)], KeccakRhoOffsets[index(x, y)]);
}

void pi(uint64_t* A) {
  unsigned int x, y;
  uint64_t tempA[25];

  for (x = 0; x < 5; x++)
    for (y = 0; y < 5; y++)
      tempA[index(x, y)] = A[index(x, y)];
  for (x = 0; x < 5; x++)
    for (y = 0; y < 5; y++)
      A[index(0 * x + 1 * y, 2 * x + 3 * y)] = tempA[index(x, y)];
}

void chi(uint64_t* A) {
  unsigned int x, y;
  uint64_t C[5];

  for (y = 0; y < 5; y++) {
    for (x = 0; x < 5; x++)
      C[x] = A[index(x, y)] ^ ((~A[index(x + 1, y)]) & A[index(x + 2, y)]);
    for (x = 0; x < 5; x++)
      A[index(x, y)] = C[x];
  }
}

void iota(uint64_t* A, unsigned int indexRound) {
  A[index(0, 0)] ^= KeccakRoundConstants[indexRound];
}

/* ---------------------------------------------------------------- */

void KeccakF1600_StateExtractBytesInLane(const void* state,
                                         unsigned int lanePosition,
                                         unsigned char* data,
                                         unsigned int offset,
                                         unsigned int length) {
  memcpy(data, (unsigned char*)state + lanePosition * 8 + offset, length);
}

/* ---------------------------------------------------------------- */

void KeccakF1600_StateExtractLanes(const void* state,
                                   unsigned char* data,
                                   unsigned int laneCount) {
  memcpy(data, state, laneCount * 8);
}

/* ---------------------------------------------------------------- */

void KeccakF1600_StateXORPermuteExtract(void* state,
                                        const unsigned char* inData,
                                        unsigned int inLaneCount,
                                        unsigned char* outData,
                                        unsigned int outLaneCount) {
  KeccakF1600_StateXORLanes(state, inData, inLaneCount);
  KeccakF1600_StatePermute(state);
  KeccakF1600_StateExtractLanes(state, outData, outLaneCount);
}

/* ---------------------------------------------------------------- */

void displayRoundConstants(FILE* f) {
  unsigned int i;

  for (i = 0; i < nrRounds; i++) {
    fprintf(f, "RC[%02i][0][0] = ", i);
    fprintf(f, "%08X", (unsigned int)(KeccakRoundConstants[i] >> 32));
    fprintf(f, "%08X", (unsigned int)(KeccakRoundConstants[i] & 0xFFFFFFFFULL));
    fprintf(f, "\n");
  }
  fprintf(f, "\n");
}

void displayRhoOffsets(FILE* f) {
  unsigned int x, y;

  for (y = 0; y < 5; y++)
    for (x = 0; x < 5; x++) {
      fprintf(f, "RhoOffset[%i][%i] = ", x, y);
      fprintf(f, "%2i", KeccakRhoOffsets[index(x, y)]);
      fprintf(f, "\n");
    }
  fprintf(f, "\n");
}
