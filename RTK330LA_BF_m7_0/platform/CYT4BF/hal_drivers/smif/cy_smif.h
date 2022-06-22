/***************************************************************************//**
* \file cy_smif.h
* \version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
*
* Provides an API declaration of the Cypress SMIF driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_smif Serial Memory Interface (SMIF)
* \{
* The SPI-based communication interface for external memory devices.
*
* SMIF: Serial Memory Interface: This IP block implements an SPI-based
* communication interface for interfacing external memory devices to TVII. The SMIF
* supports Octal-SPI, Dual Quad-SPI, Quad-SPI, DSPI, and SPI.
*
* Features
*   - Standard SPI Master interface
*   - Supports Single/Dual/Quad/Octal SPI Memories
*   - Supports Dual-Quad SPI mode
*   - Design-time configurable support for multiple (up to 4) external serial
*   memory devices
*   - eXecute-In-Place (XIP) operation mode for both read and write accesses
*   with 4KB XIP read cache and on-the-fly encryption and decryption
*   - Supports external serial memory initialization via Serial Flash
*   Discoverable Parameters (SFDP) standard
*   - Support for SPI clock frequencies up to 80 MHz
*
*
* The primary usage model for the SMIF is that of an external memory interface.
* The SMIF is capable of interfacing with different types of memory, up to four
* types.
*
* \b SMIF driver is divided in next layers
*   - cy_smif.h API
*   - cy_smif_memslot.h API
*   - SMIF configuration structures
*
* The SMIF API is divided into the low-level functions and memory-slot functions. Use
* the low level API for the SMIF block initialization and for implementing a generic 
* SPI communication interface using the SMIF block.
*
* The memory slot API has functions to implement the basic memory operations such as 
* program, read, erase etc. These functions are implemented using the memory 
* parameters in the memory device configuration data structure. The memory-slot
* initialization API initializes all the memory slots based on the settings in the
* array.
*
* \image html smif_1_0_p01_layers.png
*
* SMIF Configuration Tool is a stand-alone application, which is a part of PDL
* and could be found in \<PDL_DIR\>/tools/\<OS_DIR\>/SMIFConfigurationTool
* (e.g. for PDL 3.0.0 and Windows OS PDL/3.0.0/tools/win/SMIFConfigurationTool). Tool
* generates *.c and *.h file with configuration structures. These configuration
* structures are input parameters for cy_smif_memslot API level
*
* \warning The driver is not responsible for external memory persistence. You cannot edit
* a buffer during the Read/Write operations. If there is a memory error, the SMIF ip block 
* can require a reset. To determine if this has happened, check the SMIF 
* busy status using Cy_SMIF_IsBusy() and implement a timeout. Reset the SMIF 
* block by toggling CTL.ENABLED. Then reconfigure the SMIF block.
*
* For the Write operation, check that the SMIF driver has completed 
* transferring by calling Cy_SMIF_IsBusy(). Also, check that the memory is 
* available with Cy_SMIF_Memslot_IsBusy() before proceeding. 
*
* Simple example of external flash memory programming using low level SMIF API.
* All steps mentioned in example below are incorporated in
* \ref Cy_SMIF_Memslot_CmdWriteEnable(), \ref Cy_SMIF_Memslot_CmdProgram(), and
* \ref Cy_SMIF_Memslot_IsBusy() of the
* \ref group_smif_mem_slot_functions "memory slot level API".
* \warning Example is simplified, without checks of error conditions.
* \note Flash memories need erase operation before programming. Refer to
* external memory datasheet for specific memory commands.
*
* \snippet smif/smif_sut_01.cydsn/main_cm4.c SMIF_API: Write example
*
* For the Read operation, before accessing the read buffer, check that it is ready
* by calling Cy_SMIF_GetTxFifoStatus().
*
* Simple example of external flash memory read using low level SMIF API. All
* steps mentioned in example below are incorporated in
* \ref Cy_SMIF_Memslot_CmdRead() of the
* \ref group_smif_mem_slot_functions "memory slot level API".
* \warning Example is simplified, without checks of error conditions.
* \note Refer to external memory datasheet for specific memory commands.
*
* \snippet smif/smif_sut_01.cydsn/main_cm4.c SMIF_API: Read example
*
* The user should invalidate the cache by calling Cy_SMIF_CacheInvalidate() when 
* switching from the MMIO mode to XIP mode.
*
* \section group_smif_configuration Configuration Considerations
*
* PDL API has common parameters: base, context, config described in
* \ref page_getting_started_pdl_design "PDL Design" section.
*
* See the documentation for Cy_SMIF_Init() and Cy_SMIF_Memslot_Init() for details
* on the required configuration structures and other initialization topics. 
*
* The normal (MMIO) mode is used for implementing a generic SPI/DSPI/QSPI/Dual
* Quad-SPI/Octal-SPI communication interface using the SMIF block. This
* interface can be used to implement special commands like Program/Erase of
* flash, memory device configuration, sleep mode entry for memory devices or
* other special commands specific to the memory device. The transfer width
* (SPI/DSP/Quad-SPI/Octal-SPI) of a transmission is a parameter set for each
* transmit/receive operation. So these can be changed at run time.
*
* In a typical memory interface with flash memory, the SMIF is used in the
* memory mode when reading from the memory and it switches to the normal mode when
* writing to flash memory.
* A typical memory device has multiple types of commands.
*
* The SMIF interface can be used to transmit different types of commands. Each
* command has different phases: command, dummy cycles, and transmit and receive
* data which require separate APIs.
*
* \subsection group_smif_init SMIF Initialization
* Create interrupt function and allocate memory for SMIF context
* structure
* \snippet smif/smif_sut_01.cydsn/main_cm4.c SMIF_INIT: context and interrupt
* SMIF driver initialization for low level API usage (cysmif.h)
* \snippet smif/smif_sut_01.cydsn/main_cm4.c SMIF_INIT: low level
* Additional steps to initialize SMIF driver for memory slot level API usage
* (cy_smif_memslot.h).
* \snippet smif/smif_sut_01.cydsn/main_cm4.c SMIF_INIT: memslot level
* \note Example does not include initialization of all needed configuration
* structures (\ref cy_stc_smif_mem_device_cfg_t, \ref cy_stc_smif_mem_cmd_t).
* SMIF Configuration tool generates all configuration structures needed for
* memslot level API usage.
*
* \subsection group_smif_xip_init SMIF XIP Initialization
* The eXecute In Place (XIP) is a mode of operation where read or write commands 
* to the memory device are directed through the SMIF without any use of API 
* function calls. In this mode the SMIF block maps the AHB bus-accesses to 
* external memory device addresses to make it behave similar to internal memory. 
* This allows the CPU to execute code directly from external memory. This mode 
* is not limited to code and is suitable also for data read and write accesses. 
* \snippet smif/smif_sut_01.cydsn/main_cm4.c SMIF_INIT: XIP
* \note Example of input parameters initialization is in \ref group_smif_init 
* section.
* \warning Functions that called from external memory should be declared with 
* long call attribute.     
*
* \section group_smif_more_information More Information
*
* More information regarding the Serial Memory Interface can be found in the component 
* datasheet and the Technical Reference Manual (TRM).
* More information regarding the SMIF Configuration Tool are in SMIF
* Configuration Tool User Guide located in \<PDL_DIR\>/tools/\<OS_DIR\>/SMIFConfigurationTool/
* folder
*
* \section group_smif_changelog Changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>1.20</td>
*     <td>Flattened the driver source code organization into the single
*         source directory and the single include directory
*     </td>
*     <td>Simplified the driver library directory structure</td>
*   </tr>
*   <tr>
*     <td>1.10.1</td>
*     <td>Added Low Power Callback section</td>
*     <td>Documentation update and clarification</td>
*   </tr>
*   <tr>
*     <td>1.10</td>
*     <td>Fix write to external memory from CM0+ core. Add checks of API input parameters. 
*         Minor documentation updates</td>
*     <td></td>
*   </tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version</td>
*     <td></td>
*   </tr>
* </table>
*
* \defgroup group_smif_macros Macros
* \{
* \defgroup group_smif_macros_status Status Macros
* \defgroup group_smif_macros_cmd Command Macros
* \defgroup group_smif_macros_flags External Memory Flags
* \defgroup group_smif_macros_sfdp SFDP Macros
* \defgroup group_smif_macros_isr Interrupt Macros
* \}
* \defgroup group_smif_functions Functions
* \{
* \defgroup group_smif_low_level_functions Low Level Functions
* \{
* Basic flow for read/write commands using \ref Cy_SMIF_TransmitCommand
* \ref Cy_SMIF_TransmitData \ref Cy_SMIF_ReceiveData
* \ref Cy_SMIF_SendDummyCycles
*
*  \image html smif_1_0_p03_rw_cmd.png
*
* \}
* \defgroup group_smif_mem_slot_functions Memory Slot Functions
* \defgroup group_smif_functions_syspm_callback  Low Power Callback
* \}
* \defgroup group_smif_data_structures Data Structures
* \{
* \defgroup group_smif_data_structures_memslot SMIF Memory Description Structures
* General hierarchy of memory structures are:
* \image html smif_1_0_p02_memslot_stc.png
* Top structure is \ref cy_stc_smif_block_config_t, which could have links up to
* 4 \ref cy_stc_smif_mem_config_t which describes each connected to the SMIF
* external memory.
* \}
* \defgroup group_smif_enums Enumerated Types
*/

#if !defined(CY_SMIF_H)
#define CY_SMIF_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "syslib/cy_syslib.h"
#include "syspm/cy_syspm.h"
//#include "smif/cy_smif_hb_flash.h"
#include "cy_device_headers.h"

#ifdef CY_IP_MXSMIF



// Devices with a single SMIF instance use a different generated define name (w/o instance suffix), so make it common
#if defined (SMIF_DEVICE_NR)
#define SMIF0_DEVICE_NR          SMIF_DEVICE_NR
#endif
#if defined (SMIF_DELAY_LINES_NR)
#define SMIF0_DELAY_LINES_NR     SMIF_DELAY_LINES_NR
#endif
#if defined (SMIF_DELAY_TAPS_NR)
#define SMIF0_DELAY_TAPS_NR      SMIF_DELAY_TAPS_NR
#endif
#if defined (SMIF_SMIF_XIP_MASK)
#define SMIF0_SMIF_XIP_MASK      SMIF_SMIF_XIP_MASK
#endif
#if defined (SMIF_SMIF_XIP_ADDR)
#define SMIF0_SMIF_XIP_ADDR      SMIF_SMIF_XIP_ADDR
#endif

// Temporal Version Acceptor 
#if defined(_CYIP_SMIF_V2_H_) || (defined(_CYIP_SMIF_V3_H_) && (defined(CYT4DNDBHS) || defined(CYT4DNJBHS))) // Indicating Old SMIF
  #define CY_SMIF_OLD_VERSION
#elif defined (_CYIP_SMIF_V3_H_)  // Indicating New SMIF*/
  #define CY_SMIF_NEW_VERSION
#else
  #error "SMIF IP version not supported by the driver."
#endif

#ifdef CY_SMIF_NEW_VERSION

#define stc_SMIF_TX_CMD_FIFO_WR_field              stc_SMIF_TX_CMD_MMIO_FIFO_WR_field
#define stc_SMIF_TX_CMD_FIFO_WR_field_t            stc_SMIF_TX_CMD_MMIO_FIFO_WR_field_t
#define un_SMIF_TX_CMD_FIFO_WR                     un_SMIF_TX_CMD_MMIO_FIFO_WR
#define un_SMIF_TX_CMD_FIFO_WR_t                   un_SMIF_TX_CMD_MMIO_FIFO_WR_t
#define unTX_CMD_FIFO_WR                           unTX_CMD_MMIO_FIFO_WR
#define SMIF_TX_CMD_FIFO_WR_DATA27_Pos             SMIF_TX_CMD_MMIO_FIFO_WR_DATA27_Pos
#define SMIF_TX_CMD_FIFO_WR_DATA27_Msk             SMIF_TX_CMD_MMIO_FIFO_WR_DATA27_Msk

#define stc_SMIF_TX_DATA_FIFO_CTL_field            stc_SMIF_TX_DATA_MMIO_FIFO_CTL_field
#define stc_SMIF_TX_DATA_FIFO_CTL_field_t          stc_SMIF_TX_DATA_MMIO_FIFO_CTL_field_t
#define un_SMIF_TX_DATA_FIFO_CTL                   un_SMIF_TX_DATA_MMIO_FIFO_CTL
#define un_SMIF_TX_DATA_FIFO_CTL_t                 un_SMIF_TX_DATA_MMIO_FIFO_CTL_t
#define unTX_DATA_FIFO_CTL                         unTX_DATA_MMIO_FIFO_CTL
#define SMIF_TX_DATA_FIFO_CTL_TX_TRIGGER_LEVEL_Pos SMIF_TX_DATA_MMIO_FIFO_CTL_TX_TRIGGER_LEVEL_Pos
#define SMIF_TX_DATA_FIFO_CTL_TX_TRIGGER_LEVEL_Msk SMIF_TX_DATA_MMIO_FIFO_CTL_TX_TRIGGER_LEVEL_Msk

#define stc_SMIF_TX_DATA_FIFO_WR1_field            stc_SMIF_TX_DATA_MMIO_FIFO_WR1_field
#define stc_SMIF_TX_DATA_FIFO_WR1_field_t          stc_SMIF_TX_DATA_MMIO_FIFO_WR1_field_t
#define un_SMIF_TX_DATA_FIFO_WR1                   un_SMIF_TX_DATA_MMIO_FIFO_WR1
#define un_SMIF_TX_DATA_FIFO_WR1_t                 un_SMIF_TX_DATA_MMIO_FIFO_WR1_t
#define unTX_DATA_FIFO_WR1                         unTX_DATA_MMIO_FIFO_WR1
#define SMIF_TX_DATA_FIFO_WR1_DATA0_Pos            SMIF_TX_DATA_MMIO_FIFO_WR1_DATA0_Pos
#define SMIF_TX_DATA_FIFO_WR1_DATA0_Msk            SMIF_TX_DATA_MMIO_FIFO_WR1_DATA0_Msk

#define stc_SMIF_TX_DATA_FIFO_WR2_field            stc_SMIF_TX_DATA_MMIO_FIFO_WR2_field
#define stc_SMIF_TX_DATA_FIFO_WR2_field_t          stc_SMIF_TX_DATA_MMIO_FIFO_WR2_field_t
#define un_SMIF_TX_DATA_FIFO_WR2                   un_SMIF_TX_DATA_MMIO_FIFO_WR2
#define un_SMIF_TX_DATA_FIFO_WR2_t                 un_SMIF_TX_DATA_MMIO_FIFO_WR2_t
#define unTX_DATA_FIFO_WR2                         unTX_DATA_MMIO_FIFO_WR2
#define SMIF_TX_DATA_FIFO_WR2_DATA0_Pos            SMIF_TX_DATA_MMIO_FIFO_WR2_DATA0_Pos
#define SMIF_TX_DATA_FIFO_WR2_DATA0_Msk            SMIF_TX_DATA_MMIO_FIFO_WR2_DATA0_Msk
#define SMIF_TX_DATA_FIFO_WR2_DATA1_Pos            SMIF_TX_DATA_MMIO_FIFO_WR2_DATA1_Pos
#define SMIF_TX_DATA_FIFO_WR2_DATA1_Msk            SMIF_TX_DATA_MMIO_FIFO_WR2_DATA1_Msk

#define stc_SMIF_TX_DATA_FIFO_WR4_field            stc_SMIF_TX_DATA_MMIO_FIFO_WR4_field
#define stc_SMIF_TX_DATA_FIFO_WR4_field_t          stc_SMIF_TX_DATA_MMIO_FIFO_WR4_field_t
#define un_SMIF_TX_DATA_FIFO_WR4                   un_SMIF_TX_DATA_MMIO_FIFO_WR4
#define un_SMIF_TX_DATA_FIFO_WR4_t                 un_SMIF_TX_DATA_MMIO_FIFO_WR4_t
#define unTX_DATA_FIFO_WR4                         unTX_DATA_MMIO_FIFO_WR4
#define SMIF_TX_DATA_FIFO_WR4_DATA0_Pos            SMIF_TX_DATA_MMIO_FIFO_WR4_DATA0_Pos
#define SMIF_TX_DATA_FIFO_WR4_DATA0_Msk            SMIF_TX_DATA_MMIO_FIFO_WR4_DATA0_Msk
#define SMIF_TX_DATA_FIFO_WR4_DATA1_Pos            SMIF_TX_DATA_MMIO_FIFO_WR4_DATA1_Pos
#define SMIF_TX_DATA_FIFO_WR4_DATA1_Msk            SMIF_TX_DATA_MMIO_FIFO_WR4_DATA1_Msk
#define SMIF_TX_DATA_FIFO_WR4_DATA2_Pos            SMIF_TX_DATA_MMIO_FIFO_WR4_DATA2_Pos
#define SMIF_TX_DATA_FIFO_WR4_DATA2_Msk            SMIF_TX_DATA_MMIO_FIFO_WR4_DATA2_Msk
#define SMIF_TX_DATA_FIFO_WR4_DATA3_Pos            SMIF_TX_DATA_MMIO_FIFO_WR4_DATA3_Pos
#define SMIF_TX_DATA_FIFO_WR4_DATA3_Msk            SMIF_TX_DATA_MMIO_FIFO_WR4_DATA3_Msk

#define stc_SMIF_TX_DATA_FIFO_WR1ODD_field         stc_SMIF_TX_DATA_MMIO_FIFO_WR1ODD_field
#define stc_SMIF_TX_DATA_FIFO_WR1ODD_field_t       stc_SMIF_TX_DATA_MMIO_FIFO_WR1ODD_field_t
#define un_SMIF_TX_DATA_FIFO_WR1ODD                un_SMIF_TX_DATA_MMIO_FIFO_WR1ODD
#define un_SMIF_TX_DATA_FIFO_WR1ODD_t              un_SMIF_TX_DATA_MMIO_FIFO_WR1ODD_t
#define unTX_DATA_FIFO_WR1ODD                      unTX_DATA_MMIO_FIFO_WR1ODD
#define SMIF_TX_DATA_FIFO_WR1ODD_DATA0_Pos         SMIF_TX_DATA_MMIO_FIFO_WR1ODD_DATA0_Pos
#define SMIF_TX_DATA_FIFO_WR1ODD_DATA0_Msk         SMIF_TX_DATA_MMIO_FIFO_WR1ODD_DATA0_Msk

#define unCRYPTO_CMD     SMIF_CRYPTO[0].unCRYPTO_CMD
#define unCRYPTO_INPUT0  SMIF_CRYPTO[0].unCRYPTO_INPUT0
#define unCRYPTO_INPUT1  SMIF_CRYPTO[0].unCRYPTO_INPUT1
#define unCRYPTO_INPUT2  SMIF_CRYPTO[0].unCRYPTO_INPUT2
#define unCRYPTO_INPUT3  SMIF_CRYPTO[0].unCRYPTO_INPUT3
#define unCRYPTO_KEY0    SMIF_CRYPTO[0].unCRYPTO_KEY0
#define unCRYPTO_KEY1    SMIF_CRYPTO[0].unCRYPTO_KEY1
#define unCRYPTO_KEY2    SMIF_CRYPTO[0].unCRYPTO_KEY2
#define unCRYPTO_KEY3    SMIF_CRYPTO[0].unCRYPTO_KEY3
#define unCRYPTO_OUTPUT0 SMIF_CRYPTO[0].unCRYPTO_OUTPUT0
#define unCRYPTO_OUTPUT1 SMIF_CRYPTO[0].unCRYPTO_OUTPUT1
#define unCRYPTO_OUTPUT2 SMIF_CRYPTO[0].unCRYPTO_OUTPUT2
#define unCRYPTO_OUTPUT3 SMIF_CRYPTO[0].unCRYPTO_OUTPUT3

#undef CRYPTO

#endif

#if defined(__cplusplus)
extern "C" {
#endif
  
  /***************************************
  *            Constants
  ****************************************/
  
  /**
  * \addtogroup group_smif_macros
  * \{
  */
  
  /** The driver major version */
#define CY_SMIF_DRV_VERSION_MAJOR       `$CY_MAJOR_VERSION`
  
  /** The driver minor version */
#define CY_SMIF_DRV_VERSION_MINOR       `$CY_MINOR_VERSION`
  
  /** One microsecond timeout for Cy_SMIF_TimeoutRun() */
#define CY_SMIF_WAIT_1_UNIT        (1U)
  
  /** The SMIF driver ID, reported as part of an unsuccessful API return status
  * \ref cy_en_smif_status_t */
#define CY_SMIF_ID CY_PDL_DRV_ID(0x2CU)
  
  
  /**
  * \addtogroup group_smif_macros_isr
  * \{
  */
  
  /** Enable XIP_ALIGNMENT_ERROR interrupt see TRM for details */
#define CY_SMIF_ALIGNMENT_ERROR                 (SMIF_INTR_XIP_ALIGNMENT_ERROR_Msk)
  /** Enable RX_DATA_FIFO_UNDERFLOW interrupt see TRM for details */
#define CY_SMIF_RX_DATA_FIFO_UNDERFLOW          (SMIF_INTR_RX_DATA_MMIO_FIFO_UNDERFLOW_Msk)
  /** Enable TX_DATA_FIFO_OVERFLOW interrupt see TRM for details */
#define CY_SMIF_TX_DATA_FIFO_OVERFLOW           (SMIF_INTR_TX_DATA_FIFO_OVERFLOW_Msk)
  /** Enable TX_CMD_FIFO_OVERFLOW interrupt see TRM for details */
#define CY_SMIF_TX_COMMAND_FIFO_OVERFLOW         (SMIF_INTR_TX_CMD_FIFO_OVERFLOW_Msk)
  /** Enable TR_TX_REQ interrupt see TRM for details */
#define CY_SMIF_TX_DATA_FIFO_LEVEL_TRIGGER      (SMIF_INTR_TR_TX_REQ_Msk)
  /** Enable TR_RX_REQ interrupt see TRM for details */
#define CY_SMIF_RX_DATA_FIFO_LEVEL_TRIGGER      (SMIF_INTR_TR_RX_REQ_Msk)
  
  /** \} group_smif_macros_isr */
  
  /** \cond INTERNAL */
  
#define CY_SMIF_CMD_FIFO_TX_MODE           (0UL)
#define CY_SMIF_CMD_FIFO_TX_COUNT_MODE     (1UL)
#define CY_SMIF_CMD_FIFO_RX_COUNT_MODE     (2UL)
#define CY_SMIF_CMD_FIFO_DUMMY_COUNT_MODE  (3UL)
#define CY_SMIF_CMD_FIFO_DESELECT_MODE     (4UL)
  
#define CY_SMIF_TX_CMD_FIFO_STATUS_RANGE   (8U)
#define CY_SMIF_TX_DATA_FIFO_STATUS_RANGE  (8U)
#define CY_SMIF_RX_DATA_FIFO_STATUS_RANGE  (8U)
#define CY_SMIF_MAX_TX_TR_LEVEL            (8U)
#define CY_SMIF_MAX_RX_TR_LEVEL            (8U)
  
#define CY_SMIF_CRYPTO_START               (1UL)
#define CY_SMIF_CRYPTO_COMPLETED           (0UL)
#define CY_SMIF_CRYPTO_ADDR_MASK           (0xFFFFFFF0UL)
#define CY_SMIF_AES128_BYTES               (16U)
  
#define CY_SMIF_CTL_REG_DEFAULT  (0x00000300U) /* 3 - [13:12] CLOCK_IF_RX_SEL  */
  
  
  /** \endcond*/
  /** \} group_smif_macros */
  
  
  /**
  * \addtogroup group_smif_enums
  * \{
  */
  
  /** The Transfer width options for the command, data, the address and the mode. */
  typedef enum
  {
    CY_SMIF_WIDTH_SINGLE   = 0U,    /**< Normal SPI mode. */
    CY_SMIF_WIDTH_DUAL     = 1U,    /**< Dual SPI mode. */
    CY_SMIF_WIDTH_QUAD     = 2U,    /**< Quad SPI mode. */
    CY_SMIF_WIDTH_OCTAL    = 3U     /**< Octal SPI mode. */
  } cy_en_smif_txfr_width_t;
  
  
  /* Specifies what happens for MMIO interface read accesses to an */
  /* empty RX data FIFO or to a full TX format/data FIFO.          */
  typedef enum
  {
    /**< Generates a bus error. */
    CY_SMIF_BUS_ERROR           = 0UL,
    /** Stalls the bus with the wait states. This option will increase the 
    * interrupt latency. 
    */
    CY_SMIF_WAIT_STATES         = 1UL
  } cy_en_smif_error_event_t;
  
  /* Specifies the clock source for "clk_if".  Must be 0 (clk_hf)  */
  /* before entering DeepSleep; can be returned to 1 (clk_pll) afterwards. */
  typedef enum
  {
    CY_SMIF_CLKIF_SRC_CLK_HF  = 0,    /**< Clock source clk_hf */
    CY_SMIF_CLKIF_SRC_CLK_PLL = 1     /**< Clock source clk_pll */
  } cy_en_cy_clkif_source_t;
  
  /** The data line-selection options for a slave device. */
  typedef enum
  {
    /**
    * smif.spi_data[0] = DATA0, smif.spi_data[1] = DATA1, ..., smif.spi_data[7] = DATA7.
    * This value is allowed for the SPI, DSPI, quad-SPI, dual quad-SPI, and octal-SPI modes.
    */
    CY_SMIF_DATA_SEL0      = 0,
    /**
    * smif.spi_data[2] = DATA0, smif.spi_data[3] = DATA1.
    * This value is only allowed for the SPI and DSPI modes.
    */
    CY_SMIF_DATA_SEL1      = 1,
    /**
    * smif.spi_data[4] = DATA0, smif.spi_data[5] = DATA1, ..., smif.spi_data[7] = DATA3.
    * This value is only allowed for the SPI, DSPI, quad-SPI and dual quad-SPI modes.
    */
    CY_SMIF_DATA_SEL2      = 2,
    /**
    * smif.spi_data[6] = DATA0, smif.spi_data[7] = DATA1.
    * This value is only allowed for the SPI and DSPI modes.
    */
    CY_SMIF_DATA_SEL3      = 3
  } cy_en_smif_data_select_t;
  
  /** The SMIF modes to work with an external memory. */
  typedef enum
  {
    CY_SMIF_NORMAL = 0,         /**< Command mode (MMIO mode). */
    CY_SMIF_MEMORY = 1          /**< XIP (eXecute In Place) mode. or ARB mode for smif_3*/
  } cy_en_smif_mode_t;
  
  
  /** Device interface transmitter clock source */
  typedef enum
  {
    CY_SMIF_DIV_INV_FOR_SDR = 0, /**< transmitter clock clk_if */
    CY_SMIF_INV_FOR_DDR     = 1  /**< transmitter clock clk_if_inv */
  } cy_en_smif_clk_tx_t;
  
  
  /** Specifies the delay line used for RX data capturing with  */
  typedef enum
  {
    CY_SMIF_1_NEW_SEL_PER_TAP  = 0, /**< 1 of these new delay cells per tap providing: 
    granularity  of  max.  ~0.22ns */
    CY_SMIF_1_SEN_SEL_PER_TAP  = 1, /**< 1 SEN_DEL_L4_1 cell per tap providing: 
    granularity of max. ~0.4ns */
    CY_SMIF_2_SEN_SEL_PER_TAP  = 2, /**< 2 SEN_DEL_L4_1 cells per tap providing: 
    granularity of max. ~0.8ns */ 
    CY_SMIF_4_SEN_SEL_PER_TAP  = 3, /**< 4 SEN_DEL_L4_1 cells per tap providing: 
    granularity of max. ~1.6ns */ 
    CY_SMIF_NO_DELAY_SEL       = 0xFF,/**< No delay line (disabled) */ 
  } cy_en_smif_delay_line_t;
  
  /** Specifies use HW DLP calibration or SW DLP calibration */
  typedef enum
  {
    CY_SMIF_DLP_UPDATE_MANUAL = 0, /**< SW will calibrates and sets DLP */
    CY_SMIF_DLP_UPDATE_AUTO   = 1, /**< Use HW DLP calibration scheme */
  } cy_en_smif_dlp_auto_t;
  
  /* Specifies the read data capture cycle for internal clocking. Only applies when receiver clock source is internal clock.*/
  typedef enum
  {
    CY_SMIF_CAPTURE_DELAY_0_CYCLE = 0, /**< Cycle 0 */
    CY_SMIF_CAPTURE_DELAY_1_CYCLE = 1, /**< Cycle 1 */
    CY_SMIF_CAPTURE_DELAY_2_CYCLE = 2, /**< Cycle 2 */
  } cy_en_smif_ddr_capture_delay_t;
  
  /** The SMIF transfer status return values. */
  typedef enum
  {
    CY_SMIF_STARTED,       /**< The SMIF started. */
    CY_SMIF_SEND_CMPLT,    /**< The data transmission is complete. */
    CY_SMIF_SEND_BUSY,     /**< The data transmission is in progress. */
    CY_SMIF_REC_CMPLT,     /**< The data reception is completed. */
    CY_SMIF_REC_BUSY,      /**< The data reception is in progress. */
    CY_SMIF_XIP_ERROR,     /**< An XIP alignment error. */
    CY_SMIF_CMD_ERROR,     /**< A TX CMD FIFO overflow. */
    CY_SMIF_TX_ERROR,      /**< A TX DATA FIFO overflow. */
    CY_SMIF_RX_ERROR       /**< An RX DATA FIFO underflow. */
  } cy_en_smif_txfr_status_t;
  
  /** The SMIF API return values. */
  typedef enum
  {
    CY_SMIF_SUCCESS = 0x00U,           /**< Successful SMIF operation. */
    CY_SMIF_CMD_FIFO_FULL   = CY_SMIF_ID |CY_PDL_STATUS_ERROR | 0x01U,     /**< The command is cancelled. The command FIFO is full. */
    CY_SMIF_EXCEED_TIMEOUT  = CY_SMIF_ID |CY_PDL_STATUS_ERROR | 0x02U,    /**< The SMIF operation timeout exceeded. */
    /**
    * The device does not have a QE bit. The device detects
    * 1-1-4 and 1-4-4 Reads based on the instruction.
    */
    CY_SMIF_NO_QE_BIT       = CY_SMIF_ID |CY_PDL_STATUS_ERROR | 0x03U,
    CY_SMIF_BAD_PARAM       = CY_SMIF_ID |CY_PDL_STATUS_ERROR | 0x04U,   /**< The SMIF API received the wrong parameter */
    CY_SMIF_NO_SFDP_SUPPORT = CY_SMIF_ID |CY_PDL_STATUS_ERROR | 0x05U,   /**< The external memory does not support SFDP (JESD216B). */
    CY_SMIF_BAD_STATUS      = CY_SMIF_ID |CY_PDL_STATUS_ERROR | 0x06U,   /**< The SMIF API received the wrong parameter */
    CY_SMIF_GENERAL_ERROR   = CY_SMIF_ID |CY_PDL_STATUS_ERROR | 0x07U,   /**< Some general error */
  } cy_en_smif_status_t;
  
  /** The SMIF slave select definitions for the driver API. Each slave select is
  * represented by an enumeration that has the bit corresponding to the slave 
  * select number set. */
  typedef enum
  {
    CY_SMIF_SLAVE_SELECT_0 = 1U,  /**< The SMIF slave select 0  */
    
#if (SMIF0_DEVICE_NR >= 2) || (defined(SMIF1_DEVICE_NR) && (SMIF1_DEVICE_NR >= 2))
    CY_SMIF_SLAVE_SELECT_1 = 2U,  /**< The SMIF slave select 1  */
#endif
    
#if (SMIF0_DEVICE_NR >= 3) || (defined(SMIF1_DEVICE_NR) && (SMIF1_DEVICE_NR >= 3))
    CY_SMIF_SLAVE_SELECT_2 = 4U,  /**< The SMIF slave select 2  */
#endif
    
#if (SMIF0_DEVICE_NR >= 4) || (defined(SMIF1_DEVICE_NR) && (SMIF1_DEVICE_NR >= 4))
    CY_SMIF_SLAVE_SELECT_3 = 8U   /**< The SMIF slave select 3  */
#endif
  } cy_en_smif_slave_select_t;
  
  
  
  /** Specifies the clock source for the receiver clock. */
  typedef enum
  {
    CY_SMIF_OUTPUT_CLK       = 0U,  /**< The SMIF output clock */
    CY_SMIF_INV_OUTPUT_CLK   = 1U,  /**< The SMIF output inverted clock */
    CY_SMIF_FEEDBACK_CLK     = 2U,  /**< The SMIF feedback clock */
    CY_SMIF_INV_FEEDBACK_CLK = 3U,  /**< The SMIF feedback inverted clock */
    CY_SMIF_INTERNAL_CLK     = 4U,  /**< The SMIF internal clock */
    CY_SMIF_INV_INTERNAL_CLK = 5U,  /**< The SMIF internal inverted clock */
    CY_SMIF_INV_RWDS         = 6U,  /**< The SMIF inverted rwds */
    CY_SMIF_RWDS             = 7U,  /**< The SMIF rwds */
  } cy_en_smif_clk_rx_t;
  
  /* Specifies the minimum duration of SPI deselection in between SPI transfers: */
  typedef enum
  {
    CY_SMIF_MIN_DESELECT_1_CLK = 0U,   /**<  1 memory interface clock cycle */
    CY_SMIF_MIN_DESELECT_2_CLK = 1U,   /**<  2 memory interface clock cycles */
    CY_SMIF_MIN_DESELECT_3_CLK = 2U,   /**<  3 memory interface clock cycles */
    CY_SMIF_MIN_DESELECT_4_CLK = 3U,   /**<  4 memory interface clock cycles */
    CY_SMIF_MIN_DESELECT_5_CLK = 4U,   /**<  5 memory interface clock cycles */
    CY_SMIF_MIN_DESELECT_6_CLK = 5U,   /**<  6 memory interface clock cycles */
    CY_SMIF_MIN_DESELECT_7_CLK = 6U,   /**<  7 memory interface clock cycles */
    CY_SMIF_MIN_DESELECT_8_CLK = 7U    /**<  8 memory interface clock cycles */
  } cy_en_smif_deselection_t;
  
  /* Specifies the duration between spi_select becomes low to 1st spi_clk edge */
  /* min = 1 memory interface clock cycles when SDR, 1/4 memory interface clock cycles when DDR  */
  typedef enum
  {
    CY_SMIF_SETUP_0_CLK_PULUS_MIN = 0U,   /**< 0 memory interface clock cycles + min duration */
    CY_SMIF_SETUP_1_CLK_PULUS_MIN = 1U,   /**< 1 memory interface clock cycles + min duration */
    CY_SMIF_SETUP_2_CLK_PULUS_MIN = 2U,   /**< 2 memory interface clock cycles + min duration */
    CY_SMIF_SETUP_3_CLK_PULUS_MIN = 3U,   /**< 3 memory interface clock cycles + min duration */
  } cy_en_smif_setup_delay_t;
  
  /* Specifies the duration between last spi_clk edge to spi_select becomes high */
  /* min = 1 memory interface clock cycles when SDR, 1/4 memory interface clock cycles when DDR  */
  typedef enum
  {
    CY_SMIF_HOLD_0_CLK_PULUS_MIN = 0U,   /**< 0 memory interface clock cycles + min duration */
    CY_SMIF_HOLD_1_CLK_PULUS_MIN = 1U,   /**< 1 memory interface clock cycles + min duration */
    CY_SMIF_HOLD_2_CLK_PULUS_MIN = 2U,   /**< 2 memory interface clock cycles + min duration */
    CY_SMIF_HOLD_3_CLK_PULUS_MIN = 3U,   /**< 3 memory interface clock cycles + min duration */
  } cy_en_smif_hold_delay_t;
  
  /** Specifies enabled type of SMIF cache. */
  typedef enum
  {
    CY_SMIF_CACHE_SLOW      = 1U,   /**< The SMIF slow cache (in the clk_slow domain) see TRM for details */
    CY_SMIF_CACHE_FAST      = 2U,   /**< The SMIF fast cache  (in the clk_fast domain) see TRM for details */
    CY_SMIF_CACHE_BOTH      = 3U    /**< The SMIF both caches */
  } cy_en_smif_cache_en_t;
  
  /** Specifies Data Rate. single data rate (SDR) or double data rate (DDR) */
  typedef enum
  {
    CY_SMIF_SDR = 0, /**< Single data rate */
    CY_SMIF_DDR = 1, /**< Double data rate */
  } cy_en_smif_data_rate_t;
  
  /** The size of the device region specified by DEVICE_MASK register */
  typedef enum // casting int32_t is to avoid compiler error.
  {
    CY_SMIF_DEVICE_1M_BYTE   = (int32_t)(0xFFF00000UL), /**< for 1M size device */
    CY_SMIF_DEVICE_2M_BYTE   = (int32_t)(0xFFE00000UL), /**< for 2M size device */
    CY_SMIF_DEVICE_4M_BYTE   = (int32_t)(0xFFC00000UL), /**< for 4M size device */
    CY_SMIF_DEVICE_8M_BYTE   = (int32_t)(0xFF800000UL), /**< for 8M size device */
    CY_SMIF_DEVICE_16M_BYTE  = (int32_t)(0xFF000000UL), /**< for 16M size device */
    CY_SMIF_DEVICE_32M_BYTE  = (int32_t)(0xFE000000UL), /**< for 32M size device */
    CY_SMIF_DEVICE_64M_BYTE  = (int32_t)(0xFC000000UL), /**< for 64M size device */
    CY_SMIF_DEVICE_128M_BYTE = (int32_t)(0xF8000000UL), /**< for 128M size device */
    CY_SMIF_DEVICE_256M_BYTE = (int32_t)(0xF0000000UL), /**< for 256M size device */
    CY_SMIF_DEVICE_512M_BYTE = (int32_t)(0xE0000000UL), /**< for 512M size device */
    CY_SMIF_DEVICE_1G_BYTE   = (int32_t)(0xC0000000UL), /**< for 1G size device */
    CY_SMIF_DEVICE_2G_BYTE   = (int32_t)(0x80000000UL), /**< for 2G size device */
    CY_SMIF_DEVICE_4G_BYTE   = (int32_t)(0x00000000UL), /**< for 4G size device */
  } cy_en_device_size_t;
  
  /** Specifies the size of the XIP device address in Bytes */
  typedef enum
  {
    CY_SMIF_XIP_ADDRESS_1_BYTE = 0, /**< 1 Byte address */
    CY_SMIF_XIP_ADDRESS_2_BYTE = 1, /**< 2 Byte address */
    CY_SMIF_XIP_ADDRESS_3_BYTE = 2, /**< 3 Byte address */
    CY_SMIF_XIP_ADDRESS_4_BYTE = 3, /**< 4 Byte address */
    CY_SMIF_XIP_ADDRESS_5_BYTE = 7, /**< 4 Byte address (spread over 5 bytes) according to Hyperbus protocol */
  } cy_en_smif_xip_addr_byte_t;
  
  /** Continuous transfer merge timeout in clk_mem cycles */
  typedef enum
  {
    CY_SMIF_MER_TIMEOUT_1_CYCLE     = 0, /**< Timeout after 1 clk_mem cycle */
    CY_SMIF_MER_TIMEOUT_16_CYCLE    = 1, /**< Timeout after 16 clk_mem cycle */
    CY_SMIF_MER_TIMEOUT_256_CYCLE   = 2, /**< Timeout after 256 clk_mem cycle */
    CY_SMIF_MER_TIMEOUT_4096_CYCLE  = 3, /**< Timeout after 4096 clk_mem cycle */
    CY_SMIF_MER_TIMEOUT_65536_CYCLE = 4, /**< Timeout after 65536 clk_mem cycle */
  } cy_en_smif_dev_merge_timeout_t;
  
  /** Specify mode/dummy presence */
  typedef enum
  {
    CY_SMIF_NOT_PRESENT   = 0, /**< mode/dummy not present */
    CY_SMIF_PRESENT_1BYTE = 1, /**< mode/dummy 1 byte present */
    CY_SMIF_PRESENT_2BYTE = 2, /**< mode/dummy 2 byte present */
  } cy_en_smif_field_presence_t;
  
  /** Specify whether command to be transmitted is last one or not */
  typedef enum
  {
    NOT_LAST_COMMAND = 0, /**< not last command */
    LAST_COMMAND_BYTE = 1,/**< last command */
  } cy_en_smif_cmd_last_t;
  
  /** Specifies which of the Functional Safety Status field bits are checked */
  typedef enum
  {
    CY_SMIF_CHECK_FUNC_SAFETY      = 0, /**< The associated Functional Safety Status bit is checked */
    CY_SMIF_DONT_CHECK_FUNC_SAFETY = 1, /**< The associated Functional Safety Status bit is not checked
    If the received Functional Safety Status bit = Functional Safety Status Error Polarity (STATUS_ERROR_POLARITY),
    then a Functional Safety Status error interrupt and a XIP bus error response is generated.  */
    
  } cy_en_smif_crc_ctl_status_check_t;
  
  /** Specifies the polarity of the Functional Safety Status field bits */
  typedef enum
  {
    CY_SMIF_ERROR_POL_ACTIVE_LOW  = 0, /**< The associated Functional Safety Status bit is active-low */
    CY_SMIF_ERROR_POL_ACTIVE_HIGH = 1, /**< The associated Functional Safety Status bit is active-high */
  } cy_en_smif_crc_ctl_error_pol_t;
  
  /** Specifies which fields are included in the command / address CRC generation */
  typedef enum
  {
    CY_SMIF_INC_ADDR_MODE      = 0, /**< The command / address CRC field is generated over the address and (if present) mode fields only.*/
    CY_SMIF_INC_CMD_ADDR_MODE  = 1, /**< The command / address CRC field is generated over the command, address and (if present) mode fields.*/
  } cy_en_smif_crc_input_scope_t;
  
  /** Specifies the size of a memory sub page */
  typedef enum
  {
    SUB_PAGE_SIZE_8BYTE  = 0, /**< sub_page_size = 8 words = 16 bytes (default) */
    SUB_PAGE_SIZE_16BYTE = 1, /**< sub_page_size = 16 words = 32 bytes */
    SUB_PAGE_SIZE_32BYTE = 2, /**< sub_page_size = 32 words = 64 bytes */
    SUB_PAGE_SIZE_64BYTE = 3,/**< sub_page_size = 64 words = 128 bytes */
  } en_cy_sub_page_size_t;
  
  /** Specifies the number of sub pages per page */
  typedef enum
  {
    SUB_PAGE_1_PER_PAGE = 0, /**< 1 sub pages per page, i.e. page_size = sub_page_size */
    SUB_PAGE_2_PER_PAGE = 1, /**< 2 sub pages per page, i.e. page_size = 2 x sub_page_size */
    SUB_PAGE_4_PER_PAGE = 2, /**< 4 sub pages per page, i.e. page_size = 4 x sub_page_size */
    SUB_PAGE_8_PER_PAGE = 3, /**< 8 sub pages per page, i.e. page_size = 8 x sub_page_size */
  } en_cy_sub_page_nr_t;
  
  /** \} group_smif_enums */
  
  
  /**
  * \addtogroup group_smif_data_structures
  * \{
  */
  
  
  /***************************************************************************//**
  *
  * The SMIF user callback function type called at the end of a transfer.
  *
  * \param event
  * The event which caused a callback call.
  *
  *******************************************************************************/
  typedef void (*cy_smif_event_cb_t)(uint32_t event);
  
  /** Specifies the bit field of TX_CMD_FIFO_WR when cmd = "TX" command */
  typedef struct
  {
    uint32_t firstByte  : 8; /**< Specifies the transmitted Byte */
    uint32_t secondByte : 8; /**< Specifies the second transmitted Byte. This is only used for octal data transfer with DDR mode */
    uint32_t width      : 2; /**< Specifies the width of the data transfer */
    uint32_t dataRate   : 1; /**< Specifies the mode of transfer rate */
    uint32_t last       : 1; /**< Specifies whether this is the last TX Byte */
    uint32_t selectDev  : 4; /**< Specifies which of the four devices are selected */
    uint32_t cmd        : 3; /**< Specifies the specific command */
    uint32_t reserved0  : 5; /**< Reserved */
  } cy_stc_smif_cmd_fifo_tx_t;
  
  /** Specifies the bit field of TX_CMD_FIFO_WR when cmd = "TX_COUNT" command */
  typedef struct
  {
    uint32_t dataNum    : 16;/**< Specifies the number of to be transmitted memory data units */
    uint32_t width      : 2; /**< Specifies the width of the data transfer */
    uint32_t dataRate   : 1; /**< Specifies the mode of transfer rate */
    uint32_t last       : 1; /**< Specifies whether this is the last TX Byte */
    uint32_t selectDev  : 4; /**< Specifies which of the four devices are selected */
    uint32_t cmd        : 3; /**< Specifies the specific command */
    uint32_t reserved0  : 5; /**< Reserved */
  } cy_stc_smif_cmd_fifo_tx_count_t;
  
  /** Specifies the bit field of TX_CMD_FIFO_WR when cmd = "RX_COUNT" command */
  typedef struct
  {
    uint32_t dataNum    : 16;/**< Specifies the number of to be received memory data units */
    uint32_t width      : 2; /**< Specifies the width of the data transfer */
    uint32_t dataRate   : 1; /**< Specifies the mode of transfer rate */
    uint32_t last       : 1; /**< Specifies whether this is the last TX Byte */
    uint32_t selectDev  : 4; /**< Specifies which of the four devices are selected */
    uint32_t cmd        : 3; /**< Specifies the specific command */
    uint32_t reserved0  : 5; /**< Reserved */
  } cy_stc_smif_cmd_fifo_rx_count_t;
  
  /** Specifies the bit field of TX_CMD_FIFO_WR when cmd = "DUMMY_COUNT" command */
  typedef struct
  {
    uint32_t cycleNum   : 16;
    uint32_t width      : 2; /**< Specifies the width of the data transfer */
    uint32_t dataRate   : 1; /**< Specifies the mode of transfer rate */
    uint32_t last       : 1; /**< Specifies whether this is the last TX Byte */
    uint32_t doubleDm   : 1; /**< specifies whether an active RWDS refresh indicator (captured with the previous rising memory clock output) causes to double the number of dummy cycles. This is used for the variable latency in HyperRAM. */
    uint32_t rwdsInDm   : 1; /**< specifies whether the RWDS output signal should be driven starting in the last dummy cycle until DESELECT. This is needed for write transactions with RWDS (e.g. Hyperbus) before a TX_COUNT command. */
    uint32_t dlpEn      : 1; /**< specifies whether data learning is enabled. In XIP mode this field is set for read transactions when CTL.INT_CLOCK_DL_ENABLED is "1"). */
    uint32_t capEn      : 1; /**< specifies whether RWDS / DQS based capturing is enabled in the last dummy cycle, i.e. this needs to be set to '1' for read transactions to memory device using RWDS / DQS capturing scheme. */
    uint32_t cmd        : 3; /**< Specifies the specific command */
    uint32_t reserved0  : 5; /**< Reserved */
  } cy_stc_smif_cmd_fifo_dm_t;
  
  /** Specifies the bit field of TX_CMD_FIFO_WR when cmd = "DESELECT" command */
  typedef struct
  {
    uint32_t reserved1  : 24; /**< Reserved */
    uint32_t cmd        : 3;  /**< Specifies the specific command */
    uint32_t reserved0  : 5;  /**< Reserved */
  } cy_stc_smif_cmd_fifo_deselect_t;
  
  
  /** union for TX_CMD_FIFO_WR as per it's commands */
  typedef union
  {
    cy_stc_smif_cmd_fifo_tx_t       fieldTx;       /**< Bit field when cmd = "TX" command \ref cy_stc_smif_cmd_fifo_tx_t */
    cy_stc_smif_cmd_fifo_tx_count_t fieldTxCount;  /**< Bit field when cmd = "TX_COUNT" command \ref cy_stc_smif_cmd_fifo_tx_count_t */
    cy_stc_smif_cmd_fifo_rx_count_t fieldRxCount;  /**< Bit field when cmd = "RX_COUNT" command \ref cy_stc_smif_cmd_fifo_rx_count_t */
    cy_stc_smif_cmd_fifo_dm_t       fieldDummy;    /**< Bit field when cmd = "DUMMY_COUNT" command \ref cy_stc_smif_cmd_fifo_dm_t */
    cy_stc_smif_cmd_fifo_deselect_t fieldDeselect; /**< Bit field when cmd = "DESELECT" command \ref cy_stc_smif_cmd_fifo_deselect_t */
    uint32_t                        u32;           /**< 32-bit access */
  } cy_un_smif_cmd_fifo_t;
  
  
  /** The SMIF configuration structure. */
  typedef struct
  {
    cy_en_smif_clk_tx_t            txClk;         /**< TX clock source selection \ref cy_en_smif_clk_tx_t */
    cy_en_smif_clk_rx_t            rxClk;         /**< RX clock source selection \ref cy_en_smif_clk_rx_t */
    cy_en_smif_delay_line_t        delaySel;      /**< Delay line selection for RX clock \ref cy_en_smif_delay_line_t */
    cy_en_smif_dlp_auto_t          dlpAuto;       /**< Use SW calibration or HW calibration \ref cy_en_smif_dlp_auto_t */
    cy_en_smif_ddr_capture_delay_t capDelay;      /**< Selection for capture delay \ref cy_en_smif_ddr_capture_delay_t */
    cy_en_smif_deselection_t       deselectDelay; /**< \ref cy_en_smif_deselection_t */
    cy_en_smif_setup_delay_t       setupDelay;    /**< \ref cy_en_smif_setup_delay_t */
    cy_en_smif_hold_delay_t        holdDelay;     /**< \ref cy_en_smif_hold_delay_t */ 
    cy_en_smif_mode_t              mode;          /**<  Specifies the mode of operation \ref cy_en_smif_mode_t. */
    cy_en_smif_error_event_t       blockEvent;    /**< Specifies what happens when there is a Read  
    * from an empty RX FIFO or a Write to a full 
    * TX FIFO. \ref cy_en_smif_error_event_t. */
    cy_en_cy_clkif_source_t       clkIfSrc;       /**< \ref cy_en_cy_clkif_source_t */
  } cy_stc_smif_config_t;
  
  
  /** The SMIF internal context data. The user must not modify it. */
  typedef struct
  {
    uint8_t volatile * volatile txBufferAddress;    /**<  The pointer to the data to transfer */
    /**
    * The transfer counter.
    */
    uint32_t volatile txBufferCounter;
    uint8_t volatile * volatile rxBufferAddress;    /**<  The pointer to the variable where the received data is stored */
    /**
    * The transfer counter.
    */
    uint32_t volatile rxBufferCounter;
    /**
    * The status of the transfer. The transmitting / receiving is completed / in progress
    */
    cy_en_smif_txfr_status_t volatile transferStatus;
    cy_smif_event_cb_t volatile txCmpltCb;          /**< The user-defined callback executed at the completion of a transmission */
    cy_smif_event_cb_t volatile rxCmpltCb;          /**< The user-defined callback executed at the completion of a reception */
    /**
    * The timeout in microseconds for the blocking functions. This timeout value applies to all blocking APIs.
    */
    uint32_t timeout;
    cy_en_smif_data_rate_t  preCmdDataRate;
    cy_en_smif_txfr_width_t preCmdWidth;
    uint32_t volatile txCmdBufCounter;
  } cy_stc_smif_context_t;
  
  /** TX/RX CMD/MODE control in XIP mode */
  typedef struct
  {
    uint16_t                    cmd;      /**< Command byte code. If presence = CY_SMIF_PRESENT_1BYTE, only lower 8bit is active. */
    cy_en_smif_txfr_width_t     width;    /**< Width of data transfer \ref cy_en_smif_txfr_width_t */
    cy_en_smif_data_rate_t      dataRate; /**< Mode of transfer rate \ref cy_en_smif_data_rate_t */
    cy_en_smif_field_presence_t presence; /**< Presence of command field \ref cy_en_smif_field_presence_t */
  } stc_cy_smif_cmd_mode_ctl_t;
  
  /** TX/RX ADDR/DATA control in XIP mode */
  typedef struct
  {
    cy_en_smif_txfr_width_t width;    /**< Width of data transfer \ref cy_en_smif_txfr_width_t */
    cy_en_smif_data_rate_t  dataRate; /**< Mode of transfer rate \ref cy_en_smif_data_rate_t */
  } stc_cy_smif_addr_data_ctl_t;
  
  /** RX DUMMY control in XIP mode */
  typedef struct
  {
    uint8_t numOfDummy;                   /**< Number of dummy cycles */
    cy_en_smif_field_presence_t presence; /**< Presence of dummy cycles. If presence == CY_SMIF_PRESENT_2BYTE, Dummy cycles are doubled
    when RWDS refresh indicator is high during CA cycle. (HyperRAM variable latency mode) */
  } stc_cy_smif_rd_dummy_ctl_t;
  
  /** TX DUMMY control in XIP mode */
  typedef struct
  {
    uint8_t numOfDummy;                   /**< Number of dummy cycles */
    bool    rwdsEn;                       /**< Read-Write-Data-Strobe Enable. Specifies whether the RWDS output signal
    should be driven starting in the last dummy cycle until DESELECT.
    This is needed for write transactions with byte masking via RWDS (e.g. Hyperbus).
    '0': do not drive RWDS output
    '1': drive RWDS output starting in last dummy cycle until DESELECT */
    cy_en_smif_field_presence_t presence; /**< Presence of dummy cycles. If presence == CY_SMIF_PRESENT_2BYTE, Dummy cycles are doubled
    when RWDS refresh indicator is high during CA cycle. (HyperRAM variable latency mode) */
  } stc_cy_smif_wr_dummy_ctl_t;
  
  /** RX CRC control in XIP mode */
  typedef struct
  {
    uint8_t                        inputSize;          /**< Number of input data bytes for CRC generation */
    cy_en_smif_crc_ctl_status_check_t funcSafeChecked; /**< Specifies which of the Functional Safety Status field bits are checked.
    There is 1 mask bit for each Functional Safety Status bit. 
    \ref cy_en_smif_crc_ctl_status_check_t */
    cy_en_smif_crc_ctl_error_pol_t errPol;             /**< Specifies the polarity of the Functional Safety Status field bits.
    There is 1 polarity bit for each Functional Safety Status bit. 
    \ref cy_en_smif_crc_ctl_status_check_t */
    cy_en_smif_txfr_width_t        width;              /**< Width of command / address CRC field. \ref cy_en_smif_txfr_width_t */
    
    cy_en_smif_data_rate_t         dataRate;           /**< Mode of transfer rate of command / address CRC field \ref cy_en_smif_data_rate_t */
    
    cy_en_smif_crc_input_scope_t   inputScope;         /**< Specifies which fields are included in the command / address CRC generation \ref cy_en_smif_crc_input_scope_t */
    bool                           crcChecked;         /**< Specifies whether or not the read data CRC field is checked.
    '0': Read data CRC field is reserved in RX data stream, but not checked.
    '1': Read data CRC field is checked in RX data stream. If CRC does not match an CRC error interrupt and a XIP bus error response is generated. */
    bool                           cmd_addrCrcPresent; /**< Presence of command / address CRC field */
    bool                           dataCrcPresent;     /**< Presence of data CRC field */
  } stc_cy_smif_rd_crc_ctl_t;
  
  /** TX CRC control in XIP mode */
  typedef struct
  {
    uint8_t                      inputSize;            /**< Number of input data bytes for CRC generation */
    cy_en_smif_txfr_width_t      width;                /**< Width of command / address CRC field \ref cy_en_smif_txfr_width_t */
    cy_en_smif_data_rate_t       dataRate;             /**< Mode of transfer rate of command / address CRC field \ref cy_en_smif_data_rate_t */
    cy_en_smif_crc_input_scope_t inputScope;           /**< Specifies which fields are included in the command / address CRC generation \ref cy_en_smif_crc_input_scope_t*/
    bool                         cmd_addrCrcPresent;   /**< Presence of command / address CRC field */
    bool                         dataCrcPresent;       /**< Presence of data CRC field */
  } stc_cy_smif_wr_crc_ctl_t;
  
  /** RX BOUND control in XIP mode */
  typedef struct
  {
    uint8_t               numOfCycle;  /**< Number of base latency cycles used for calculating the number of fist page boundary crossing latency cycles */
    en_cy_sub_page_size_t subPageSize; /**< Specifies the size of a memory sub page \ref en_cy_sub_page_size_t */
    en_cy_sub_page_nr_t   subPageNr;   /**< Specifies the number of sub pages per page \ref en_cy_sub_page_nr_t */
    bool                  subsqBndEn;  /**< Enable subsequent page boundary latency cycles */
    bool                  bdLPresent;  /**< Presence of first page boundary latency cycles */
  } stc_cy_smif_rd_bound_ctl_t;
  
  /** \} group_smif_data_structures */
  
  
  /**
  * \addtogroup group_smif_low_level_functions
  * \{
  */
  
  cy_en_smif_status_t Cy_SMIF_Init(volatile stc_SMIF_t *base, cy_stc_smif_config_t const *config,
                                   uint32_t timeout,
                                   cy_stc_smif_context_t *context);
  void Cy_SMIF_DeInit(volatile stc_SMIF_t *base);
  __STATIC_INLINE cy_en_smif_mode_t Cy_SMIF_GetMode(volatile stc_SMIF_t *base);
#if defined (tviibh4m) || defined (tviibh8m)
  cy_en_smif_status_t Cy_SMIF_DLP_Caliblation(volatile stc_SMIF_t *base, uint8_t dlp, uint32_t dummyReadAddr);
#elif defined (tviic2d6m) || defined (tviic2d4m)
  cy_en_smif_status_t Cy_SMIF_DLP_Caliblation(volatile stc_SMIF_t *base, uint16_t dlp, uint8_t dlp_size, uint8_t w_lvl, uint32_t dummyReadAddr);
#endif
  cy_en_smif_status_t  Cy_SMIF_TransmitCommand(volatile stc_SMIF_t *base,
                                               uint16_t cmd,
                                               bool isCommand2Byte,
                                               cy_en_smif_txfr_width_t cmdTxfrWidth,
                                               cy_en_smif_data_rate_t  cmdDataRate,
                                               uint8_t const cmdParam[],
                                               uint32_t paramSize,
                                               cy_en_smif_txfr_width_t paramTxfrWidth,
                                               cy_en_smif_data_rate_t  paramDataRate,
                                               cy_en_smif_slave_select_t  slaveSelect,
                                               cy_en_smif_cmd_last_t cmpltTxfr,
                                               cy_stc_smif_context_t *context);
  cy_en_smif_status_t  Cy_SMIF_TransmitData(volatile stc_SMIF_t *base,
                                            const uint8_t* txBuffer,
                                            uint32_t size,
                                            cy_en_smif_txfr_width_t transferWidth,
                                            cy_en_smif_data_rate_t  dataDataRate,
                                            cy_en_smif_slave_select_t  slaveSelect,
                                            cy_smif_event_cb_t TxCmpltCb,
                                            cy_stc_smif_context_t *context);
  cy_en_smif_status_t  Cy_SMIF_TransmitDataBlocking(volatile stc_SMIF_t *base,
                                                    const uint8_t *txBuffer,
                                                    uint32_t size,
                                                    cy_en_smif_txfr_width_t transferWidth,
                                                    cy_en_smif_data_rate_t  dataDataRate,
                                                    cy_en_smif_slave_select_t  slaveSelect,
                                                    cy_stc_smif_context_t *context);
  cy_en_smif_status_t  Cy_SMIF_ReceiveData(volatile stc_SMIF_t *base,
                                           uint8_t *rxBuffer,
                                           uint32_t size,
                                           cy_en_smif_txfr_width_t transferWidth,
                                           cy_en_smif_data_rate_t  dataRate,
                                           cy_en_smif_slave_select_t  slaveSelect,
                                           cy_smif_event_cb_t RxCmpltCb,
                                           cy_stc_smif_context_t *context);
  cy_en_smif_status_t  Cy_SMIF_ReceiveDataBlocking(volatile stc_SMIF_t *base,
                                                   uint8_t *rxBuffer,
                                                   uint32_t size,
                                                   cy_en_smif_txfr_width_t transferWidth,
                                                   cy_en_smif_data_rate_t  dataRate,
                                                   cy_en_smif_slave_select_t  slaveSelect,
                                                   cy_stc_smif_context_t *context);
  cy_en_smif_status_t  Cy_SMIF_ReceiveDataCmd(volatile stc_SMIF_t *base,
                                              uint8_t *rxBuffer,
                                              uint32_t size,
                                              cy_en_smif_txfr_width_t transferWidth,
                                              cy_en_smif_data_rate_t  dataRate,
                                              cy_en_smif_slave_select_t  slaveSelect,
                                              cy_smif_event_cb_t RxCmpltCb,
                                              cy_stc_smif_context_t *context);
  extern cy_en_smif_status_t  Cy_SMIF_SendDummyCycles(volatile stc_SMIF_t *base,
                                                      cy_en_smif_txfr_width_t transferWidth,
                                                      cy_en_smif_data_rate_t  dataRate,
                                                      bool doubleDm,
                                                      bool rwdsInDm,
                                                      bool dlpEn,
                                                      bool capEn,
                                                      uint32_t cycles,
                                                      cy_stc_smif_context_t *context);
  cy_en_smif_status_t  Cy_SMIF_Deselect(volatile stc_SMIF_t *base, cy_stc_smif_context_t *context);
  cy_en_smif_txfr_status_t Cy_SMIF_GetTxfrStatus(volatile stc_SMIF_t *base, cy_stc_smif_context_t const *context);
  void Cy_SMIF_Enable(volatile stc_SMIF_t *base, cy_stc_smif_context_t *context);
  cy_en_smif_status_t Cy_SMIF_SetMode(volatile stc_SMIF_t *base, cy_en_smif_mode_t mode);
  __STATIC_INLINE void Cy_SMIF_Disable(volatile stc_SMIF_t *base);
#if defined (tviibh4m) || defined (tviibh8m)
  __STATIC_INLINE void Cy_SMIF_SetMasterDLP(volatile stc_SMIF_t *base, uint8_t dlp);
  __STATIC_INLINE uint8_t Cy_SMIF_GetMasterDLP(volatile stc_SMIF_t *base);
#elif defined (tviic2d6m) || defined (tviic2d4m)
  __STATIC_INLINE cy_en_smif_status_t Cy_SMIF_SetMasterDLP(volatile stc_SMIF_t *base, uint16_t dlp, uint8_t dlp_size, uint8_t warning_level);
  __STATIC_INLINE uint16_t Cy_SMIF_GetMasterDLP(volatile stc_SMIF_t *base);
  __STATIC_INLINE uint8_t Cy_SMIF_GetMasterDLP_Size(volatile stc_SMIF_t *base);
  __STATIC_INLINE uint8_t Cy_SMIF_GetMasterDLP_WarnLevel(volatile stc_SMIF_t *base);
#endif
  __STATIC_INLINE uint8_t Cy_SMIF_GetTapNumCapturedCorrectDLP(volatile stc_SMIF_t *base, uint8_t bit);
  __STATIC_INLINE cy_en_smif_status_t Cy_SMIF_Set_RWDS_DelayTap(volatile stc_SMIF_t *base, uint8_t tapSel);
  __STATIC_INLINE uint8_t Cy_SMIF_Get_RWDS_DelayTap(volatile stc_SMIF_t *base);
  __STATIC_INLINE uint8_t Cy_SMIF_GetMmioFifoStatus(volatile stc_SMIF_t *base);
  __STATIC_INLINE void  Cy_SMIF_SetInterruptMask(volatile stc_SMIF_t *base, uint32_t interrupt);
  __STATIC_INLINE uint32_t  Cy_SMIF_GetInterruptMask(volatile stc_SMIF_t *base);
  __STATIC_INLINE uint32_t  Cy_SMIF_GetInterruptStatusMasked(volatile stc_SMIF_t *base);
  __STATIC_INLINE uint32_t  Cy_SMIF_GetInterruptStatus(volatile stc_SMIF_t *base);
  __STATIC_INLINE void  Cy_SMIF_SetInterrupt(volatile stc_SMIF_t *base, uint32_t interrupt);
  __STATIC_INLINE void  Cy_SMIF_ClearInterrupt(volatile stc_SMIF_t *base, uint32_t interrupt);
  __STATIC_INLINE void  Cy_SMIF_SetTxFifoTriggerLevel(volatile stc_SMIF_t *base, uint32_t level);
  __STATIC_INLINE void  Cy_SMIF_SetRxFifoTriggerLevel(volatile stc_SMIF_t *base, uint32_t level);
  __STATIC_INLINE uint32_t  Cy_SMIF_GetCmdFifoStatus(volatile stc_SMIF_t *base);
  __STATIC_INLINE uint32_t  Cy_SMIF_GetTxFifoStatus(volatile stc_SMIF_t *base);
  __STATIC_INLINE uint32_t  Cy_SMIF_GetRxFifoStatus(volatile stc_SMIF_t *base);
  cy_en_smif_status_t  Cy_SMIF_Encrypt(volatile stc_SMIF_t *base,
                                       uint32_t address,
                                       uint8_t data[],
                                       uint32_t sizeInByte,
                                       cy_stc_smif_context_t const *context);
  __STATIC_INLINE bool Cy_SMIF_IsBusy(volatile stc_SMIF_t *base);
  __STATIC_INLINE void Cy_SMIF_Interrupt(volatile stc_SMIF_t *base, cy_stc_smif_context_t *context);
  cy_en_smif_status_t Cy_SMIF_CacheEnable(volatile stc_SMIF_t *base, cy_en_smif_cache_en_t cacheType);
  cy_en_smif_status_t Cy_SMIF_CacheDisable(volatile stc_SMIF_t *base, cy_en_smif_cache_en_t cacheType);
  cy_en_smif_status_t Cy_SMIF_CachePrefetchingEnable(volatile stc_SMIF_t *base, cy_en_smif_cache_en_t cacheType);
  cy_en_smif_status_t Cy_SMIF_CachePrefetchingDisable(volatile stc_SMIF_t *base, cy_en_smif_cache_en_t cacheType);
  cy_en_smif_status_t Cy_SMIF_CacheInvalidate(volatile stc_SMIF_t *base, cy_en_smif_cache_en_t cacheType);
  cy_en_smif_status_t Cy_SMIF_SetDelayLineForFreq(volatile stc_SMIF_t *base, uint8_t freqMhz, bool isDDR, uint8_t *calculatedTap);
  volatile stc_SMIF_t * Cy_SMIF_GetSmifBasePtrFromAnySmifPtr(volatile void *any_smif_address);
  cy_en_smif_slave_select_t Cy_SMIF_GetSlaveSelectFromDeviceBasePtr(volatile stc_SMIF_DEVICE_t *device);
  cy_en_smif_slave_select_t Cy_SMIF_GetSlaveSelectFromDeviceNumber(uint8_t device_number);
  volatile stc_SMIF_DEVICE_t * Cy_SMIF_GetDeviceBasePtrFromSmifBasePtrAndSlaveSelect(volatile stc_SMIF_t *base, cy_en_smif_slave_select_t slave);
  volatile stc_SMIF_DEVICE_t * Cy_SMIF_GetDeviceBasePtrFromSmifBasePtrAndDeviceNumber(volatile stc_SMIF_t *base, uint8_t device_number);
  
  
  /** \addtogroup group_smif_functions_syspm_callback
  * The driver supports SysPm callback for Deep Sleep and Hibernate transition.
  * \{
  */
  cy_en_syspm_status_t Cy_SMIF_DeepSleepCallback(cy_stc_syspm_callback_params_t *callbackParams);
  cy_en_syspm_status_t Cy_SMIF_HibernateCallback(cy_stc_syspm_callback_params_t *callbackParams);
  /** \} */
  
  
  /***************************************
  *  Internal SMIF function declarations
  ****************************************/
  /** \cond INTERNAL */
  __STATIC_INLINE void Cy_SMIF_PushTxFifo(volatile stc_SMIF_t *baseaddr, cy_stc_smif_context_t *context);      /**< Writes transmitted data into the FIFO. */
  __STATIC_INLINE void Cy_SMIF_PopRxFifo(volatile stc_SMIF_t *baseaddr, cy_stc_smif_context_t *context);       /**< Reads received data from the FIFO. */
  __STATIC_INLINE uint32_t Cy_SMIF_PackBytesArray(uint8_t const buff[], bool fourBytes);
  __STATIC_INLINE void Cy_SMIF_UnPackByteArray(uint32_t inValue, uint8_t outBuff[], bool fourBytes);
  __STATIC_INLINE cy_en_smif_status_t Cy_SMIF_TimeoutRun(uint32_t *timeoutUnits);
  cy_en_smif_status_t Cy_SMIF_CRC_Calibration(volatile stc_SMIF_t *base, uint8_t inData[], uint32_t sizeInByte, uint8_t* result);
  __STATIC_INLINE uint32_t CY_SMIF_GetDeviceNumber(volatile void *base);
  __STATIC_INLINE uint32_t CY_SMIF_GetDelayTapsNumber(volatile void *base);
  __STATIC_INLINE uint32_t CY_SMIF_GetXIP_Mask(volatile void *base);
  __STATIC_INLINE uint32_t CY_SMIF_GetXIP_Address(volatile void *base);
  __STATIC_INLINE uint32_t CY_SMIF_MaskValueToSizeInByte(cy_en_device_size_t mask);
  __STATIC_INLINE uint32_t Cy_SMIF_Reverse4ByteEndian(uint32_t in);
  /** \endcond*/
  
  /** \} group_smif_low_level_functions */
  
  /**
  * \addtogroup group_smif_low_level_functions
  * \{
  */
  /*******************************************************************************
  * Function Name: Cy_SMIF_GetMode
  ****************************************************************************//**
  *
  * Reads the mode of operation for the SMIF. The mode of operation can be the
  * XIP mode where the slave devices are mapped as memories and are directly
  * accessed from the TVII register map. In the MMIO mode, the SMIF block acts as
  * a simple SPI engine.
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  * \return The mode of SMIF operation (see \ref cy_en_smif_mode_t).
  *
  *******************************************************************************/
  __STATIC_INLINE cy_en_smif_mode_t Cy_SMIF_GetMode(volatile stc_SMIF_t *base)
  {
    return (cy_en_smif_mode_t)(base->unCTL.stcField.u1XIP_MODE);
  }
  
#if defined (tviibh4m) || defined (tviibh8m)
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_SetMasterDLP
  ****************************************************************************//**
  *
  * This function sets Data Learning Pattern to be compared.
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  * \param dlp
  * DLP value to be set.
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_SetMasterDLP(volatile stc_SMIF_t *base, uint8_t dlp)
  {
    base->unDLP.stcField.u8DLP = dlp;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_GetMasterDLP
  ****************************************************************************//**
  *
  * This function reads Data Learning Pattern which have been set.
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  * \return Master DLP value.
  *
  *******************************************************************************/
  __STATIC_INLINE uint8_t Cy_SMIF_GetMasterDLP(volatile stc_SMIF_t *base)
  {
    return (uint8_t)(base->unDLP.stcField.u8DLP);
  }
  
#elif defined (tviic2d6m) || defined (tviic2d4m)
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_SetMasterDLP
  ****************************************************************************//**
  *
  * This function sets Data Learning Pattern to be compared.
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  * \param dlp
  * DLP value to be set.
  *
  * \param dlp_size
  * Size of data learning pattern bit. For DDR read data transfers the size of 
  * the DLP must be an even number. e.g dlp_size = 8 means 8 bit DLP
  *
  * \param warning_level
  * Determines when the DL_WARNING interrupt cause field is set.
  * DL_WARNING is set when the number of delay line taps resulting to a correctly
  * captured DLP is less than DL_WARNING_LEVEL for at least one input data line.
  *
  * \return \ref cy_en_smif_status_t
  *
  *******************************************************************************/
  __STATIC_INLINE cy_en_smif_status_t Cy_SMIF_SetMasterDLP(volatile stc_SMIF_t *base, uint16_t dlp, uint8_t dlp_size, uint8_t warning_level)
  {
    un_SMIF_DL_CTL_t tmpDL_CTL               = { 0u };
    
    if((dlp_size == 0) && (16 < dlp_size))
    {
      return CY_SMIF_BAD_PARAM;
    }
    
    if(15 < warning_level)
    {
      return CY_SMIF_BAD_PARAM;
    }
    
    tmpDL_CTL.stcField.u16DLP             = dlp;
    tmpDL_CTL.stcField.u4DLP_SIZE         = (dlp_size - 1);
    tmpDL_CTL.stcField.u4DL_WARNING_LEVEL = warning_level;
    
    base->unDL_CTL.u32Register               = tmpDL_CTL.u32Register;
    
    return CY_SMIF_SUCCESS;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_GetMasterDLP
  ****************************************************************************//**
  *
  * This function reads Data Learning Pattern which have been set.
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  * \return Master DLP value.
  *
  *******************************************************************************/
  __STATIC_INLINE uint16_t Cy_SMIF_GetMasterDLP(volatile stc_SMIF_t *base)
  {
    return (uint16_t)(base->unDL_CTL.stcField.u16DLP);
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_GetMasterDLP_Size
  ****************************************************************************//**
  *
  * This function reads DLP size which have been set.
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  * \return Master DLP size value.
  *
  *******************************************************************************/
  __STATIC_INLINE uint8_t Cy_SMIF_GetMasterDLP_Size(volatile stc_SMIF_t *base)
  {
    return (uint8_t)(base->unDL_CTL.stcField.u4DLP_SIZE);
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_GetMasterDLP_WarnLevel
  ****************************************************************************//**
  *
  * This function reads DLP warning level which have been set.
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  * \return Master DLP warning level value.
  *
  *******************************************************************************/
  __STATIC_INLINE uint8_t Cy_SMIF_GetMasterDLP_WarnLevel(volatile stc_SMIF_t *base)
  {
    return (uint8_t)(base->unDL_CTL.stcField.u4DL_WARNING_LEVEL);
  }
  
#endif
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_GetTapNumCapturedCorrectDLP
  ****************************************************************************//**
  *
  * This function return tap number indicated by input bit number.
  * \note not tested yet. this function may cause hardfault.
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  * \param bit
  * The tap number of the bit will be read.
  *
  * \return tap number
  *
  *******************************************************************************/
  __STATIC_INLINE uint8_t Cy_SMIF_GetTapNumCapturedCorrectDLP(volatile stc_SMIF_t *base, uint8_t bit)
  {
    CY_ASSERT_L2(bit < 8u);
    
    if(bit < 4u)
    {
      return(base->unDL_STATUS0.au8Byte[bit]);
    }
    else
    {
      return(base->unDL_STATUS1.au8Byte[bit - 4u]);
    }
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_Set_DelayLineSel
  ****************************************************************************//**
  *
  * This function enable delay line, and set line selection which is input.
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  * \param lineSel
  * delay line selection to be set
  *
  * \return status (see \ref cy_en_smif_status_t).
  *
  *******************************************************************************/
  __STATIC_INLINE cy_en_smif_status_t Cy_SMIF_Set_DelayLineSel(volatile stc_SMIF_t *base, cy_en_smif_delay_line_t lineSel)
  {
    if(lineSel == CY_SMIF_NO_DELAY_SEL)
    {
      base->unCTL.stcField.u1DELAY_TAP_ENABLED = 0;
      return CY_SMIF_SUCCESS;
    }
    
    un_SMIF_CTL_t tempCTL;
    tempCTL.u32Register = base->unCTL.u32Register;
    
    tempCTL.stcField.u1DELAY_TAP_ENABLED = 1;
    tempCTL.stcField.u3DELAY_LINE_SEL    = lineSel;
    
    base->unCTL.u32Register = tempCTL.u32Register;
    return CY_SMIF_SUCCESS;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_Get_DelayLineSel
  ****************************************************************************//**
  *
  * This function returns the currently selected delay line
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  * \return delay line (see \ref cy_en_smif_delay_line_t).
  *
  *******************************************************************************/
  __STATIC_INLINE cy_en_smif_delay_line_t Cy_SMIF_Get_DelayLineSel(volatile stc_SMIF_t *base)
  {
    if(base->unCTL.stcField.u1DELAY_TAP_ENABLED == 0)
    {
      return CY_SMIF_NO_DELAY_SEL;
    }
    else
    {
      return (cy_en_smif_delay_line_t) base->unCTL.stcField.u3DELAY_LINE_SEL;
    }
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_Set_DelayTapSel
  ****************************************************************************//**
  *
  * This function sets delay tap number for the SMIF (common for all its devices).
  * To ensure compatibility with users of this API independent of the current
  * SMIF IP version, it accepts any SMIF pointer as parameter, e.g. a SMIF base
  * pointer or a SMIF_DEVICE base pointer. This means this API can be called with
  * a SMIF_DEVICE base pointer and the calling code will work with CY_SMIF_NEW_VERSION
  * or CY_SMIF_OLD_VERSION
  *
  * \param smif_or_device_base
  * Holds the base address of the SMIF block or SMIF_DEVICE block registers.
  *
  * \param tapSel
  * delay tap selection to be set
  *
  * \return status (see \ref cy_en_smif_status_t).
  *
  *******************************************************************************/
  __STATIC_INLINE cy_en_smif_status_t Cy_SMIF_Set_DelayTapSel(volatile void *smif_or_device_base, uint8_t tapSel)
  {
#ifdef CY_SMIF_NEW_VERSION
    if(tapSel > (CY_SMIF_GetDelayTapsNumber(smif_or_device_base) - 1))
    {
      return CY_SMIF_BAD_PARAM;
    }
    
    ((stc_SMIF_DEVICE_t*)smif_or_device_base)->unDELAY_TAP_SEL.stcField.u8SEL = tapSel;
    
    return CY_SMIF_SUCCESS;
    
#else
    // Ensure that if a device pointer has been passed, it is converted to a SMIF base pointer
    volatile stc_SMIF_t * base = Cy_SMIF_GetSmifBasePtrFromAnySmifPtr(smif_or_device_base);
    
    if(tapSel > (CY_SMIF_GetDelayTapsNumber(base) - 1))
    {
      return CY_SMIF_BAD_PARAM;
    }
    
    base->unDELAY_TAP_SEL.stcField.u8SEL = tapSel;
    
    return CY_SMIF_SUCCESS;
#endif
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_Get_DelayTapSel
  ****************************************************************************//**
  *
  * This function returns delay tap number which has been set for the SMIF (common for all its devices).
  * To ensure compatibility with users of this API independent of the current
  * SMIF IP version, it accepts any SMIF pointer as parameter, e.g. a SMIF base
  * pointer or a SMIF_DEVICE base pointer. This means this API can be called with
  * a SMIF_DEVICE base pointer and the calling code will work with CY_SMIF_NEW_VERSION
  * or CY_SMIF_OLD_VERSION
  *
  * \param smif_or_device_base
  * Holds the base address of the SMIF block or SMIF_DEVICE block registers.
  *
  * \return read tap selection
  *
  *******************************************************************************/
  __STATIC_INLINE uint8_t Cy_SMIF_Get_DelayTapSel(volatile void *smif_or_device_base)
  {
#ifdef CY_SMIF_NEW_VERSION
    return (uint8_t)(((stc_SMIF_DEVICE_t*)smif_or_device_base)->unDELAY_TAP_SEL.stcField.u8SEL);
#else
    // Ensure that if a device pointer has been passed, it is converted to a SMIF base pointer
    volatile stc_SMIF_t * base = Cy_SMIF_GetSmifBasePtrFromAnySmifPtr(smif_or_device_base);
    
    return (uint8_t)(base->unDELAY_TAP_SEL.stcField.u8SEL);
#endif
  }
  
  // #if defined (tviic2d6m) || defined (tviic2d4m)
  // /*******************************************************************************
  // * Function Name: Cy_SMIF_Set_TXDataDelayTapSel
  // ****************************************************************************//**
  // *
  // *
  // * \param base
  // * Holds the base address of the SMIF block registers.
  // *
  // * \param tapSed
  // * TX delay tap selection to be set
  // *
  // * \return status (see \ref cy_en_smif_status_t).
  // *
  // *******************************************************************************/
  // __STATIC_INLINE cy_en_smif_status_t Cy_SMIF_Set_TXDataDelayTapSel(volatile stc_SMIF_t *base, uint8_t tapSel)
  // {
  // base->unTX_DATA_DELAY_TAP_SEL.stcField.u8SEL = tapSel;
  // return CY_SMIF_SUCCESS;
  // }
  
  // /*******************************************************************************
  // * Function Name: Cy_SMIF_Get_TXDataDelayTapSel
  // ****************************************************************************//**
  // *
  // *
  // * \param base
  // * Holds the base address of the SMIF block registers.
  // *
  // * \return read TX delay tap selection
  // *
  // *******************************************************************************/
  // __STATIC_INLINE uint8_t Cy_SMIF_Get_TXDataDelayTapSel(volatile stc_SMIF_t *base)
  // {
  // return (uint8_t)(base->unTX_DATA_DELAY_TAP_SEL.stcField.u8SEL);
  // }
  // #endif
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_DeviceSetDataSelect
  ****************************************************************************//**
  *
  * This function set the connection of the
  * IP's data lines (spi_data[0], c, spi_data[7]) to the
  * device's data lines (SI/IO0, SO/IO1, IO2, IO3, IO4, IO5, IO6, IO7)
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  * \param sel (see \cy_en_smif_data_select_t)
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_DeviceSetDataSelect(volatile stc_SMIF_DEVICE_t *device, cy_en_smif_data_select_t sel)
  {
    device->unCTL.stcField.u2DATA_SEL  = sel;
    
    /* The void Read of the CTL register to handle buffering */
    (void)device->unCTL.u32Register;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_DeviceEnable
  ****************************************************************************//**
  *
  * This function enables input SMIF Device.
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_DeviceEnable(volatile stc_SMIF_DEVICE_t *device)
  {
    device->unCTL.stcField.u1ENABLED = 1;
    
    /* The void Read of the CTL register to handle buffering */
    (void)device->unCTL.u32Register;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_DeviceDisable
  ****************************************************************************//**
  *
  * This function disables input SMIF Device.
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_DeviceDisable(volatile stc_SMIF_DEVICE_t *device)
  {
    device->unCTL.stcField.u1ENABLED = 0;
    
    /* The void Read of the CTL register to handle buffering */
    (void)device->unCTL.u32Register;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_DeviceWriteEnable
  ****************************************************************************//**
  *
  * This function enables writing for input SMIF Device.
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_DeviceWriteEnable(volatile stc_SMIF_DEVICE_t *device)
  {
    device->unCTL.stcField.u1WR_EN = 1;
    
    /* The void Read of the CTL register to handle buffering */
    (void)device->unCTL.u32Register;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_DeviceWriteDisable
  ****************************************************************************//**
  *
  * This function disables writing for input SMIF Device.
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_DeviceWriteDisable(volatile stc_SMIF_DEVICE_t *device)
  {
    device->unCTL.stcField.u1WR_EN = 0;
    
    /* The void Read of the CTL register to handle buffering */
    (void)device->unCTL.u32Register;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_DeviceCryptoEnable
  ****************************************************************************//**
  *
  * This function enables on the fly cryptography for input SMIF Device.
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_DeviceCryptoEnable(volatile stc_SMIF_DEVICE_t *device)
  {
    device->unCTL.stcField.u1CRYPTO_EN = 1;
    
    /* The void Read of the CTL register to handle buffering */
    (void)device->unCTL.u32Register;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_DeviceCryptoDisable
  ****************************************************************************//**
  *
  * This function disables on the fly cryptography for input SMIF Device.
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_DeviceCryptoDisable(volatile stc_SMIF_DEVICE_t *device)
  {
    device->unCTL.stcField.u1CRYPTO_EN = 0;
    
    /* The void Read of the CTL register to handle buffering */
    (void)device->unCTL.u32Register;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_DeviceTransferMergingEnable
  ****************************************************************************//**
  *
  * This function enables transfer merging, and sets merge timeout.
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  * \param timeout (see \cy_en_smif_dev_merge_timeout_t)
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_DeviceTransferMergingEnable(volatile stc_SMIF_DEVICE_t *device, cy_en_smif_dev_merge_timeout_t timeout)
  {
    un_SMIF_DEVICE_CTL_t temp;
    
    temp.u32Register              = device->unCTL.u32Register;
    temp.stcField.u3MERGE_TIMEOUT = timeout;
    temp.stcField.u1MERGE_EN      = 1u;
    device->unCTL.u32Register     = temp.u32Register;
    
    /* The void Read of the CTL register to handle buffering */
    (void)device->unCTL.u32Register;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_DeviceTransferMergingDisable
  ****************************************************************************//**
  *
  * This function disables transfer merging.
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_DeviceTransferMergingDisable(volatile stc_SMIF_DEVICE_t *device)
  {
    device->unCTL.stcField.u1MERGE_EN = 0u;
    
    /* The void Read of the CTL register to handle buffering */
    (void)device->unCTL.u32Register;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_DeviceTotalTimeoutEnable
  ****************************************************************************//**
  *
  * This function enables total timeout of input device, and sets total timeout
  * value.
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  * \param timeout
  * Timeout clock number
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_DeviceTotalTimeoutEnable(volatile stc_SMIF_DEVICE_t *device, uint16_t timeout)
  {
    un_SMIF_DEVICE_CTL_t temp;
    
    temp.u32Register                 = device->unCTL.u32Register;
    temp.stcField.u14TOTAL_TIMEOUT   = (timeout & 0x3FFF);
    temp.stcField.u1TOTAL_TIMEOUT_EN = 1u;
    device->unCTL.u32Register        = temp.u32Register;
    
    /* The void Read of the CTL register to handle buffering */
    (void)device->unCTL.u32Register;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_DeviceTotalTimeoutDisable
  ****************************************************************************//**
  *
  * This function disables total timeout of input device.
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_DeviceTotalTimeoutDisable(volatile stc_SMIF_DEVICE_t *device)
  {
    device->unCTL.stcField.u1TOTAL_TIMEOUT_EN = 0u;
    
    /* The void Read of the CTL register to handle buffering */
    (void)device->unCTL.u32Register;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_GetReadStatus
  ****************************************************************************//**
  *
  * This function returns functional safety status register.
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  * \return Functional Safety Status Register of the memory 
  *         received with the last read transfer.
  *
  *******************************************************************************/
  __STATIC_INLINE uint8_t Cy_SMIF_GetReadStatus(volatile stc_SMIF_DEVICE_t *device)
  {
    return (uint8_t)(device->unRD_STATUS.stcField.u8FS_STATUS);
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_SetXIPBaseAddress
  ****************************************************************************//**
  *
  * This function sets base address in XIP mode
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  * \param addr
  * Base address which will be used in XIP mode
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_SetXIPBaseAddress(volatile stc_SMIF_DEVICE_t *device, uint32_t addr)
  {
    device->unADDR.u32Register = (addr & SMIF_DEVICE_ADDR_ADDR_Msk);
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_SetXIPAddressSize
  ****************************************************************************//**
  *
  * This function sets size of address in XIP mode
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  * \param size
  * Address size which will be used in XIP mode (see \cy_en_device_size_t).
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_SetXIPAddressSize(volatile stc_SMIF_DEVICE_t *device, cy_en_device_size_t size)
  {
    device->unMASK.u32Register = (uint32_t)size;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_SetXIPAddrCtl
  ****************************************************************************//**
  *
  * This function sets the size of the XIP device address and if the AHB-Lite
  * bus transfer address is divided by 2.
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  * \param addrSize
  * (see \cy_en_smif_xip_addr_byte_t).
  *
  * \param divForDualQuad
  * true: dual quad mode setting
  * faluse: not dual quad mode setting
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_SetXIPAddrCtl(volatile stc_SMIF_DEVICE_t *device, cy_en_smif_xip_addr_byte_t addrSize, bool divForDualQuad)
  {
    un_SMIF_DEVICE_ADDR_CTL_t tmp_ADDR_CTL = { .u32Register = 0x00000000ul };
    tmp_ADDR_CTL.stcField.u3SIZE3  = addrSize;
    tmp_ADDR_CTL.stcField.u1DIV2   = divForDualQuad;
    device->unADDR_CTL.u32Register = tmp_ADDR_CTL.u32Register;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_SetReadCmdCtl
  ****************************************************************************//**
  *
  * This function controls read command in XIP mode.
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  * \param config
  * (see \stc_cy_smif_cmd_mode_ctl_t).
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_SetReadCmdCtl(volatile stc_SMIF_DEVICE_t *device, const stc_cy_smif_cmd_mode_ctl_t* config)
  {
    un_SMIF_DEVICE_RD_CMD_CTL_t tmp_RD_CMD_CTL = { .u32Register = 0x00000000ul };
    tmp_RD_CMD_CTL.au16Halfword[0]     = config->cmd;
    tmp_RD_CMD_CTL.stcField.u2WIDTH    = config->width;
    tmp_RD_CMD_CTL.stcField.u1DDR_MODE = config->dataRate;
    tmp_RD_CMD_CTL.stcField.u2PRESENT2 = config->presence;
    device->unRD_CMD_CTL.u32Register   = tmp_RD_CMD_CTL.u32Register;
    return;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_SetWriteCmdCtl
  ****************************************************************************//**
  *
  * This function controls write command in XIP mode.
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  * \param config
  * (see \stc_cy_smif_cmd_mode_ctl_t).
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_SetWriteCmdCtl(volatile stc_SMIF_DEVICE_t *device, const stc_cy_smif_cmd_mode_ctl_t* config)
  {
    un_SMIF_DEVICE_WR_CMD_CTL_t tmp_WR_CMD_CTL = { .u32Register = 0x00000000ul };
    tmp_WR_CMD_CTL.au16Halfword[0]     = config->cmd;
    tmp_WR_CMD_CTL.stcField.u2WIDTH    = config->width;
    tmp_WR_CMD_CTL.stcField.u1DDR_MODE = config->dataRate;
    tmp_WR_CMD_CTL.stcField.u2PRESENT2 = config->presence;
    device->unWR_CMD_CTL.u32Register   = tmp_WR_CMD_CTL.u32Register;
    return;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_SetReadAddrCtl
  ****************************************************************************//**
  *
  * This function controls read address in XIP mode
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  * \param config
  * (see \stc_cy_smif_addr_data_ctl_t).
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_SetReadAddrCtl(volatile stc_SMIF_DEVICE_t *device, const stc_cy_smif_addr_data_ctl_t* config)
  {
    un_SMIF_DEVICE_RD_ADDR_CTL_t tmp_RD_ADDR_CTL = { .u32Register = 0x00000000ul };
    tmp_RD_ADDR_CTL.stcField.u2WIDTH    = config->width;
    tmp_RD_ADDR_CTL.stcField.u1DDR_MODE = config->dataRate;
    device->unRD_ADDR_CTL.u32Register   = tmp_RD_ADDR_CTL.u32Register;
    return;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_SetWriteAddrCtl
  ****************************************************************************//**
  *
  * This function controls write address in XIP mode.
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  * \param config
  * (see \stc_cy_smif_addr_data_ctl_t).
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_SetWriteAddrCtl(volatile stc_SMIF_DEVICE_t *device, const stc_cy_smif_addr_data_ctl_t* config)
  {
    un_SMIF_DEVICE_WR_ADDR_CTL_t tmp_WR_ADDR_CTL = { .u32Register = 0x00000000ul };
    
    tmp_WR_ADDR_CTL.stcField.u2WIDTH    = config->width;
    tmp_WR_ADDR_CTL.stcField.u1DDR_MODE = config->dataRate;
    device->unWR_ADDR_CTL.u32Register   = tmp_WR_ADDR_CTL.u32Register;
    return;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_SetReadModeCtl
  ****************************************************************************//**
  *
  * This function controls read mode in XIP mode
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  * \param config
  * (see \stc_cy_smif_cmd_mode_ctl_t).
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_SetReadModeCtl(volatile stc_SMIF_DEVICE_t *device, const stc_cy_smif_cmd_mode_ctl_t* config)
  {
    un_SMIF_DEVICE_RD_MODE_CTL_t tmpRD_MODE_CTL = { .u32Register = 0x00000000ul };
    
    tmpRD_MODE_CTL.au16Halfword[0]     = config->cmd;
    tmpRD_MODE_CTL.stcField.u2WIDTH    = config->width;
    tmpRD_MODE_CTL.stcField.u1DDR_MODE = config->dataRate;
    tmpRD_MODE_CTL.stcField.u2PRESENT2 = config->presence;
    device->unRD_MODE_CTL.u32Register  = tmpRD_MODE_CTL.u32Register;
    return;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_SetWriteModeCtl
  ****************************************************************************//**
  *
  * This function controls write mode in XIP mode
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  * \param config
  * (see \stc_cy_smif_cmd_mode_ctl_t).
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_SetWriteModeCtl(volatile stc_SMIF_DEVICE_t *device, const stc_cy_smif_cmd_mode_ctl_t* config)
  {
    un_SMIF_DEVICE_WR_MODE_CTL_t tmp_WR_MODE_CTL = { .u32Register = 0x00000000ul };
    
    tmp_WR_MODE_CTL.au16Halfword[0]     = config->cmd;
    tmp_WR_MODE_CTL.stcField.u2WIDTH    = config->width;
    tmp_WR_MODE_CTL.stcField.u1DDR_MODE = config->dataRate;
    tmp_WR_MODE_CTL.stcField.u2PRESENT2 = config->presence;
    device->unWR_MODE_CTL.u32Register   = tmp_WR_MODE_CTL.u32Register;
    return;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_SetReadDummyCtl
  ****************************************************************************//**
  *
  * This function controls read dummy in XIP mode
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  * \param config
  * (see \stc_cy_smif_rd_dummy_ctl_t).
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_SetReadDummyCtl(volatile stc_SMIF_DEVICE_t *device, const stc_cy_smif_rd_dummy_ctl_t *config)
  {
    un_SMIF_DEVICE_RD_DUMMY_CTL_t tmpRD_DUMMY_CTL = { .u32Register = 0x00000000ul };
    
    if(config->numOfDummy == 0)
    {
      tmpRD_DUMMY_CTL.stcField.u5SIZE5    = 0;
      tmpRD_DUMMY_CTL.stcField.u2PRESENT2 = CY_SMIF_NOT_PRESENT;
    }
    else
    {
      tmpRD_DUMMY_CTL.stcField.u5SIZE5    = config->numOfDummy - 1u;
      tmpRD_DUMMY_CTL.stcField.u2PRESENT2 = config->presence;
    }
    
    device->unRD_DUMMY_CTL.u32Register  = tmpRD_DUMMY_CTL.u32Register;
    return;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_SetWriteDummyCtl
  ****************************************************************************//**
  *
  * This function controls write dummy in XIP mode.
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  * \param config
  * (see \stc_cy_smif_wr_dummy_ctl_t).
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_SetWriteDummyCtl(volatile stc_SMIF_DEVICE_t *device, const stc_cy_smif_wr_dummy_ctl_t* config)
  {
    un_SMIF_DEVICE_WR_DUMMY_CTL_t tmpWR_DUMMY_CTL = { .u32Register = 0x00000000ul };
    
    tmpWR_DUMMY_CTL.stcField.u1RWDS_EN  = config->rwdsEn;
    if(config->numOfDummy == 0)
    {
      tmpWR_DUMMY_CTL.stcField.u5SIZE5    = 0u;
      tmpWR_DUMMY_CTL.stcField.u2PRESENT2 = CY_SMIF_NOT_PRESENT;
    }
    else
    {
      tmpWR_DUMMY_CTL.stcField.u5SIZE5    = config->numOfDummy - 1ul;
      tmpWR_DUMMY_CTL.stcField.u2PRESENT2 = config->presence;
    }
    
    device->unWR_DUMMY_CTL.u32Register   = tmpWR_DUMMY_CTL.u32Register;
    return;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_SetReadDataCtl
  ****************************************************************************//**
  *
  * This function controls read data in XIP mode.
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  * \param config
  * (see \stc_cy_smif_addr_data_ctl_t).
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_SetReadDataCtl(volatile stc_SMIF_DEVICE_t *device, const stc_cy_smif_addr_data_ctl_t* config)
  {
    un_SMIF_DEVICE_RD_DATA_CTL_t tmpRD_DATA_CTL = { .u32Register = 0x00000000ul };
    
    tmpRD_DATA_CTL.stcField.u2WIDTH    = config->width;
    tmpRD_DATA_CTL.stcField.u1DDR_MODE = config->dataRate;
    device->unRD_DATA_CTL.u32Register  = tmpRD_DATA_CTL.u32Register;
    return;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_SetWriteDataCtl
  ****************************************************************************//**
  *
  * This function controls write data in XIP mode.
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  * \param config
  * (see \stc_cy_smif_addr_data_ctl_t).
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_SetWriteDataCtl(volatile stc_SMIF_DEVICE_t *device, const stc_cy_smif_addr_data_ctl_t* config)
  {
    un_SMIF_DEVICE_WR_DATA_CTL_t tmpWR_ADDR_CTL = { .u32Register = 0x00000000ul };
    
    tmpWR_ADDR_CTL.stcField.u2WIDTH    = config->width;
    tmpWR_ADDR_CTL.stcField.u1DDR_MODE = config->dataRate;
    device->unWR_DATA_CTL.u32Register   = tmpWR_ADDR_CTL.u32Register;
    return;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_SetReadCrcCtl
  ****************************************************************************//**
  *
  * This function controls read CRC in XIP mode.
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  * \param config
  * (see \stc_cy_smif_rd_crc_ctl_t).
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_SetReadCrcCtl(volatile stc_SMIF_DEVICE_t *device, const stc_cy_smif_rd_crc_ctl_t *config)
  {
    un_SMIF_DEVICE_RD_CRC_CTL_t tmp_RD_CRC_CTL = { .u32Register = 0x00000000ul };
    tmp_RD_CRC_CTL.stcField.u8STATUS_CHECK_MASK     = config->funcSafeChecked;
    tmp_RD_CRC_CTL.stcField.u8STATUS_ERROR_POL      = config->errPol;
    tmp_RD_CRC_CTL.stcField.u8DATA_CRC_INPUT_SIZE   = config->inputSize;
    tmp_RD_CRC_CTL.stcField.u2CMD_ADDR_CRC_WIDTH    = config->width;
    tmp_RD_CRC_CTL.stcField.u1CMD_ADDR_CRC_DDR_MODE = config->dataRate;
    tmp_RD_CRC_CTL.stcField.u1CMD_ADDR_CRC_INPUT    = config->inputScope;
    tmp_RD_CRC_CTL.stcField.u1DATA_CRC_CHECK        = config->crcChecked;
    tmp_RD_CRC_CTL.stcField.u1CMD_ADDR_CRC_PRESENT  = config->cmd_addrCrcPresent;
    tmp_RD_CRC_CTL.stcField.u1DATA_CRC_PRESENT      = config->dataCrcPresent;
    device->unRD_CRC_CTL.u32Register = tmp_RD_CRC_CTL.u32Register;
    return;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_SetWriteCrcCtl
  ****************************************************************************//**
  *
  * This function controls write CRC in XIP mode.
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  * \param config
  * (see \stc_cy_smif_wr_crc_ctl_t).
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_SetWriteCrcCtl(volatile stc_SMIF_DEVICE_t *device, const stc_cy_smif_wr_crc_ctl_t *config)
  {
    un_SMIF_DEVICE_WR_CRC_CTL_t tmp_WR_CRC_CTL      = { .u32Register = 0x00000000ul };
    tmp_WR_CRC_CTL.stcField.u8DATA_CRC_INPUT_SIZE   = config->inputSize;
    tmp_WR_CRC_CTL.stcField.u2CMD_ADDR_CRC_WIDTH    = config->width;
    tmp_WR_CRC_CTL.stcField.u1CMD_ADDR_CRC_DDR_MODE = config->dataRate;
    tmp_WR_CRC_CTL.stcField.u1CMD_ADDR_CRC_INPUT    = config->inputScope;
    tmp_WR_CRC_CTL.stcField.u1CMD_ADDR_CRC_PRESENT  = config->cmd_addrCrcPresent;
    tmp_WR_CRC_CTL.stcField.u1DATA_CRC_PRESENT      = config->dataCrcPresent;
    device->unWR_CRC_CTL.u32Register = tmp_WR_CRC_CTL.u32Register;
    return;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_SetReadBoundCtl
  ****************************************************************************//**
  *
  * This function controls read bound in XIP mode.
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  * \param config
  * (see \stc_cy_smif_rd_bound_ctl_t).
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_SetReadBoundCtl(volatile stc_SMIF_DEVICE_t *device, const stc_cy_smif_rd_bound_ctl_t *config)
  {
    un_SMIF_DEVICE_RD_BOUND_CTL_t tmp_RD_BOUND_CTL  = { .u32Register = 0x00000000ul };
    if(config->numOfCycle == 0)
    {
      tmp_RD_BOUND_CTL.stcField.u5SIZE5           = 0;
      tmp_RD_BOUND_CTL.stcField.u1PRESENT         = false;
    }
    else
    {
      tmp_RD_BOUND_CTL.stcField.u5SIZE5           = config->numOfCycle - 1;
      tmp_RD_BOUND_CTL.stcField.u1PRESENT         = config->bdLPresent;
    }
    tmp_RD_BOUND_CTL.stcField.u2SUB_PAGE_SIZE   = config->subPageSize;
    tmp_RD_BOUND_CTL.stcField.u2SUB_PAGE_NR     = config->subPageNr;
    tmp_RD_BOUND_CTL.stcField.u1SUBSEQ_BOUND_EN = config->subsqBndEn;
    
    device->unRD_BOUND_CTL.u32Register = tmp_RD_BOUND_CTL.u32Register;
    return;
  }
  
  /*******************************************************************************
  * Function Name:  Cy_SMIF_SetCryptoInput128
  ****************************************************************************//**
  *
  * This function sets 128 bit data of crypto input (plain text).
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  * \param inData Address of input 16 byte input data to be crypted
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_SetCryptoInput128(volatile stc_SMIF_t *base, const uint8_t inData[])
  {
    base->unCRYPTO_INPUT0.u32Register = *(uint32_t*)&inData[0u];
    base->unCRYPTO_INPUT1.u32Register = *(uint32_t*)&inData[4u];
    base->unCRYPTO_INPUT2.u32Register = *(uint32_t*)&inData[8u];
    base->unCRYPTO_INPUT3.u32Register = *(uint32_t*)&inData[12u];
  }
  
  /*******************************************************************************
  * Function Name:  Cy_SMIF_SetCryptoKey128
  ****************************************************************************//**
  *
  * This function sets 128 bit data of crypto key.
  *
  * \param device
  * Holds the base address of the SMIF Device registers.
  *
  * \param key Address of 16 byte AES key data
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_SetCryptoKey128(volatile stc_SMIF_t *base, const uint8_t key[])
  {
    base->unCRYPTO_KEY0.u32Register = *(uint32_t*)&key[0u];
    base->unCRYPTO_KEY1.u32Register = *(uint32_t*)&key[4u];
    base->unCRYPTO_KEY2.u32Register = *(uint32_t*)&key[8u];
    base->unCRYPTO_KEY3.u32Register = *(uint32_t*)&key[12u];
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_Disable
  ****************************************************************************//**
  *
  * This function disables SMIF.
  *
  * Disables the operation of the SMIF block. The SMIF block can be disabled only
  * when it is not in the active state. Use the Cy_SMIF_IsBusy() API to check
  * it before calling this API.
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_Disable(volatile stc_SMIF_t *base)
  {
    base->unCTL.stcField.u1ENABLED = 0u;
    
    /* The void Read of the CTL register to handle buffering */
    (void) base->unCTL.u32Register;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_SetInterruptMask
  ****************************************************************************//**
  *
  * This function is used to set an interrupt mask for the SMIF Interrupt.
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  * \param interrupt
  * This is the mask for different source options that can be masked. See
  * \ref group_smif_macros_isr "Interrupt Macros" for possible values.
  *
  *******************************************************************************/
  __STATIC_INLINE void  Cy_SMIF_SetInterruptMask(volatile stc_SMIF_t *base, uint32_t interrupt)
  {
    base->unINTR_MASK.u32Register = interrupt;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_GetInterruptMask
  ****************************************************************************//**
  *
  * This function is used to read an interrupt mask for the SMIF Interrupt.
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  * \return Returns the mask set for the SMIF interrupt.
  *
  *******************************************************************************/
  __STATIC_INLINE uint32_t  Cy_SMIF_GetInterruptMask(volatile stc_SMIF_t *base)
  {
    return (base->unINTR_MASK.u32Register);
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_GetInterruptStatusMasked
  ****************************************************************************//**
  *
  * This function is used to read an active masked interrupt. This function can
  * be used in the interrupt service-routine to find which source triggered the
  * interrupt.
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  * \return Returns a word with bits set at positions corresponding to the
  * interrupts triggered in the system.
  *
  *******************************************************************************/
  __STATIC_INLINE uint32_t  Cy_SMIF_GetInterruptStatusMasked(volatile stc_SMIF_t *base)
  {
    return (base->unINTR_MASKED.u32Register);
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_GetInterruptStatus
  ****************************************************************************//**
  *
  * This function is used to read an active interrupt. This status is the unmasked
  * result, so will also show interrupts that will not generate active interrupts.
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  * \return Returns a word with bits set at positions corresponding to the
  * interrupts triggered in the system.
  *
  *******************************************************************************/
  __STATIC_INLINE uint32_t  Cy_SMIF_GetInterruptStatus(volatile stc_SMIF_t *base)
  {
    return (base->unINTR.u32Register);
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_SetInterrupt
  ****************************************************************************//**
  *
  * This function is used to set an interrupt source. This function can be used
  * to activate interrupts through the software.
  *
  * \note Interrupt sources set using this interrupt will generate interrupts only
  * if they are not masked.
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  * \param interrupt
  * An encoded integer with a bit set corresponding to the interrupt to be
  * triggered. See \ref group_smif_macros_isr "Interrupt Macros" for possible
  * values.
  *
  *******************************************************************************/
  __STATIC_INLINE void  Cy_SMIF_SetInterrupt(volatile stc_SMIF_t *base, uint32_t interrupt)
  {
    base->unINTR_SET.u32Register = interrupt;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_ClearInterrupt
  ****************************************************************************//**
  *
  * This function is used to clear an interrupt source. This function can be used
  * in the user code to clear all pending interrupts.
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  * \param interrupt
  * An encoded integer with a bit set corresponding to the interrupt that must
  * be cleared. See \ref group_smif_macros_isr "Interrupt Macros" for possible
  * values.
  *
  *******************************************************************************/
  __STATIC_INLINE void  Cy_SMIF_ClearInterrupt(volatile stc_SMIF_t *base, uint32_t interrupt)
  {
    base->unINTR.u32Register = interrupt;
    /* Ensure that the initial Write has been flushed out to the hardware. */
    interrupt = base->unINTR.u32Register;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_SetTxFifoTriggerLevel()
  ****************************************************************************//**
  *
  * This function is used to set a trigger level for the TX FIFO. This value must
  * be an integer between 0 and 7. For the normal mode only.
  * The triggering is active when TX_DATA_FIFO_STATUS <= level.
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  * \param level
  * The trigger level to set (0-8).
  *
  *******************************************************************************/
  __STATIC_INLINE void  Cy_SMIF_SetTxFifoTriggerLevel(volatile stc_SMIF_t *base, uint32_t level)
  {
    CY_ASSERT_L2(level < CY_SMIF_MAX_TX_TR_LEVEL);
    base->unTX_DATA_FIFO_CTL.u32Register = level;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_SetRxFifoTriggerLevel()
  ****************************************************************************//**
  *
  * This function is used to set a trigger level for the RX FIFO. This value must
  * be an integer between 0 and 7. For the normal mode only.
  * The triggering is active when RX_DATA_FIFOSTATUS > level.
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  * \param level
  * The trigger level to set(0-8).
  *
  *******************************************************************************/
  __STATIC_INLINE void  Cy_SMIF_SetRxFifoTriggerLevel(volatile stc_SMIF_t *base, uint32_t level)
  {
    CY_ASSERT_L2(level < CY_SMIF_MAX_RX_TR_LEVEL);
    base->unRX_DATA_MMIO_FIFO_CTL.stcField.u3RX_TRIGGER_LEVEL = level;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_GetCmdFifoStatus()
  ****************************************************************************//**
  *
  * This function is used to read the status of the CMD FIFO.
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  * \return Returns the number of the entries in the CMD FIFO.
  *
  *******************************************************************************/
  __STATIC_INLINE uint32_t  Cy_SMIF_GetCmdFifoStatus(volatile stc_SMIF_t *base)
  {
    return (base->unTX_CMD_FIFO_STATUS.stcField.u4USED4);
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_GetTxFifoStatus()
  ****************************************************************************//**
  *
  * This function is used to read the status of the TX FIFO.
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  * \return Returns the number of the entries in the TX FIFO.
  *
  *******************************************************************************/
  __STATIC_INLINE uint32_t  Cy_SMIF_GetTxFifoStatus(volatile stc_SMIF_t *base)
  {
    return (base->unTX_DATA_FIFO_STATUS.stcField.u4USED4);
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_GetRxFifoStatus()
  ****************************************************************************//**
  *
  * This function is used to read the status of the RX FIFO.
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  * \return Returns the number of the entries in the RX FIFO.
  *
  *******************************************************************************/
  __STATIC_INLINE uint32_t  Cy_SMIF_GetRxFifoStatus(volatile stc_SMIF_t *base)
  {
    return (base->unRX_DATA_MMIO_FIFO_STATUS.stcField.u4USED4);
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_IsBusy
  ****************************************************************************//**
  *
  * This function provides the status of the IP block (False - not busy,
  * True - busy).
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  * \return Returns an IP block status.
  *
  *******************************************************************************/
  __STATIC_INLINE bool Cy_SMIF_IsBusy(volatile stc_SMIF_t *base)
  {
    return (bool)(base->unSTATUS.stcField.u1BUSY);
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_Interrupt
  ****************************************************************************//**
  *
  * The Interrupt Service Routine for the SMIF. The interrupt code will be
  * responsible for the FIFO operations on FIFO interrupts during ongoing transfers.
  * The user must place a call to this interrupt function in the interrupt
  * routine corresponding to the interrupt attached to the SMIF. If the
  * user does not do this, will break: the functionality of all the API functions in
  * the SMIF driver that use SMIF interrupts to affect transfers.
  *
  * \param base
  * Holds the base address of the SMIF block registers.
  *
  * \param context
  * Passes a configuration structure that contains the transfer parameters of the
  * SMIF block.
  *
  * \globalvars
  *  - context->txBufferAddress - The pointer to the data to be transferred.
  *
  *  - context->txBufferCounter - The number of data entries left to be transferred.
  *
  * All the Global variables described above are used when the Software Buffer is
  * used.
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_Interrupt(volatile stc_SMIF_t *base, cy_stc_smif_context_t *context)
  {
    uint32_t interruptStatus = Cy_SMIF_GetInterruptStatusMasked(base);
    
    /* Check which interrupt occurred */
    if ((interruptStatus & SMIF_INTR_TR_TX_REQ_Msk) != 0ul)
    {
      /* Send data */
      Cy_SMIF_PushTxFifo(base, context);
      
      Cy_SMIF_ClearInterrupt(base, SMIF_INTR_TR_TX_REQ_Msk);
    }
    else if ((interruptStatus & SMIF_INTR_TR_RX_REQ_Msk) != 0ul)
    {
      /* Receive data */
      Cy_SMIF_PopRxFifo(base, context);
      
      Cy_SMIF_ClearInterrupt(base, SMIF_INTR_TR_RX_REQ_Msk);
    }
    else if ((interruptStatus & SMIF_INTR_XIP_ALIGNMENT_ERROR_Msk) != 0ul)
    {
      /* An XIP alignment error */
      context->transferStatus = CY_SMIF_XIP_ERROR;
      
      Cy_SMIF_ClearInterrupt(base, SMIF_INTR_XIP_ALIGNMENT_ERROR_Msk);
    }
    else if ((interruptStatus & SMIF_INTR_TX_CMD_FIFO_OVERFLOW_Msk) != 0ul)
    {
      /* TX CMD FIFO overflow */
      context->transferStatus = CY_SMIF_CMD_ERROR;
      
      Cy_SMIF_ClearInterrupt(base, SMIF_INTR_TX_CMD_FIFO_OVERFLOW_Msk);
    }
    else if ((interruptStatus & SMIF_INTR_TX_DATA_FIFO_OVERFLOW_Msk) != 0ul)
    {
      /* A TX DATA FIFO overflow */
      context->transferStatus = CY_SMIF_TX_ERROR;
      
      Cy_SMIF_ClearInterrupt(base, SMIF_INTR_TX_DATA_FIFO_OVERFLOW_Msk);
    }
    else if ((interruptStatus & SMIF_INTR_RX_DATA_MMIO_FIFO_UNDERFLOW_Msk) != 0ul)
    {
      /* RX DATA FIFO underflow */
      context->transferStatus = CY_SMIF_RX_ERROR;
      
      Cy_SMIF_ClearInterrupt(base, SMIF_INTR_RX_DATA_MMIO_FIFO_UNDERFLOW_Msk);
    }
    else
    {
      /* Processing of errors */
    }
  }
  
  /***************************************
  *  Internal SMIF in-line functions
  ****************************************/
  
  /** \cond INTERNAL */
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_PushTxFifo
  ***************************************************************************//***
  *
  * \internal
  *
  * \param baseaddr
  * Holds the base address of the SMIF block registers.
  *
  * \param context
  * Passes a configuration structure that contains the transfer parameters of the
  * SMIF block.
  *
  * This function writes data in the TX FIFO SMIF buffer by 4, 2, or 1 bytes based
  * on the residual number of bytes and the available space in the TX FIFO.
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_PushTxFifo(volatile stc_SMIF_t *baseaddr, cy_stc_smif_context_t *context)
  {
    /* The variable that shows which is smaller: the free FIFO size or amount of bytes to be sent */
    uint32_t freeFifoBytes = CY_SMIF_TX_DATA_FIFO_STATUS_RANGE - Cy_SMIF_GetTxFifoStatus(baseaddr);;
    uint32_t buffCounter   = context->txBufferCounter;
    
    // writeBytes = MIN(freeFifoBytes, buffCounter);
    uint32_t writeBytes    = (freeFifoBytes > buffCounter)? buffCounter: freeFifoBytes;
    
    // If it sends 2 byte per one clock, then it needs using TX_DATA_FIFO_WR1ODD when sending one byte.
    // Save proper one byte sending fifo address into ptr1ByteFifo accordingly.
    uint32_t* ptr1ByteFifo;
    if((context->preCmdDataRate == CY_SMIF_DDR) &&(context->preCmdWidth == CY_SMIF_WIDTH_OCTAL))
    {
      ptr1ByteFifo = (uint32_t*)&(baseaddr->unTX_DATA_FIFO_WR1ODD);
    }
    else
    {
      ptr1ByteFifo = (uint32_t*)&(baseaddr->unTX_DATA_FIFO_WR1);
    }
    
    /* Check that after a FIFO Write, no data/FIFO space remains */
    uint8_t *buff = (uint8_t*)context->txBufferAddress;
    while (writeBytes != 0U)
    {
      // Even if writeBytes == 1, do not send it. Wait more space becoming available.
      // Because 1 byte fifo (TX_DATA_FIFO_WR1) should be used only last 1 byte.
      // If bufferCounter == 1, it means it is the last byte. then send it.
      if((writeBytes >= 2U) || (buffCounter == 1))
      {
        if(writeBytes == 1)
        {
          // Enter here only when this is the last sending.
          *ptr1ByteFifo = buff[0U];
        }
        else if((writeBytes == 2u) || (writeBytes == 3u))
        {
          baseaddr->unTX_DATA_FIFO_WR2.u32Register = Cy_SMIF_PackBytesArray(&buff[0U], false);
          writeBytes = 2u;
        }
        else if((writeBytes == 4u) || (writeBytes == 5u))
        {
          baseaddr->unTX_DATA_FIFO_WR4.u32Register = Cy_SMIF_PackBytesArray(&buff[0U], true);
          writeBytes = 4u;
        }
        else if((writeBytes == 6u) || (writeBytes == 7u))
        {
          baseaddr->unTX_DATA_FIFO_WR4.u32Register = Cy_SMIF_PackBytesArray(&buff[0U], true);
          baseaddr->unTX_DATA_FIFO_WR2.u32Register = Cy_SMIF_PackBytesArray(&buff[4U], false);
          writeBytes = 6u;
        }
        else /* writeBytes => 8*/
        {
          baseaddr->unTX_DATA_FIFO_WR4.u32Register = Cy_SMIF_PackBytesArray(&buff[0U], true);
          baseaddr->unTX_DATA_FIFO_WR4.u32Register = Cy_SMIF_PackBytesArray(&buff[4U], true);
          writeBytes = 8u;
        }
        buff          = &buff[writeBytes];
        buffCounter  -= writeBytes;
      }
      
      /* Check if we already got new data in TX_FIFO*/
      freeFifoBytes = CY_SMIF_TX_DATA_FIFO_STATUS_RANGE - Cy_SMIF_GetTxFifoStatus(baseaddr);
      writeBytes    = (freeFifoBytes > buffCounter)? buffCounter: freeFifoBytes;
    }
    
    /* Save changes in the context */
    context->txBufferAddress = buff;
    context->txBufferCounter = buffCounter;
    
    /* Check if all bytes are sent */
    if (buffCounter == 0u)
    {
      /* Disable the TR_TX_REQ interrupt */
      baseaddr->unINTR_MASK.stcField.u1TR_TX_REQ = 0u;
      
      context->transferStatus = CY_SMIF_SEND_CMPLT;
      if (NULL != context->txCmpltCb)
      {
        context->txCmpltCb((uint32_t) CY_SMIF_SEND_CMPLT);
      }
    }
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_PopRxFifo
  ***************************************************************************//***
  *
  * \internal
  *
  * \param baseaddr
  * Holds the base address of the SMIF block registers.
  *
  * \param context
  * Passes a configuration structure that contains the transfer parameters of the
  * SMIF block.
  *
  * This function reads data from the RX FIFO SMIF buffer by 4, 2, or 1 bytes
  * based on the data availability in the RX FIFO and amount of bytes to be
  * received.
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_PopRxFifo(volatile stc_SMIF_t *baseaddr, cy_stc_smif_context_t *context)
  {
    /* The variable that shows which is smaller: the free FIFO size or amount of bytes to be received */
    uint32_t loadedFifoBytes = Cy_SMIF_GetRxFifoStatus(baseaddr);
    uint32_t buffCounter     = context->rxBufferCounter;
    /* readBytes = MIN(loadedFifoBytes, buffCounter) */
    uint32_t readBytes       = (loadedFifoBytes > buffCounter)? buffCounter: loadedFifoBytes;
    
    /* Check that after a FIFO Read, no new data is available */
    uint8_t *buff            = (uint8_t*) context->rxBufferAddress;
    while(readBytes != 0U)
    {
      if(readBytes == 1u)
      {
        buff[0U] = (uint8_t)baseaddr->unRX_DATA_MMIO_FIFO_RD1.u32Register;
      }
      else if(readBytes == 2u)
      {
        Cy_SMIF_UnPackByteArray(baseaddr->unRX_DATA_MMIO_FIFO_RD2.u32Register, &buff[0U], false);
      }
      else if(readBytes == 3u)
      {
        Cy_SMIF_UnPackByteArray(baseaddr->unRX_DATA_MMIO_FIFO_RD2.u32Register, &buff[0U], false);
        buff[2U] = (uint8_t)baseaddr->unRX_DATA_MMIO_FIFO_RD1.u32Register;
      }
      else if(readBytes == 4u)
      {
        Cy_SMIF_UnPackByteArray(baseaddr->unRX_DATA_MMIO_FIFO_RD4.u32Register, &buff[0U], true);
      }
      else if(readBytes == 5u)
      {
        Cy_SMIF_UnPackByteArray(baseaddr->unRX_DATA_MMIO_FIFO_RD4.u32Register, &buff[0U], true);
        buff[4U] = (uint8_t)baseaddr->unRX_DATA_MMIO_FIFO_RD1.u32Register;
      }
      else if(readBytes == 6u)
      {
        Cy_SMIF_UnPackByteArray(baseaddr->unRX_DATA_MMIO_FIFO_RD4.u32Register, &buff[0U], true);
        Cy_SMIF_UnPackByteArray(baseaddr->unRX_DATA_MMIO_FIFO_RD2.u32Register, &buff[4U], false);
      }
      else if(readBytes == 7u)
      {
        Cy_SMIF_UnPackByteArray(baseaddr->unRX_DATA_MMIO_FIFO_RD4.u32Register, &buff[0U], true);
        Cy_SMIF_UnPackByteArray(baseaddr->unRX_DATA_MMIO_FIFO_RD2.u32Register, &buff[4U], false);
        buff[6U] = (uint8_t)baseaddr->unRX_DATA_MMIO_FIFO_RD1.u32Register;
      }
      else /* readBytes => 8*/
      {
        Cy_SMIF_UnPackByteArray(baseaddr->unRX_DATA_MMIO_FIFO_RD4.u32Register, &buff[0U], true);
        Cy_SMIF_UnPackByteArray(baseaddr->unRX_DATA_MMIO_FIFO_RD4.u32Register, &buff[4U], true);
        readBytes = 8u;
      }
      
      buff            = &buff[readBytes];
      buffCounter    -= readBytes;
      /* Check if we already got new data in RX_FIFO*/
      loadedFifoBytes = Cy_SMIF_GetRxFifoStatus(baseaddr);
      readBytes       = (loadedFifoBytes > buffCounter)? buffCounter: loadedFifoBytes;
    }
    
    /* Save changes in the context */
    context->rxBufferAddress = buff;
    context->rxBufferCounter = buffCounter;
    
    /* Check if all bytes are received */
    if (buffCounter == 0UL)
    {
      /* Disable the TR_RX_REQ interrupt */
      baseaddr->unINTR_MASK.stcField.u1TR_RX_REQ = 0u;
      
      context->transferStatus = CY_SMIF_REC_CMPLT;
      if (NULL != context->rxCmpltCb)
      {
        context->rxCmpltCb((uint32_t) CY_SMIF_REC_CMPLT);
      }
    }
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_CheckRxFifo
  ***************************************************************************//***
  *
  * This function reads RX FIFO without shifting data.
  *
  * \param baseaddr
  * Holds the base address of the SMIF block registers.
  *
  * \return RX FIFO DATA
  *
  *
  *******************************************************************************/
  __STATIC_INLINE uint8_t Cy_SMIF_CheckRxFifoContents(volatile stc_SMIF_t *baseaddr)
  {
    return baseaddr->unRX_DATA_MMIO_FIFO_RD1_SILENT.stcField.u8DATA0;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_PackBytesArray
  ***************************************************************************//***
  *
  * \internal
  *
  * This function packs 0-numBytes of the buff byte array into a 4-byte value.
  *
  * \param buff
  * The byte array to pack.
  *
  * \param fourBytes
  *  - The True pack is for a 32-bit value.
  *  - The False pack is for a 16-bit value.
  *
  * \return
  *  The 4-byte value packed from the byte array.
  *
  *******************************************************************************/
  __STATIC_INLINE uint32_t Cy_SMIF_PackBytesArray(uint8_t const buff[], bool fourBytes)
  {
    uint32_t result = 0UL;
    
    result = ((uint32_t)buff[1UL] << 8UL) | (uint32_t)buff[0UL];
    
    if(fourBytes)
    {
      result |= ((uint32_t)buff[3UL] << 24UL) | ((uint32_t)buff[2UL] << 16UL);
    }
    
    return result;
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_UnPackByteArray
  ***************************************************************************//***
  *
  * \internal
  *
  * This function unpacks 0-numBytes from a 4-byte value into the byte array outBuff.
  *
  * \param smifReg
  *  The 4-byte value to unpack.
  *
  * \param outBuff
  * The byte array to fill.
  *
  * \param fourBytes
  *  - The True unpack is for a 32-bit value.
  *  - The False unpack is for a 16-bit value.
  *
  *******************************************************************************/
  __STATIC_INLINE void Cy_SMIF_UnPackByteArray(uint32_t inValue, uint8_t outBuff[], bool fourBytes)
  {
    outBuff[0UL] = (uint8_t)(inValue & 0xFFUL);
    outBuff[1UL] = (uint8_t)((inValue >> 8UL ) & 0xFFUL);
    
    if(fourBytes)
    {
      outBuff[2UL] = (uint8_t)((inValue >> 16UL) & 0xFFUL);
      outBuff[3UL] = (uint8_t)((inValue >> 24UL) & 0xFFUL);
    }
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_TimeoutRun
  ****************************************************************************//**
  *
  * \internal
  *
  * This function checks if the timeout is expired. Use the Cy_SysLib_DelayUs() function for
  * implementation.
  *
  * \param timeoutUnits
  *  The pointer to the timeout. The timeout measured in microseconds is multiplied by
  *  CY_SMIF_WAIT_1_UNIT.
  *
  * \return
  * A timeout status:
  *     - \ref CY_SMIF_SUCCESS - The timeout has not expired or input timeoutUnits is 0.
  *     - \ref CY_SMIF_EXCEED_TIMEOUT - The timeout has expired.
  *
  *******************************************************************************/
  __STATIC_INLINE cy_en_smif_status_t Cy_SMIF_TimeoutRun(uint32_t *timeoutUnits)
  {
    cy_en_smif_status_t status = CY_SMIF_SUCCESS;
    if (*timeoutUnits > 0u)
    {
      Cy_SysLib_DelayUs(CY_SMIF_WAIT_1_UNIT);
      --(*timeoutUnits);
      status = (0u == (*timeoutUnits))? CY_SMIF_EXCEED_TIMEOUT: CY_SMIF_SUCCESS;
    }
    return status;
  }
  
  /*******************************************************************************
  * Function Name: CY_SMIF_GetDeviceNumber
  ****************************************************************************//**
  *
  * This function returns device number which the SMIF IP has. 
  * User can input both SMIF DEVICE block address and SMIF block address
  *
  * \param base
  * Base address of the SMIF block or the SMIF DEVICE block.
  *
  * \ return device number of the SMIF IP.
  *
  *******************************************************************************/
  __STATIC_INLINE uint32_t CY_SMIF_GetDeviceNumber(volatile void *base)
  {
    if((base == SMIF0) || (base == SMIF0_DEVICE0) || (base == SMIF0_DEVICE1))
    {
      return (SMIF0_DEVICE_NR);
    }
#if defined (SMIF1)
    else if((base == SMIF1) || (base == SMIF1_DEVICE0) || (base == SMIF1_DEVICE1))
    {
      return (SMIF1_DEVICE_NR);
    }
#endif    
    else
    {
      CY_ASSERT_L2(false);
    }
  }
  
  /*******************************************************************************
  * Function Name: CY_SMIF_GetDelayTapsNumber
  ****************************************************************************//**
  *
  * This function returns tap number which the SMIF IP has. 
  * User can input both SMIF DEVICE block address and SMIF block address
  *
  * \param base
  * Base address of the SMIF block or the SMIF DEVICE block.
  *
  * \return Delay tap Number of the SMIF IP
  *
  *******************************************************************************/
  __STATIC_INLINE uint32_t CY_SMIF_GetDelayTapsNumber(volatile void *base)
  {
    if((base == SMIF0) || (base == SMIF0_DEVICE0) || (base == SMIF0_DEVICE1))
    {
      return (SMIF0_DELAY_TAPS_NR);
    }
#if defined (SMIF1)
    else if((base == SMIF1) || (base == SMIF1_DEVICE0) || (base == SMIF1_DEVICE1))
    {
      return (SMIF1_DELAY_TAPS_NR);
    }
#endif
    else
    {
      CY_ASSERT_L2(false);
    }
  }
  
  /*******************************************************************************
  * Function Name: CY_SMIF_GetDelayLinesNumber
  ****************************************************************************//**
  *
  * This function returns the number of delay lines which the SMIF IP has. 
  * User can input both SMIF DEVICE block address and SMIF block address
  *
  * \param base
  * Base address of the SMIF block or the SMIF DEVICE block.
  *
  * \return Number of delay lines of the SMIF IP
  *
  *******************************************************************************/
  __STATIC_INLINE uint32_t CY_SMIF_GetDelayLinesNumber(volatile void *base)
  {
    if((base == SMIF0) || (base == SMIF0_DEVICE0) || (base == SMIF0_DEVICE1))
    {
      return (SMIF0_DELAY_LINES_NR);
    }
#if defined (SMIF1)
    else if((base == SMIF1) || (base == SMIF1_DEVICE0) || (base == SMIF1_DEVICE1))
    {
      return (SMIF1_DELAY_LINES_NR);
    }
#endif
    else
    {
      CY_ASSERT_L2(false);
    }
  }
  
  /*******************************************************************************
  * Function Name: CY_SMIF_GetXIP_Mask
  ****************************************************************************//**
  *
  * This function returns XIP mask value which the SMIF IP has. 
  * User can input both SMIF DEVICE block address and SMIF block address
  *
  * \param base
  * Base address of the SMIF block or the SMIF DEVICE block.
  *
  * \return XIP address mask value of the SMIF IP
  *
  *******************************************************************************/
  __STATIC_INLINE uint32_t CY_SMIF_GetXIP_Mask(volatile void *base)
  {
    if((base == SMIF0) || (base == SMIF0_DEVICE0) || (base == SMIF0_DEVICE1))
    {
      return (SMIF0_SMIF_XIP_MASK);
    }
#if defined (SMIF1)
    else if((base == SMIF1) || (base == SMIF1_DEVICE0) || (base == SMIF1_DEVICE1))
    {
      return (SMIF1_SMIF_XIP_MASK);
    }
#endif
    else
    {
      CY_ASSERT_L2(false);
    }
  }
  
  /*******************************************************************************
  * Function Name: CY_SMIF_GetXIP_Address
  ****************************************************************************//**
  *
  * This function returns XIP address which the SMIF IP has. 
  * User can input both SMIF DEVICE block address and SMIF block address
  *
  * \param base
  * Base address of the SMIF block or the SMIF DEVICE block.
  *
  * \return XIP address of the SMIF IP
  *
  *******************************************************************************/
  __STATIC_INLINE uint32_t CY_SMIF_GetXIP_Address(volatile void *base)
  {
    if((base == SMIF0) || (base == SMIF0_DEVICE0) || (base == SMIF0_DEVICE1))
    {
      return (SMIF0_SMIF_XIP_ADDR);
    }
#if defined (SMIF1)
    else if((base == SMIF1) || (base == SMIF1_DEVICE0) || (base == SMIF1_DEVICE1))
    {
      return (SMIF1_SMIF_XIP_ADDR);
    }
#endif
    else
    {
      CY_ASSERT_L2(false);
    }
  }
  
  /*******************************************************************************
  * Function Name: CY_SMIF_MaskValueToSizeInByte
  ****************************************************************************//**
  *
  * This function converts \ref cy_en_device_size_t value to 32 bit value in byte.
  *
  * \param mask
  * \ref cy_en_device_size_t
  *
  * \return size in byte corresponding to the input
  *
  *******************************************************************************/
  __STATIC_INLINE uint32_t Cy_SMIF_MaskValueToSizeInByte(cy_en_device_size_t mask)
  {
    return (~((uint32_t)mask) + 1UL);
  }
  
  /*******************************************************************************
  * Function Name: Cy_SMIF_Reverse4ByteEndian
  ****************************************************************************//**
  *
  * This function reverses endianness of 32 bit input
  *
  * \param in
  * input 32 bit data to be reversed its endianness
  *
  * \return 32 bit data which has been reversed its endianness
  *
  *******************************************************************************/
  __STATIC_INLINE uint32_t Cy_SMIF_Reverse4ByteEndian(uint32_t in)
  {
    return ((in & 0xFF000000UL) >> 24UL) |
      ((in & 0x00FF0000UL) >> 8UL)  |
        ((in & 0x0000FF00UL) << 8UL)  |
          ((in & 0x000000FFUL) << 24UL);
  }
  /** \endcond */
  /** \} group_smif_low_level_functions */
  
  
#if defined(__cplusplus)
}
#endif

#ifdef CY_SMIF_NEW_VERSION
#define CRYPTO ((volatile stc_CRYPTO_t*) CRYPTO_BASE)
#endif

#endif /* CY_IP_MXSMIF */

#endif /* (CY_SMIF_H) */

/** \} group_smif */


/* [] END OF FILE */
