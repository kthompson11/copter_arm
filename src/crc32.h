#ifndef CRC32_H
#define CRC32_H

#include <stdint.h>

/**
 * Calculate the CRC32 checksum for an input buffer.
 * buf: input data buffer
 * len: len of input buffer
 * 
 * Return: value of CRC32 checksum
 */
uint32_t crc32(const uint8_t *buf, int len);

/**
 * Initializes CRC32 lookup tables
 */
void crc32_init(void);

#endif /* CRC32_H */