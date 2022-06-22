/** ***************************************************************************
 * @file eeprom_api.h 
 * @Author
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 *****************************************************************************/
#ifndef EEPROM_API_H
#define EEPROM_API_H

#include <stdint.h> 
#include "syslib/cy_syslib.h"

/**********************************************
* @brief 
* 
* @return BOOL 
***********************************************/
extern int32_t EEPROM_PrepareToEnterBootloader(void);

void HW_EnforceBootMode(void);

int32_t EEPROM_PrepareToEnterBootloader(void);

int32_t EEPROM_PrepareToEnteApp(void);

BOOL EEPROM_WriteApp(uint32_t offset, uint16_t num, uint8_t source[],uint8_t core_to_upgrade);

#endif /* S_EEPROM_H */ 


