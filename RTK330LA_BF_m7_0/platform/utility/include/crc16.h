/** ***************************************************************************
 * @file crc16.h
 *
 */

#ifndef CRC16_H
#define CRC16_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief
 *
 * @param data
 * @param len
 * @return uint16_t
 */
uint16_t CRC16(uint8_t data[], int32_t const len);

/**
 * @brief
 *
 * @param buf
 * @param length
 * @return uint16_t
 */
uint16_t CalculateCRC(uint8_t *buf, uint32_t const length);

#ifdef __cplusplus
}
#endif

#endif
