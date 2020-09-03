/* 
   Software implementation of LSB-first CRC32 using a lookup table based on pseudocode at 
   https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks#Multi-bit_computation
 */

#include "crc32.h"

#include <stdio.h>

static uint32_t lookup_table[256];
static const uint32_t CRC_POLY = 0xEDB88320;

uint32_t crc32(const uint8_t *buf, int len)
{
    uint32_t rem = 0;
    rem = ~rem;
    for (int i = 0; i < len; ++i) {
        rem = rem ^ (uint32_t)buf[i];

        for (int j = 1; j < 9; ++j) {
            if (rem & 1) {
                rem = (rem >> 1) ^ CRC_POLY;
            } else {
                rem = rem >> 1;
            }
        }
    }

    rem = ~rem;
    return rem;
}

/* TODO: get CRC32 working with a lookup table */
#ifdef COMMENTED_OUT
uint32_t crc32(const uint8_t *buf, int len)
{
    uint32_t rem = 0;
    rem = ~rem;
    for (int i = 0; i < len; ++i) {
        uint32_t lookup_index = buf[i] ^ (rem & 0xFF);
        rem = (rem >> 8) ^ lookup_table[lookup_index];
    }

    rem = ~rem;
    return rem;
}

void crc32_init(void)
{
    lookup_table[0] = 0;
    uint32_t rem = 1;
    for (int i = 128; i > 0; i = i / 2) {
        if (rem & 1) {
            rem = (rem / 2) ^ CRC_POLY;
        } else {
            rem = rem / 2;
        }

        for (int j = 0; j < 256; j += 2 * i) {
            printf("i = %d, j = %d, i + j = %d\n", i, j, (i + j));
            uint32_t res = rem ^ lookup_table[j];
            printf("%u\n", res);
            uint32_t idx = i + j;
            printf("idx = %d\n", idx);
            lookup_table[idx] = res; //755167117ul;
            printf("after\n");
        }
    }
}
#endif
