#include "Constructions/KeccakDuplex.h"
#include "Constructions/KeccakSponge.h"
#include "KeccakF-1600/KeccakF-1600-interface.h"
#include "Tests/timing.h"
#include "Tests/dotiming.h"

#include <stdio.h>
#include <stdint.h>

#define measureTimingBegin                 \
  uint32_t tMin = 0xFFFFFFFF;              \
  uint32_t t0, t1, i;                      \
  for (i = 0; i < TIMER_SAMPLE_CNT; i++) { \
    t0 = HiResTime();

#define measureTimingEnd                              \
  t1 = HiResTime();                                   \
  if (tMin > t1 - t0 - dtMin) tMin = t1 - t0 - dtMin; \
  }                                                   \
  return tMin;


static inline uint32_t measureKeccakF1600_StatePermute(uint32_t dtMin) {
  ALIGN unsigned char state[KeccakF_width / 8];

  measureTimingBegin KeccakF1600_StatePermute(state);
  measureTimingEnd
}

static inline uint32_t measureKeccakF1600_StateXORPermuteExtract_0_0(uint32_t dtMin) {
  ALIGN unsigned char state[KeccakF_width / 8];

  measureTimingBegin KeccakF1600_StateXORPermuteExtract(state, 0, 0, 0, 0);
  measureTimingEnd
}

static inline uint32_t measureKeccakF1600_StateXORPermuteExtract_16_0(uint32_t dtMin) {
  ALIGN unsigned char state[KeccakF_width / 8];
  ALIGN unsigned char data[200];

  measureTimingBegin KeccakF1600_StateXORPermuteExtract(state, data, 16, 0, 0);
  measureTimingEnd
}

static inline uint32_t measureKeccakF1600_StateXORPermuteExtract_17_0(uint32_t dtMin) {
  ALIGN unsigned char state[KeccakF_width / 8];
  ALIGN unsigned char data[200];

  measureTimingBegin KeccakF1600_StateXORPermuteExtract(state, data, 17, 0, 0);
  measureTimingEnd
}

static inline uint32_t measureKeccakF1600_StateXORPermuteExtract_21_0(uint32_t dtMin) {
  ALIGN unsigned char state[KeccakF_width / 8];
  ALIGN unsigned char data[200];

  measureTimingBegin KeccakF1600_StateXORPermuteExtract(state, data, 21, 0, 0);
  measureTimingEnd
}

static inline uint32_t measureKeccakF1600_StateXORPermuteExtract_0_16(uint32_t dtMin) {
  ALIGN unsigned char state[KeccakF_width / 8];
  ALIGN unsigned char data[200];

  measureTimingBegin KeccakF1600_StateXORPermuteExtract(state, 0, 0, data, 16);
  measureTimingEnd
}

static inline uint32_t measureKeccakF1600_StateXORPermuteExtract_0_17(uint32_t dtMin) {
  ALIGN unsigned char state[KeccakF_width / 8];
  ALIGN unsigned char data[200];

  measureTimingBegin KeccakF1600_StateXORPermuteExtract(state, 0, 0, data, 17);
  measureTimingEnd
}

static inline uint32_t measureKeccakF1600_StateXORPermuteExtract_0_21(uint32_t dtMin) {
  ALIGN unsigned char state[KeccakF_width / 8];
  ALIGN unsigned char data[200];

  measureTimingBegin KeccakF1600_StateXORPermuteExtract(state, 0, 0, data, 21);
  measureTimingEnd
}

static inline uint32_t measureKeccakAbsorb1000blocks(uint32_t dtMin) {
  Keccak_SpongeInstance sponge;
  ALIGN unsigned char data[1000 * 200];

  measureTimingBegin Keccak_SpongeInitialize(&sponge, 1344, 256);
  Keccak_SpongeAbsorb(&sponge, data, 999 * 1344 / 8 + 1);
  Keccak_SpongeAbsorbLastFewBits(&sponge, 0x01);
  measureTimingEnd
}

static inline uint32_t measureKeccakSqueeze1000blocks(uint32_t dtMin) {
  Keccak_SpongeInstance sponge;
  ALIGN unsigned char data[1000 * 200];

  measureTimingBegin Keccak_SpongeInitialize(&sponge, 1344, 256);
  Keccak_SpongeSqueeze(&sponge, data, 1000 * 1344 / 8);
  measureTimingEnd
}

static inline uint32_t measureKeccakDuplexing1000blocks(uint32_t dtMin) {
  Keccak_DuplexInstance duplex;
  int j;
  ALIGN unsigned char dataIn[200];
  ALIGN unsigned char dataOut[200];

  measureTimingBegin Keccak_DuplexInitialize(&duplex, 1344 + 3, 256 - 3);
  for (j = 0; j < 1000; j++)
    Keccak_Duplexing(&duplex, dataIn, 1344 / 8, dataOut, 1344 / 8, 0x03);
  measureTimingEnd
}

void doTiming(void) {
  uint32_t calibration;
  uint32_t measurement;

  measureKeccakAbsorb1000blocks(0);
  calibration = calibrate();

  measurement = measureKeccakF1600_StatePermute(calibration);
  printf("Cycles for KeccakF1600_StatePermute(state): %d\n\n", measurement);

  measurement = measureKeccakF1600_StateXORPermuteExtract_0_0(calibration);
  printf(
      "Cycles for KeccakF1600_StateXORPermuteExtract(state, 0, 0, 0, 0): "
      "%d\n\n",
      measurement);

  measurement = measureKeccakF1600_StateXORPermuteExtract_16_0(calibration);
  printf(
      "Cycles for KeccakF1600_StateXORPermuteExtract(state, data, 16, 0, 0): "
      "%d\n",
      measurement);
  printf("Cycles per byte for rate 1024: %f\n\n", measurement / 128.0);

  measurement = measureKeccakF1600_StateXORPermuteExtract_17_0(calibration);
  printf(
      "Cycles for KeccakF1600_StateXORPermuteExtract(state, data, 17, 0, 0): "
      "%d\n",
      measurement);
  printf("Cycles per byte for rate 1088: %f\n\n", measurement / 136.0);

  measurement = measureKeccakF1600_StateXORPermuteExtract_21_0(calibration);
  printf(
      "Cycles for KeccakF1600_StateXORPermuteExtract(state, data, 21, 0, 0): "
      "%d\n",
      measurement);
  printf("Cycles per byte for rate 1344: %f\n\n", measurement / 168.0);

  measurement = measureKeccakF1600_StateXORPermuteExtract_0_16(calibration);
  printf(
      "Cycles for KeccakF1600_StateXORPermuteExtract(state, 0, 0, data, 16): "
      "%d\n",
      measurement);
  printf("Cycles per byte for rate 1024: %f\n\n", measurement / 128.0);

  measurement = measureKeccakF1600_StateXORPermuteExtract_0_17(calibration);
  printf(
      "Cycles for KeccakF1600_StateXORPermuteExtract(state, 0, 0, data, 17): "
      "%d\n",
      measurement);
  printf("Cycles per byte for rate 1088: %f\n\n", measurement / 136.0);

  measurement = measureKeccakF1600_StateXORPermuteExtract_0_21(calibration);
  printf(
      "Cycles for KeccakF1600_StateXORPermuteExtract(state, 0, 0, data, 21): "
      "%d\n",
      measurement);
  printf("Cycles per byte for rate 1344: %f\n\n", measurement / 168.0);

  measurement = measureKeccakAbsorb1000blocks(calibration);
  printf(
      "Cycles for Keccak_SpongeInitialize, Absorb (1000 blocks) and "
      "AbsorbLastFewBits: %d\n\n",
      measurement);

  measurement = measureKeccakSqueeze1000blocks(calibration);
  printf("Cycles for Keccak_SpongeInitialize and Squeeze (1000 blocks): %d\n\n",
         measurement);

  measurement = measureKeccakDuplexing1000blocks(calibration);
  printf(
      "Cycles for Keccak_DuplexInitialize and Duplexing (1000 blocks): %d\n\n",
      measurement);
}
