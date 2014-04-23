/*
The Keccak sponge function, designed by Guido Bertoni, Joan Daemen,
Michaël Peeters and Gilles Van Assche. For more information, feedback or
questions, please refer to our website: http://keccak.noekeon.org/
*/
#include "Tests/timing.h"

#include <stdio.h>
#include <stdint.h>

/************** Timing routine (for performance measurements) ***********/
/* By Doug Whiting */
/* unfortunately, this is generally assembly code and not very portable */
#if defined(_M_IX86) || defined(__i386) || defined(_i386) || \
    defined(__i386__) || defined(i386) || defined(_X86_) ||  \
    defined(__x86_64__) || defined(_M_X64) || defined(__x86_64)
#define _Is_X86_ 1
#endif

#if defined(_Is_X86_) && (!defined(__STRICT_ANSI__)) &&                  \
    (defined(__GNUC__) || !defined(__STDC__)) &&                         \
    (defined(__BORLANDC__) || defined(_MSC_VER) || defined(__MINGW_H) || \
     defined(__GNUC__))
#define HI_RES_CLK_OK 1 /* it's ok to use RDTSC opcode */

#if defined(_MSC_VER)  // && defined(_M_X64)
#include <intrin.h>
#pragma intrinsic(__rdtsc) /* use MSVC rdtsc call where defined */
#endif

#endif

/* return the current value of time stamp counter */
uint32_t HiResTime(void) {
#if defined(HI_RES_CLK_OK)
  uint32_t x[2];
#if defined(__BORLANDC__)
#define COMPILER_ID "BCC"
  __emit__(0x0F, 0x31); /* RDTSC instruction */
  _asm{mov x[0], eax};
#elif defined(_MSC_VER)
#define COMPILER_ID "MSC"
#if defined(_MSC_VER)  // && defined(_M_X64)
  x[0] = (uint32_t)__rdtsc();
#else
  _asm{_emit 0fh};
  _asm{_emit 031h};
  _asm{mov x[0], eax};
#endif
#elif defined(__MINGW_H) || defined(__GNUC__)
#define COMPILER_ID "GCC"
  asm volatile("rdtsc" : "=a"(x[0]), "=d"(x[1]));
#else
#error "HI_RES_CLK_OK -- but no assembler code for this platform (?)"
#endif
  return x[0];
#else
/* avoid annoying MSVC 9.0 compiler warning #4720 in ANSI mode! */
#if (!defined(_MSC_VER)) || (!defined(__STDC__)) || (_MSC_VER < 1300)
  #warning-- "No support for RDTSC; simulating by returning zero."
  return 0;
  //FatalError("No support for RDTSC on this CPU platform\n");
#endif
  return 0;
#endif /* defined(HI_RES_CLK_OK) */
}

uint32_t calibrate(void) {
  uint32_t dtMin = 0xFFFFFFFF; /* big number to start */
  uint32_t t0, t1, i;

  for (i = 0; i < TIMER_SAMPLE_CNT;
       i++) /* calibrate the overhead for measuring time */
  {
    t0 = HiResTime();
    t1 = HiResTime();
    if (dtMin > t1 - t0) /* keep only the minimum time */
      dtMin = t1 - t0;
  }
  return dtMin;
}
