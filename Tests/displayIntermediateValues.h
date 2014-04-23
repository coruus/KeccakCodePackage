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

#ifndef _displayIntermediateValues_h_
#define _displayIntermediateValues_h_

#include <stdio.h>
#include <stdint.h>

void displaySetIntermediateValueFile(FILE* f);
void displaySetLevel(int level);
void displayBytes(int level,
                  const int8_t* text,
                  const uint8_t* bytes,
                  uint32_t size);
void displayBits(int level,
                 const int8_t* text,
                 const uint8_t* data,
                 uint32_t size,
                 int MSBfirst);
void displayStateAsBytes(int level, const int8_t* text, const uint8_t* state);
void displayStateAs32bitWords(int level,
                              const int8_t* text,
                              const uint32_t* state);
void displayStateAs64bitWords(int level,
                              const int8_t* text,
                              const uint64_t* state);
void displayRoundNumber(int level, uint32_t i);
void displayText(int level, const int8_t* text);

#endif
