/** ****************************************************************************
 * @file eeprom.c
 ******************************************************************************/

#include <stdint.h>
#include <string.h> // memcpy

//**************************
#include "crc16.h"
#include "eepromAPI.h"
#include "eeprom.h"
#include "cy_project.h"
   
/* Declare a FLASH config struct which initialized by FlashInit, and will be used by all flash operations */

/**********************************************
* @brief
*
* @return BOOL
***********************************************/
uint8_t  programData_flash[SIZE_4096_BIT_IN_BYTE] = {0};
uint32_t CY_FLASH_Program(uint32_t SectorAddr,uint32_t SectorSizeInByte,uint8_t* src)
{
    uint32_t* p_TestFlsTop = (uint32_t*)SectorAddr;
    uint32_t SectorSizeInWord = SectorSizeInByte / 4ul;

    for(uint32_t i_addr = SectorAddr, i_addrOffset = 0; i_addr < SectorAddr + SectorSizeInByte; i_addr+=4, i_addrOffset+=4)
    {
        // Flash 
        //Cy_FlashWriteWork(i_addr, (uint32_t*)&programData_flash[0], CY_FLASH_DRIVER_BLOCKING);        
        //Cy_FlashWriteWork(i_addr, 0x50A01030, CY_FLASH_DRIVER_BLOCKING);
        Cy_FlashWriteWork(i_addr, (uint32_t*)&src[i_addrOffset], CY_FLASH_DRIVER_BLOCKING);
    }
    return 1;
}


uint32_t CY_FLASH_Program_Code(uint32_t SectorAddr,uint32_t SectorSizeInByte,const uint8_t* src)
{
    uint8_t write_buff[SIZE_256_BIT_IN_BYTE];
    // uint32_t* p_TestFlsTop = (uint32_t*)SectorAddr;
    // uint32_t SectorSizeInWord = SectorSizeInByte / 4ul;
    // for(uint32_t i_addr = SectorAddr,i_addrOffset = 0; i_addr < SectorAddr + SectorSizeInByte; i_addr+=SIZE_128_BIT_IN_BYTE, i_addrOffset+=SIZE_128_BIT_IN_BYTE)
    // {
    //     // Flash 
    //     Cy_FlashWriteCode(i_addr, (uint32_t*)&src[i_addrOffset], CY_FLASH_PROGRAMROW_DATA_SIZE_128BIT,CY_FLASH_DRIVER_BLOCKING);
    // }
#if 0  
    for(uint32_t i_addr = SectorAddr; i_addr < SectorAddr + SectorSizeInByte; i_addr+=SIZE_1024_BIT_IN_BYTE)
    { 
        // Flash 
        Cy_FlashWriteCode(i_addr, (uint32_t*)programData, CY_FLASH_PROGRAMROW_DATA_SIZE_256BIT,CY_FLASH_DRIVER_BLOCKING);
    }  
#endif
#if 1 

    for(uint32_t i_addr = SectorAddr,i_addrOffset = 0; i_addr < SectorAddr + SectorSizeInByte; i_addr+=SIZE_256_BIT_IN_BYTE, i_addrOffset+=SIZE_256_BIT_IN_BYTE)
    {
        memcpy(write_buff,src + i_addrOffset,SIZE_256_BIT_IN_BYTE);
        Cy_FlashWriteCode(i_addr, (uint32_t*)write_buff, CY_FLASH_PROGRAMROW_DATA_SIZE_256BIT,CY_FLASH_DRIVER_BLOCKING);     
    }
    return TRUE;
#endif
}

void Cy_Flash_Sectors_Erase(uint32_t start_sector_addr,uint32_t size,uint8_t type)
{
    uint32_t sector_num = 0;
    uint32_t sector_size = 0;
    int sector_index = 0;
    if(type == LARGE_SECTOR)
    {
        sector_size = CY_DF_LS_BLOCK_SECTOR_SIZE;
    }
    else if(type == SHORT_SECTOR)
    {
        sector_size = CY_DF_SS_BLOCK_SECTOR_SIZE;
    }
	else
	{	
		/**default**/
        sector_size = CY_DF_LS_BLOCK_SECTOR_SIZE;
	}
    sector_num = size / sector_size + 1;

    for(sector_index = 0;sector_index < sector_num;sector_index++)
    {
        Cy_FlashSectorErase(start_sector_addr + sector_index*sector_size, CY_FLASH_DRIVER_BLOCKING);
        Cy_SysTick_DelayInUs(6000);
    }
}

bool Cy_WorkFlash_Blanks_Check(uint32_t start_sector_addr,uint32_t size,uint8_t type)
{
    uint32_t sector_num = 0;
    uint32_t sector_size = 0;
    int status = 0;
    int sector_index = 0;
    if(type == LARGE_SECTOR)
    {
        sector_size = CY_DF_LS_BLOCK_SECTOR_SIZE;
    }
    else if(type == SHORT_SECTOR)
    {
        sector_size = CY_DF_SS_BLOCK_SECTOR_SIZE;
    }
    sector_num = size / sector_size + 0.5;
    for(sector_index = 0;sector_index < sector_num;sector_index++)
    {
        status = Cy_WorkFlashBlankCheck(start_sector_addr + sector_index*sector_size, CY_FLASH_DRIVER_BLOCKING);
        if(status == FALSE)
        {
            return FALSE;
        }
        Cy_SysTick_DelayInUs(200);
    }
    return TRUE;
}


bool Cy_WorkFlash_Write_Check(uint32_t start_sector_addr,uint32_t size,uint8_t * expected_data)
{
    uint32_t* pProgramData = (uint32_t*)start_sector_addr;
    uint32_t SectorSizeInWord = size / 4ul;
    int status = 1;
    for(uint32_t i_wordId = 0; i_wordId < SectorSizeInWord; i_wordId++)
    {
        if(expected_data[i_wordId] != pProgramData[i_wordId])
        {
            status = 0;
            break; 
        }
    }
    return status;
}

/*******************************************
 * @brief Set the Jump Flag object
 * 
 * @param dat ==
 * @return int 
********************************************/
static int32_t    setJumpFlag(uint32_t const dat)
{
    uint32_t        status;
    uint32_t        failAddr;
    int32_t         retry_cnt = 0;
	uint32_t 	    read_dat  = 0U;
    uint32_t        value;
    //__disable_irq();

    uint32_t size = CY_DF_LS_BLOCK_SECTOR_SIZE;
    value = dat;

    Cy_Flash_Sectors_Erase(APP_UPDATE_FLAG_ADDR, size,LARGE_SECTOR);
    status = Cy_WorkFlash_Blanks_Check(APP_UPDATE_FLAG_ADDR,size,LARGE_SECTOR);       
    if (status != TRUE) {
        //__enable_irq();
        return FALSE;
    }
    while(retry_cnt < 3){
        retry_cnt++;
        status = CY_FLASH_Program(APP_UPDATE_FLAG_ADDR,4U,(uint8_t*)&value);
        status = Cy_WorkFlash_Write_Check(APP_UPDATE_FLAG_ADDR, 4U,(uint8_t*)&value);        
        if(status !=TRUE){
            break;
        }
    }

    //__enable_irq();

    if(status != TRUE){
		return BAD;
	}

	read_dat = *((uint32_t *)APP_UPDATE_FLAG_ADDR);

	if(dat != read_dat){
		return BAD;
    }

    return  GOOD  ;
}

/**********************************************
* @brief
*
* @return BOOL
***********************************************/
int32_t EEPROM_PrepareToEnterBootloader(void)
{
    return setJumpFlag(ENTER_BOOTLOADER_FLAG);
}

int32_t EEPROM_PrepareToEnteApp(void)
{
    return setJumpFlag(ENTER_APP);
}

// the structure should have next format
// ============== Sensor chip 0 =========
// Accel0 a0 b0, b1, b2
// Accel0 a1 b0, b1, b2
// Accel0 a2 b0, b1, b2
// Accel0 a3 b0, b1, b2
// Accel0 a4 b0, b1, b2
// Accel0 a5 b0, b1, b2
// **************************************
// Accel1 a0 b0, b1, b2
// Accel1 a1 b0, b1, b2
// Accel1 a2 b0, b1, b2
// Accel1 a3 b0, b1, b2
// Accel1 a4 b0, b1, b2
// Accel1 a5 b0, b1, b2
// **************************************
// Accel2 a0 b0, b1, b2
// Accel2 a1 b0, b1, b2
// Accel2 a2 b0, b1, b2
// Accel2 a3 b0, b1, b2
// Accel2 a4 b0, b1, b2
// Accel2 a5 b0, b1, b2
// **************************************
// Rate0  a0 b0, b1, b2
// Rate0  a1 b0, b1, b2
// Rate0  a2 b0, b1, b2
// Rate0  a3 b0, b1, b2
// Rate0  a4 b0, b1, b2
// Rate0  a5 b0, b1, b2
// **************************************
// Rate1  a0 b0, b1, b2
// Rate1  a1 b0, b1, b2
// Rate1  a2 b0, b1, b2
// Rate1  a3 b0, b1, b2
// Rate1  a4 b0, b1, b2
// Rate1  a5 b0, b1, b2
// **************************************
// Rate2  a0 b0, b1, b2
// Rate2  a1 b0, b1, b2
// Rate2  a2 b0, b1, b2
// Rate2  a3 b0, b1, b2
// Rate2  a4 b0, b1, b2
// Rate2  a5 b0, b1, b2
// ============== Sensor chip 1 =========
//.......................................
extern uint32_t get_bin_size();
BOOL EEPROM_WriteApp(uint32_t offset, uint16_t num, uint8_t source[],uint8_t core_to_upgrade)
{
    uint32_t    status = TRUE;
    uint32_t    pageOffset = 0;
	static uint8_t write_buff_to_check[1024];
	
    if(offset < 60000)
    {
        status = FALSE;
    }
    if (!num || ((num % 16U) != 0U)) {
        return FALSE;
    }

    if (((uint32_t)(offset + num) > APP_CODE_MAX_SIZE) || ((offset % 8U) != 0U)){   //only offset even to 8 bytes
        return FALSE;
    }
    if(core_to_upgrade == CORE_CM7_0)
    {
        pageOffset = offset + CM7_0_CODE_ADDR;
    }
    else if(core_to_upgrade == CORE_CM7_1)
    {
        pageOffset = offset + CM7_1_CODE_ADDR;
    }
	else
	{
		return FALSE;
	}
	
    
    //BIT_NotifyEepromWriteEvent();

    if (offset == 0U) {
        uint32_t size = get_bin_size();
        // Cy_Flash_Sectors_Erase(pageOffset, APP_CODE_MAX_SIZE,LARGE_SECTOR);
        Cy_Flash_Sectors_Erase(pageOffset, size, LARGE_SECTOR);  
    }

    status = CY_FLASH_Program_Code(pageOffset,num,source);
    // Verify 
    if(status != TRUE){
        return FALSE;
    }

    //uint8_t flash_buff[num];
    memcpy(write_buff_to_check,source,num);
    uint32_t* pProgramData = (uint32_t*)write_buff_to_check;
    uint32_t* p_TestFlsTop = (uint32_t*)pageOffset;
    uint32_t SectorSizeInWord = num / 4ul;
#if 1
    for(uint32_t i_wordId = 0; i_wordId < SectorSizeInWord; i_wordId++)
    {
        if(p_TestFlsTop[i_wordId] != pProgramData[i_wordId])
        {
          return FALSE;          
        }
    }
#endif
    //__enable_irq();
    return TRUE;
} /* end writeEEPROMCal() */

void HW_EnforceBootMode(void)
{
    static uint32_t const serialBootSig[] = {
        0x02698721U,
        0x12696734U,
        0x29827567U,
        0x55966637U
    };

    uint32_t *dataptr = (uint32_t *)BOOT_SIGNATURE_ADDR;
    for(int32_t i = 0; i < 4; i++){
        *dataptr = serialBootSig[i];
         dataptr++;
    }
    EEPROM_PrepareToEnterBootloader();
}

