#ifndef GENKAT_H
#define GENKAT_H
#include <stdint.h>

int genKAT_main(void);
STATUS_CODES genShortMsgHash(unsigned int rate,
                             unsigned int capacity,
                             unsigned char delimitedSuffix,
                             unsigned int hashbitlen,
                             unsigned int squeezedOutputLength,
                             const char* fileName,
                             const char* description);
int FindMarker(FILE* infile, const char* marker);
int ReadHex(FILE* infile, BitSequence* A, int Length, char* str);
void fprintBstr(FILE* fp, char* S, BitSequence* A, int L);
void convertShortMsgToPureLSB(void);
#endif
