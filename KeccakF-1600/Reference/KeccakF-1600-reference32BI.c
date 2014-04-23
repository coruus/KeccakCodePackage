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
#include "Tests/displayIntermediateValues.h"
#include "KeccakF-1600/KeccakF-1600-interface.h"

#include <stdio.h>
#include <string.h>

#define nrRounds 24
uint32_t KeccakRoundConstants[nrRounds][2];
#define nrLanes 25
unsigned int KeccakRhoOffsets[nrLanes];

/* ---------------------------------------------------------------- */

void toBitInterleaving(uint32_t low, uint32_t high, uint32_t* even, uint32_t* odd);
void fromBitInterleaving(uint32_t even, uint32_t odd, uint32_t* low, uint32_t* high);

void toBitInterleaving(uint32_t low, uint32_t high, uint32_t* even, uint32_t* odd) {
  unsigned int i;

  *even = 0;
  *odd = 0;
  for (i = 0; i < 64; i++) {
    unsigned int inBit;
    if (i < 32)
      inBit = (low >> i) & 1;
    else
      inBit = (high >> (i - 32)) & 1;
    if ((i % 2) == 0)
      *even |= inBit << (i / 2);
    else
      *odd |= inBit << ((i - 1) / 2);
  }
}

void fromBitInterleaving(uint32_t even, uint32_t odd, uint32_t* low, uint32_t* high) {
  unsigned int i;

  *low = 0;
  *high = 0;
  for (i = 0; i < 64; i++) {
    unsigned int inBit;
    if ((i % 2) == 0)
      inBit = (even >> (i / 2)) & 1;
    else
      inBit = (odd >> ((i - 1) / 2)) & 1;
    if (i < 32)
      *low |= inBit << i;
    else
      *high |= inBit << (i - 32);
  }
}

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
  uint32_t low, high;

  for (i = 0; i < nrRounds; i++) {
    low = high = 0;
    for (j = 0; j < 7; j++) {
      bitPosition = (1 << j) - 1;  // 2^j-1
      if (LFSR86540(&LFSRstate)) {
        if (bitPosition < 32)
          low ^= (uint32_t)1 << bitPosition;
        else
          high ^= (uint32_t)1 << (bitPosition - 32);
      }
    }
    toBitInterleaving(low,
                      high,
                      &(KeccakRoundConstants[i][0]),
                      &(KeccakRoundConstants[i][1]));
  }
}

void KeccakF1600_InitializeRhoOffsets() {
  unsigned int x, y, t, newX, newY;

  KeccakRhoOffsets[0] = 0;
  x = 1;
  y = 0;
  for (t = 0; t < 24; t++) {
    KeccakRhoOffsets[5 * y + x] = ((t + 1) * (t + 2) / 2) % 64;
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
  if ((lanePosition < 25) && (offset < 8) && (offset + length <= 8)) {
    uint8_t laneAsBytes[8];
    uint32_t low, high;
    uint32_t lane[2];
    uint32_t* stateAsHalfLanes;

    memset(laneAsBytes, 0, 8);
    memcpy(laneAsBytes + offset, data, length);
    low = laneAsBytes[0] | ((uint32_t)(laneAsBytes[1]) << 8) |
          ((uint32_t)(laneAsBytes[2]) << 16) | ((uint32_t)(laneAsBytes[3]) << 24);
    high = laneAsBytes[4] | ((uint32_t)(laneAsBytes[5]) << 8) |
           ((uint32_t)(laneAsBytes[6]) << 16) | ((uint32_t)(laneAsBytes[7]) << 24);
    toBitInterleaving(low, high, lane, lane + 1);
    stateAsHalfLanes = (uint32_t*)state;
    stateAsHalfLanes[lanePosition * 2 + 0] ^= lane[0];
    stateAsHalfLanes[lanePosition * 2 + 1] ^= lane[1];
  }
}

/* ---------------------------------------------------------------- */

void KeccakF1600_StateXORLanes(void* state,
                               const unsigned char* data,
                               unsigned int laneCount) {
  if (laneCount <= 25) {
    unsigned int lanePosition;
    for (lanePosition = 0; lanePosition < laneCount; lanePosition++) {
      uint8_t laneAsBytes[8];
      uint32_t low, high;
      uint32_t lane[2];
      uint32_t* stateAsHalfLanes;

      memcpy(laneAsBytes, data + lanePosition * 8, 8);
      low = laneAsBytes[0] | ((uint32_t)(laneAsBytes[1]) << 8) |
            ((uint32_t)(laneAsBytes[2]) << 16) | ((uint32_t)(laneAsBytes[3]) << 24);
      high = laneAsBytes[4] | ((uint32_t)(laneAsBytes[5]) << 8) |
             ((uint32_t)(laneAsBytes[6]) << 16) |
             ((uint32_t)(laneAsBytes[7]) << 24);
      toBitInterleaving(low, high, lane, lane + 1);
      stateAsHalfLanes = (uint32_t*)state;
      stateAsHalfLanes[lanePosition * 2 + 0] ^= lane[0];
      stateAsHalfLanes[lanePosition * 2 + 1] ^= lane[1];
    }
  }
}

/* ---------------------------------------------------------------- */

void KeccakF1600_StateComplementBit(void* state, unsigned int position) {
  if (position < 1600) {
    uint32_t* stateAsHalfLanes = (uint32_t*)state;
    unsigned int lanePosition = position / 64;
    unsigned int zeta = position % 2;
    unsigned int bitInLane = (position % 64) / 2;
    stateAsHalfLanes[lanePosition * 2 + zeta] ^= (uint32_t)1 << bitInLane;
  }
}

/* ---------------------------------------------------------------- */

void KeccakF1600_PermutationOnWords(uint32_t* state);
void theta(uint32_t* A);
void rho(uint32_t* A);
void pi(uint32_t* A);
void chi(uint32_t* A);
void iota(uint32_t* A, unsigned int indexRound);

void KeccakF1600_StatePermute(void* state) {
  uint32_t* stateAsHalfLanes = (uint32_t*)state;
  {
    uint8_t stateAsBytes[KeccakF_width / 8];
    KeccakF1600_StateExtractLanes(
        state, stateAsBytes, KeccakF_width / 8 / KeccakF_laneInBytes);
    displayStateAsBytes(1, "Input of permutation", stateAsBytes);
  }
  KeccakF1600_PermutationOnWords(stateAsHalfLanes);
  {
    uint8_t stateAsBytes[KeccakF_width / 8];
    KeccakF1600_StateExtractLanes(
        state, stateAsBytes, KeccakF_width / 8 / KeccakF_laneInBytes);
    displayStateAsBytes(1, "State after permutation", stateAsBytes);
  }
}

void KeccakF1600_PermutationOnWords(uint32_t* state) {
  unsigned int i;

  displayStateAs32bitWords(
      3, "Same, with lanes as pairs of 32-bit words (bit interleaving)", state);

  for (i = 0; i < nrRounds; i++) {
    displayRoundNumber(3, i);

    theta(state);
    displayStateAs32bitWords(3, "After theta", state);

    rho(state);
    displayStateAs32bitWords(3, "After rho", state);

    pi(state);
    displayStateAs32bitWords(3, "After pi", state);

    chi(state);
    displayStateAs32bitWords(3, "After chi", state);

    iota(state, i);
    displayStateAs32bitWords(3, "After iota", state);
  }
}

#define index(x, y, z) ((((x) % 5) + 5 * ((y) % 5)) * 2 + z)
#define ROL32(a, offset)                                                      \
  ((offset != 0) ? ((((uint32_t)a) << offset) ^ (((uint32_t)a) >> (32 - offset))) \
                 : a)

void ROL64(uint32_t inEven,
           uint32_t inOdd,
           uint32_t* outEven,
           uint32_t* outOdd,
           unsigned int offset) {
  if ((offset % 2) == 0) {
    *outEven = ROL32(inEven, offset / 2);
    *outOdd = ROL32(inOdd, offset / 2);
  } else {
    *outEven = ROL32(inOdd, (offset + 1) / 2);
    *outOdd = ROL32(inEven, (offset - 1) / 2);
  }
}

void theta(uint32_t* A) {
  unsigned int x, y, z;
  uint32_t C[5][2], D[5][2];

  for (x = 0; x < 5; x++) {
    for (z = 0; z < 2; z++) {
      C[x][z] = 0;
      for (y = 0; y < 5; y++)
        C[x][z] ^= A[index(x, y, z)];
    }
  }
  for (x = 0; x < 5; x++) {
    ROL64(C[(x + 1) % 5][0], C[(x + 1) % 5][1], &(D[x][0]), &(D[x][1]), 1);
    for (z = 0; z < 2; z++)
      D[x][z] ^= C[(x + 4) % 5][z];
  }
  for (x = 0; x < 5; x++)
    for (y = 0; y < 5; y++)
      for (z = 0; z < 2; z++)
        A[index(x, y, z)] ^= D[x][z];
}

void rho(uint32_t* A) {
  unsigned int x, y;

  for (x = 0; x < 5; x++)
    for (y = 0; y < 5; y++)
      ROL64(A[index(x, y, 0)],
            A[index(x, y, 1)],
            &(A[index(x, y, 0)]),
            &(A[index(x, y, 1)]),
            KeccakRhoOffsets[5 * y + x]);
}

void pi(uint32_t* A) {
  unsigned int x, y, z;
  uint32_t tempA[50];

  for (x = 0; x < 5; x++)
    for (y = 0; y < 5; y++)
      for (z = 0; z < 2; z++)
        tempA[index(x, y, z)] = A[index(x, y, z)];
  for (x = 0; x < 5; x++)
    for (y = 0; y < 5; y++)
      for (z = 0; z < 2; z++)
        A[index(0 * x + 1 * y, 2 * x + 3 * y, z)] = tempA[index(x, y, z)];
}

void chi(uint32_t* A) {
  unsigned int x, y, z;
  uint32_t C[5][2];

  for (y = 0; y < 5; y++) {
    for (x = 0; x < 5; x++)
      for (z = 0; z < 2; z++)
        C[x][z] = A[index(x, y, z)] ^
                  ((~A[index(x + 1, y, z)]) & A[index(x + 2, y, z)]);
    for (x = 0; x < 5; x++)
      for (z = 0; z < 2; z++)
        A[index(x, y, z)] = C[x][z];
  }
}

void iota(uint32_t* A, unsigned int indexRound) {
  A[index(0, 0, 0)] ^= KeccakRoundConstants[indexRound][0];
  A[index(0, 0, 1)] ^= KeccakRoundConstants[indexRound][1];
}

/* ---------------------------------------------------------------- */

void KeccakF1600_StateExtractBytesInLane(const void* state,
                                         unsigned int lanePosition,
                                         unsigned char* data,
                                         unsigned int offset,
                                         unsigned int length) {
  if ((lanePosition < 25) && (offset < 8) && (offset + length <= 8)) {
    uint32_t* stateAsHalfLanes = (uint32_t*)state;
    uint32_t lane[2];
    uint8_t laneAsBytes[8];
    fromBitInterleaving(stateAsHalfLanes[lanePosition * 2],
                        stateAsHalfLanes[lanePosition * 2 + 1],
                        lane,
                        lane + 1);
    laneAsBytes[0] = lane[0] & 0xFF;
    laneAsBytes[1] = (lane[0] >> 8) & 0xFF;
    laneAsBytes[2] = (lane[0] >> 16) & 0xFF;
    laneAsBytes[3] = (lane[0] >> 24) & 0xFF;
    laneAsBytes[4] = lane[1] & 0xFF;
    laneAsBytes[5] = (lane[1] >> 8) & 0xFF;
    laneAsBytes[6] = (lane[1] >> 16) & 0xFF;
    laneAsBytes[7] = (lane[1] >> 24) & 0xFF;
    memcpy(data, laneAsBytes + offset, length);
  }
}

/* ---------------------------------------------------------------- */

void KeccakF1600_StateExtractLanes(const void* state,
                                   unsigned char* data,
                                   unsigned int laneCount) {
  if (laneCount <= 25) {
    unsigned int lanePosition;
    for (lanePosition = 0; lanePosition < laneCount; lanePosition++) {
      uint32_t* stateAsHalfLanes = (uint32_t*)state;
      uint32_t lane[2];
      uint8_t laneAsBytes[8];
      fromBitInterleaving(stateAsHalfLanes[lanePosition * 2],
                          stateAsHalfLanes[lanePosition * 2 + 1],
                          lane,
                          lane + 1);
      laneAsBytes[0] = lane[0] & 0xFF;
      laneAsBytes[1] = (lane[0] >> 8) & 0xFF;
      laneAsBytes[2] = (lane[0] >> 16) & 0xFF;
      laneAsBytes[3] = (lane[0] >> 24) & 0xFF;
      laneAsBytes[4] = lane[1] & 0xFF;
      laneAsBytes[5] = (lane[1] >> 8) & 0xFF;
      laneAsBytes[6] = (lane[1] >> 16) & 0xFF;
      laneAsBytes[7] = (lane[1] >> 24) & 0xFF;
      memcpy(data + lanePosition * 8, laneAsBytes, 8);
    }
  }
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
    fprintf(f,
            "%08X:%08X",
            (unsigned int)(KeccakRoundConstants[i][0]),
            (unsigned int)(KeccakRoundConstants[i][1]));
    fprintf(f, "\n");
  }
  fprintf(f, "\n");
}

void displayRhoOffsets(FILE* f) {
  unsigned int x, y;

  for (y = 0; y < 5; y++)
    for (x = 0; x < 5; x++) {
      fprintf(f, "RhoOffset[%i][%i] = ", x, y);
      fprintf(f, "%2i", KeccakRhoOffsets[5 * y + x]);
      fprintf(f, "\n");
    }
  fprintf(f, "\n");
}
