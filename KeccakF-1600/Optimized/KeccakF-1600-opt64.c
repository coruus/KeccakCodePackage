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

#include "./KeccakF-1600-opt64-settings.h"
#include "KeccakF-1600/KeccakF-1600-interface.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#if defined(__GNUC__)
#define ALIGN __attribute__((aligned(32)))
#elif defined(_MSC_VER)
#define ALIGN __declspec(align(32))
#else
#define ALIGN
#endif

#if defined(UseLaneComplementing)
#define UseBebigokimisa
#endif

#if defined(_MSC_VER)
#define ROL64(a, offset) _rotl64(a, offset)
#elif defined(UseSHLD)
#define ROL64(x, N)                                             \
  ({                                                            \
    register uint64_t __out;                                      \
    register uint64_t __in = x;                                   \
    __asm__("shld %2,%0,%0" : "=r"(__out) : "0"(__in), "i"(N)); \
    __out;                                                      \
  })
#else
#define ROL64(a, offset) \
  ((((uint64_t)a) << offset) ^ (((uint64_t)a) >> (64 - offset)))
#endif

#include "KeccakF-1600-64.macros"
#include "KeccakF-1600-unrolling.macros"

/* ---------------------------------------------------------------- */

void KeccakF1600_Initialize(void) {}

/* ---------------------------------------------------------------- */

void KeccakF1600_StateInitialize(void* state) {
  memset(state, 0, 200);
#ifdef UseLaneComplementing
  ((uint64_t*)state)[1] = ~(uint64_t)0;
  ((uint64_t*)state)[2] = ~(uint64_t)0;
  ((uint64_t*)state)[8] = ~(uint64_t)0;
  ((uint64_t*)state)[12] = ~(uint64_t)0;
  ((uint64_t*)state)[17] = ~(uint64_t)0;
  ((uint64_t*)state)[20] = ~(uint64_t)0;
#endif
}

/* ---------------------------------------------------------------- */

void KeccakF1600_StateXORBytesInLane(void* state,
                                     unsigned int lanePosition,
                                     const unsigned char* data,
                                     unsigned int offset,
                                     unsigned int length) {
  if (length == 0) return;
  uint64_t lane = 0;
  memcpy(&lane, data, length);
  lane <<= (8 - length) * 8;
  lane >>= (8 - length - offset) * 8;
  ((uint64_t*)state)[lanePosition] ^= lane;
}

/* ---------------------------------------------------------------- */

void KeccakF1600_StateXORLanes(void* state,
                               const unsigned char* data,
                               unsigned int laneCount) {
  unsigned int i = 0;
  // If either pointer is misaligned, fall back to byte-wise xor.
  if (((((uintptr_t)state) & 7) != 0) || ((((uintptr_t)data) & 7) != 0)) {
    for (i = 0; i < laneCount * 8; i++) {
      ((unsigned char*)state)[i] ^= data[i];
    }
  } else {
    // Otherwise...
    for (; (i + 8) <= laneCount; i += 8) {
      ((uint64_t*)state)[i + 0] ^= ((uint64_t*)data)[i + 0];
      ((uint64_t*)state)[i + 1] ^= ((uint64_t*)data)[i + 1];
      ((uint64_t*)state)[i + 2] ^= ((uint64_t*)data)[i + 2];
      ((uint64_t*)state)[i + 3] ^= ((uint64_t*)data)[i + 3];
      ((uint64_t*)state)[i + 4] ^= ((uint64_t*)data)[i + 4];
      ((uint64_t*)state)[i + 5] ^= ((uint64_t*)data)[i + 5];
      ((uint64_t*)state)[i + 6] ^= ((uint64_t*)data)[i + 6];
      ((uint64_t*)state)[i + 7] ^= ((uint64_t*)data)[i + 7];
    }
    for (; (i + 4) <= laneCount; i += 4) {
      ((uint64_t*)state)[i + 0] ^= ((uint64_t*)data)[i + 0];
      ((uint64_t*)state)[i + 1] ^= ((uint64_t*)data)[i + 1];
      ((uint64_t*)state)[i + 2] ^= ((uint64_t*)data)[i + 2];
      ((uint64_t*)state)[i + 3] ^= ((uint64_t*)data)[i + 3];
    }
    for (; (i + 2) <= laneCount; i += 2) {
      ((uint64_t*)state)[i + 0] ^= ((uint64_t*)data)[i + 0];
      ((uint64_t*)state)[i + 1] ^= ((uint64_t*)data)[i + 1];
    }
    if (i < laneCount) {
      ((uint64_t*)state)[i + 0] ^= ((uint64_t*)data)[i + 0];
    }
  }
}

/* ---------------------------------------------------------------- */

void KeccakF1600_StateComplementBit(void* state, unsigned int position) {
  uint64_t lane = (uint64_t)1 << (position % 64);
  ((uint64_t*)state)[position / 64] ^= lane;
}

/* ---------------------------------------------------------------- */

void KeccakF1600_StatePermute(void* state) {
  KeccakF1600_StateXORPermuteExtract(state, 0, 0, 0, 0);
}

/* ---------------------------------------------------------------- */

void KeccakF1600_StateExtractBytesInLane(const void* state,
                                         unsigned int lanePosition,
                                         unsigned char* data,
                                         unsigned int offset,
                                         unsigned int length) {
  uint64_t lane = ((uint64_t*)state)[lanePosition];
#ifdef UseLaneComplementing
  if ((lanePosition == 1) || (lanePosition == 2) || (lanePosition == 8) ||
      (lanePosition == 12) || (lanePosition == 17) || (lanePosition == 20))
    lane = ~lane;
#endif
  {
    uint64_t lane1[1];
    lane1[0] = lane;
    memcpy(data, (uint8_t*)lane1 + offset, length);
  }
}

/* ---------------------------------------------------------------- */

void KeccakF1600_StateExtractLanes(const void* state,
                                   unsigned char* data,
                                   unsigned int laneCount) {
  memcpy(data, state, laneCount * 8);
#ifdef UseLaneComplementing
  if (laneCount > 1) {
    ((uint64_t*)data)[1] = ~((uint64_t*)data)[1];
    if (laneCount > 2) {
      ((uint64_t*)data)[2] = ~((uint64_t*)data)[2];
      if (laneCount > 8) {
        ((uint64_t*)data)[8] = ~((uint64_t*)data)[8];
        if (laneCount > 12) {
          ((uint64_t*)data)[12] = ~((uint64_t*)data)[12];
          if (laneCount > 17) {
            ((uint64_t*)data)[17] = ~((uint64_t*)data)[17];
            if (laneCount > 20) {
              ((uint64_t*)data)[20] = ~((uint64_t*)data)[20];
            }
          }
        }
      }
    }
  }
#endif
}

/* ---------------------------------------------------------------- */

void KeccakF1600_StateXORPermuteExtract(void* state,
                                        const unsigned char* inData,
                                        unsigned int inLaneCount,
                                        unsigned char* outData,
                                        unsigned int outLaneCount) {
  {
    declareABCDE
#if (Unrolling != 24)
        unsigned int i;
#endif
    uint64_t* stateAsLanes = (uint64_t*)state;
    uint64_t* inDataAsLanes = (uint64_t*)inData;
    uint64_t* outDataAsLanes = (uint64_t*)outData;

    copyFromStateAndXOR(A, stateAsLanes, inDataAsLanes, inLaneCount)
    rounds copyToStateAndOutput(A, stateAsLanes, outDataAsLanes, outLaneCount)
  }
}

/* ---------------------------------------------------------------- */
