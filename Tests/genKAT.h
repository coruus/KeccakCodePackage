#ifndef GENKAT_H
#define GENKAT_H
#include "Modes/KeccakHash.h"
#include <stdint.h>

typedef enum {
  KAT_SUCCESS = 0,
  KAT_FILE_OPEN_ERROR = 1,
  KAT_HEADER_ERROR = 2,
  KAT_DATA_ERROR = 3,
  KAT_HASH_ERROR = 4
} STATUS_CODES;

STATUS_CODES genKAT_main(void);
STATUS_CODES genShortMsgHash(uint32_t rate,
                             uint32_t capacity,
                             uint8_t delimitedSuffix,
                             uint32_t hashbitlen,
                             uint32_t squeezedOutputLength,
                             const char* fileName,
                             const char* description);
int FindMarker(FILE* infile, const char* marker);
int ReadHex(FILE* infile, BitSequence* A, int Length, char* str);
void fprintBstr(FILE* fp, char* S, BitSequence* A, int L);
void convertShortMsgToPureLSB(void);
#endif
