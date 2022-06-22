/** ***************************************************************************
 * @file flash.h
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 *****************************************************************************/
#ifndef EEPROM_H
#define EEPROM_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include "syslib/cy_syslib.h"

#define CY_DF_LS_BLOCK_SECTOR_SIZE (2048u)
#define CY_DF_SS_BLOCK_SECTOR_SIZE (128u)
  
extern char *  __cm7_0_vector_base_linker_symbol;
extern char *  __cm7_1_vector_base_linker_symbol;
#define CM7_0_CODE_ADDR      ((uint32_t) &__cm7_0_vector_base_linker_symbol)
#define CM7_1_CODE_ADDR      ((uint32_t) &__cm7_1_vector_base_linker_symbol)
enum {
    EEPROM_CAL_ADDR1 = 0x14000000UL,
    EEPROM_CAL_ADDR2 = 0x14001000UL,
    EEPROM_CAL_ADDR3 = 0x14002000UL,
    EEPROM_CAL_ADDR4 = 0x14003000UL,
    EEPROM_SFTAB_ADDR = 0x14004000UL,
    EEPROM_FACTORY_CONFIG_ADDR = 0x14008000UL,
    EEPROM_USER_CONFIG_ADDR = 0x14009000UL,
    APP_UPDATE_FLAG_ADDR = 0x1400c000UL,
    APP_START_SIGNATURE_ADDR = 0x1400c010UL,
    CAL_PARTITION_SIZE = 4096,
    CAL_CRC_OFFSET = 4092,
    APP_USER_CONFIG_SIZE = 0x1000,
    APP_START_ADDR = 0x10000,
    APP_LAST_ADDR = 0x7ffff,
    APP_MAX_SIZE = 0x80000,
    ENTER_APP = 0x55AA1234,
    ENTER_BOOTLOADER_FLAG = 0x50A01030,
    BOOT_SIGNATURE_ADDR = 0x2806b000,
    BOOT_ROOT_CAUSE_ADDR = 0x2806b010,
    ROOT_CAUSE_PATTERN = 0xAA559600,
    APP_CODE_MAX_SIZE = 0x96000
};

enum {
    APP_SIGNATURE_OFFSET3 = 0x01A0,
    APP_SIGNATURE_OFFSET4 = 0x01A4,
    APP_SIGNATURE_OFFSET5 = 0x01A8,
    APP_SIGNATURE_OFFSET6 = 0x01AC,
};

enum {
    APP_SIGNATURE3 = 0x01234567,
    APP_SIGNATURE4 = 0x89ABCDEF,
    APP_SIGNATURE5 = 0xAA55AA55,
    APP_SIGNATURE6 = 0x55AA55AA,
};

typedef struct {
        uint8_t  memory[2048];
} uConfigInEeprom;

enum {
    BAD   = 1,
    GOOD  = 0,
};

enum {
    LARGE_SECTOR   = 1,
    SHORT_SECTOR  = 0,
};



#pragma pack(1)
#define N_TABLES_A       15
#define N_MISALIGN       18
#define N_RESERVED_CAL   6
#define N_ROWS_TABLE_A   360
#define NUM_SENSOR_CHIPS 3
typedef struct {
    int32_t counts;
    float32_t value;
} cal_table_entry_t;

typedef struct {
    uint32_t serialNumber;
    char versionString[128];

    /// index of first element for each sensor (including 7th sensor)
    uint16_t calibrationTableIndexA[N_TABLES_A + 1]; // 15 + 1
    /// table order: (Temp Tables: xAccel, yAccel, zAccel, xRate, yRate, zRate),
    /// Inertial Tables: (same order), mag tables (x, y, z)
    cal_table_entry_t calibrationTablesA[N_ROWS_TABLE_A]; // [360]
    float32_t misalign[N_MISALIGN]; // [18]

    uint16_t reserved[N_RESERVED_CAL]; // [6]
    uint16_t productConfiguration;

    /// added from the 440 - NAV-view may not program them
    float32_t AccelSensorRange;
    float32_t GyroSensorRange;
    float32_t MagSensorRange;

    uint16_t RollIncidenceLimit;
    uint16_t PitchIncidenceLimit;

    uint16_t HardIronLimit;
    uint16_t SoftIronLimit;
} CalibrationStruct;
#pragma pack()


static uint32_t const UNLOCK_KEY_1  = 0x19623392U;
static uint32_t const UNLOCK_KEY_2  = 0x85422764U;

#define SIZE_4096_BIT_IN_BYTE (0x200ul)
#define SIZE_4096_BIT_IN_WORD (0x200ul / 4ul)
#define SIZE_1024_BIT_IN_BYTE (0x80ul)
#define SIZE_1024_BIT_IN_WORD (0x80ul / 4ul)

#define SIZE_256_BIT_IN_BYTE (0x20ul)
#define SIZE_256_BIT_IN_WORD (0x20ul / 4ul)


#define PROGRAM_DATA_SIZE_IN_BYTE (sizeof(programData_flash)/sizeof(uint8_t))
#define PROGRAM_DATA_SIZE_IN_WORD (sizeof(programData_flash)/sizeof(uint32_t))

#ifdef __cplusplus
}
#endif

#endif /* S_EEPROM_H */
