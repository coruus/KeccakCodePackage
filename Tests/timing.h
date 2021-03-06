/*
The Keccak sponge function, designed by Guido Bertoni, Joan Daemen,
Michaël Peeters and Gilles Van Assche. For more information, feedback or
questions, please refer to our website: http://keccak.noekeon.org/
*/

#ifndef _timing_h_
#define _timing_h_
#include <stdint.h>

#define TIMER_SAMPLE_CNT (100)

uint32_t HiResTime(void);
uint32_t calibrate(void);

#endif
