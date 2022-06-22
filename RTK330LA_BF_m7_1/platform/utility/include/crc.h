/** 
 * @file crc.h
 *
 */

#ifndef CRC_H
#define CRC_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t CrcCcittType;
typedef uint32_t Crc32Type;

static uint32_t const CRC_32_INITIAL_SEED = 0xFFFFFFFFU;

enum {
    CRC_32_LENGTH = 4U,
    CRC_CCITT_LENGTH = 2U
};

/**
 * @brief
 *
 * @param data
 * @param length
 * @param seed
 * @return Crc32Type
 */
extern Crc32Type Crc32(uint8_t const data [], uint16_t const length, Crc32Type const seed);

/**
 * @brief
 *
 * @param v
 * @param seed
 * @return uint16_t
 */
uint16_t initCRC_16bit(uint16_t  const v, uint16_t const seed);

#ifdef __cplusplus
}
#endif

#endif
