#include "maintain_ins.h"
#include "eeprom.h"
#include <string.h>
#include "crc16.h"
#include "uart.h"
#include <stdlib.h>
#include "gnss.h"
#include "ins_interface_API.h"

uint8_t maintain_ins_valid = 0;
maintain_ins_status_t maintain_ins_status;
// extern ins_solution_t* g_ptr_ins_sol;
extern OutputMsg ins_fusion_solution;
extern uint8_t last_ins_save_buff[INS_SAVE_BUF_SIZE];
extern uint8_t ins_save_buff[INS_SAVE_BUF_SIZE];
void maintain_ins_clear_inflash(void)
{
    uint32_t size = sizeof(maintain_ins_status_t);
    uint32_t const start = (uint32_t)EEPROM_INS_STATUS_ADDR;
    uint32_t status;

    if (maintain_ins_valid == 1) {
        Cy_Flash_Sectors_Erase(start, CY_DF_LS_BLOCK_SECTOR_SIZE, LARGE_SECTOR);
        status = Cy_WorkFlash_Blanks_Check(start, CY_DF_LS_BLOCK_SECTOR_SIZE, LARGE_SECTOR);
        if (status != 1) {
            Cy_Flash_Sectors_Erase(start, CY_DF_LS_BLOCK_SECTOR_SIZE, LARGE_SECTOR);
        }
        memset((uint8_t *)&maintain_ins_status, 0, size);
        status = CY_FLASH_Program(start, size, (uint8_t*)&maintain_ins_status);
        status = Cy_WorkFlash_Write_Check(start, size, (uint8_t*)&maintain_ins_status);

        if(status == 0){
            CY_FLASH_Program(start, size, (uint8_t*)&maintain_ins_status);
        }
    }
}

void maintain_ins_read_inflash(void)
{
    uint32_t size = sizeof(maintain_ins_status_t);
    uint32_t const start = (uint32_t)EEPROM_INS_STATUS_ADDR;
    uint32_t crc;
    
    memcpy((uint8_t*)&maintain_ins_status, (uint8_t*)start, size);

    crc = CalculateCRC((uint8_t*)start, size - 4);
    if (crc == maintain_ins_status.crc) {
        maintain_ins_valid = 1;
    } else {
        maintain_ins_valid = 0;
        memset((uint8_t *)&maintain_ins_status, 0, size);
    }
}

void maintain_ins_write_inflash(void)
{
    uint32_t size = sizeof(maintain_ins_status_t);
    uint32_t const start = (uint32_t)EEPROM_INS_STATUS_ADDR;
    uint32_t status;

    if (gnss_valid == 1) {
        // maintain_ins_status.latitude  = g_ptr_ins_sol->latitude;
        // maintain_ins_status.longitude = g_ptr_ins_sol->longitude;
        // maintain_ins_status.height    = g_ptr_ins_sol->height;
        
        maintain_ins_status.latitude  = ins_fusion_solution.ins_solution.latitude;
        maintain_ins_status.longitude = ins_fusion_solution.ins_solution.longitude;
        maintain_ins_status.height    = ins_fusion_solution.ins_solution.height;

        maintain_ins_status.crc = CalculateCRC((uint8_t*)&maintain_ins_status, size - 4);

        Cy_Flash_Sectors_Erase(start, CY_DF_LS_BLOCK_SECTOR_SIZE, LARGE_SECTOR);
        status = Cy_WorkFlash_Blanks_Check(start, CY_DF_LS_BLOCK_SECTOR_SIZE, LARGE_SECTOR);
        if (status != 1) {
            Cy_Flash_Sectors_Erase(start, CY_DF_LS_BLOCK_SECTOR_SIZE, LARGE_SECTOR);
        }

        status = CY_FLASH_Program(start, size, (uint8_t*)&maintain_ins_status);
        status = Cy_WorkFlash_Write_Check(start, size, (uint8_t*)&maintain_ins_status);
        if (status == 0){
            CY_FLASH_Program(start, size, (uint8_t*)&maintain_ins_status);
        }
    }
}


void maintain_ins_save_buf_inflash(void)
{
    uint32_t size = INS_SAVE_BUF_SIZE;
    uint32_t const start = (uint32_t)EEPROM_INS_SAVE_BUF_ADDR;
    uint32_t status;

    Cy_Flash_Sectors_Erase(start, CY_DF_LS_BLOCK_SECTOR_SIZE, LARGE_SECTOR);
    status = Cy_WorkFlash_Blanks_Check(start, CY_DF_LS_BLOCK_SECTOR_SIZE, LARGE_SECTOR);
    if (status != 1) {
        Cy_Flash_Sectors_Erase(start, CY_DF_LS_BLOCK_SECTOR_SIZE, LARGE_SECTOR);
    }

    status = CY_FLASH_Program(start, size, (uint8_t*)ins_save_buff);
    status = Cy_WorkFlash_Write_Check(start, size, (uint8_t*)ins_save_buff);
    if (status == 0){
        CY_FLASH_Program(start, size, (uint8_t*)ins_save_buff);
    }
}

void maintain_ins_read_ins_save_buf(void)
{
    uint32_t size = INS_SAVE_BUF_SIZE;
    uint32_t const start = (uint32_t)EEPROM_INS_SAVE_BUF_ADDR;
    
    memcpy((uint8_t*)last_ins_save_buff, (uint8_t*)start, size);
}



void ins_save_buf_flash_erase(void)
{
    uint32_t const start = (uint32_t)EEPROM_INS_SAVE_BUF_ADDR;
    uint32_t status;

    Cy_Flash_Sectors_Erase(start, CY_DF_LS_BLOCK_SECTOR_SIZE, LARGE_SECTOR);
    status = Cy_WorkFlash_Blanks_Check(start, CY_DF_LS_BLOCK_SECTOR_SIZE, LARGE_SECTOR);
    if (status != 1) {
        Cy_Flash_Sectors_Erase(start, CY_DF_LS_BLOCK_SECTOR_SIZE, LARGE_SECTOR);
    }

}