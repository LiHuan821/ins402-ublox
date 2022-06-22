#include "ethernet.h"
#include "uart.h"
#include <stdint.h>

//#define __TJA1100__    //ON TJA1100   OFF: LAN8770M

ethernet_dev_struct ethernet_dev;

#define ETHERNET_MAX_RXBUF_SIZE 4096
static uint8_t eth_rx_buff[ETHERNET_MAX_RXBUF_SIZE] = {0};
static fifo_type eth_rx_fifo;

#pragma pack(1)
typedef struct
{
    uint8_t desMacAddr[6];
    uint8_t srcMacAddr[6];
    uint16_t type;
    uint8_t data[1500];
} cy_stc_ether_frame_t;
#pragma pack()

static cy_stc_ether_frame_t eth_tx_frame;

/********************************************************/
#ifdef __TJA1100__
#define EMAC_MII    0
#define EMAC_RMII   1

#define EMAC_INTERFACE  EMAC_RMII
#else
#define EMAC_MII    1
#define EMAC_RMII   0

#define EMAC_INTERFACE  EMAC_MII
#endif

/** Interrupt Handlers for Ethernet 0  */
void Cy_Ethx_InterruptHandler(void) 
{
    Cy_EthIf00_DecodeEvent(); 
}

static void _eth_rx_frame_callback(cy_pstc_eth_type_t pstcEth, uint8_t* u8RxBuffer, uint32_t u32Length);
#ifdef __TJA1100__
#define PHY_ADDR         (0)
#else   //LAN8770M
#define PHY_ADDR         (5)
#endif
#define CORE_FREQ_IN_MHZ (CY_INITIAL_TARGET_FAST0_FREQ / 1000000ul)

/* Port configuration */
typedef struct
{
    volatile stc_GPIO_PRT_t* portReg;
    uint8_t                  pinNum;
    cy_stc_gpio_pin_config_t cfg;
} stc_pin_config;

stc_pin_config g_EthModeConfigTbl[] =
{
    {     ETHx_RD0_PORT,     ETHx_RD0_PIN,     0u,         CY_GPIO_DM_STRONG_IN_OFF,     P19_0_GPIO,      0u,      0u,    0u,       0u,       0u },
    {     ETHx_RD3_PORT,     ETHx_RD3_PIN,     0u,         CY_GPIO_DM_STRONG_IN_OFF,     P19_3_GPIO,      0u,      0u,    0u,       0u,       0u }

};

stc_pin_config g_EthConfigTbl[] =
{
    {     ETHx_RD0_PORT,     ETHx_RD0_PIN,     0u,         CY_GPIO_DM_HIGHZ,     ETHx_RD0_PIN_MUX,      0u,      0u,    0u,       0u,       0u },
    {     ETHx_RD3_PORT,     ETHx_RD3_PIN,     0u,         CY_GPIO_DM_HIGHZ,     ETHx_RD3_PIN_MUX,      0u,      0u,    0u,       0u,       0u }
};

stc_pin_config g_EthLanEnTbl = { ETHx_LAN_EN_PORT,   ETHx_LAN_EN_PIN,     1u, CY_GPIO_DM_STRONG_IN_OFF,   ETHx_LAN_EN_PIN_MUX,     0u,      0u,    0u,       0u,       0u };

stc_pin_config g_EtherPortConfigTbl[] =
{
  //{              port,              pin, outVal,                driveMode,                hsiom, intEdge, intMask, vtrip, slewRate, driveSel },
    {     ETHx_TD0_PORT,     ETHx_TD0_PIN,     0u, CY_GPIO_DM_STRONG_IN_OFF,     ETHx_TD0_PIN_MUX,      0u,      0u,    0u,       0u,       0u },
    {     ETHx_TD1_PORT,     ETHx_TD1_PIN,     0u, CY_GPIO_DM_STRONG_IN_OFF,     ETHx_TD1_PIN_MUX,      0u,      0u,    0u,       0u,       0u },
    // {    ETHx_TXER_PORT,    ETHx_TXER_PIN,     0u, CY_GPIO_DM_STRONG_IN_OFF,    ETHx_TXER_PIN_MUX,      0u,      0u,    0u,       0u,       0u },
    {  ETHx_TX_CTL_PORT,  ETHx_TX_CTL_PIN,     0u, CY_GPIO_DM_STRONG_IN_OFF,  ETHx_TX_CTL_PIN_MUX,      0u,      0u,    0u,       0u,       0u },
    {     ETHx_RD0_PORT,     ETHx_RD0_PIN,     0u,         CY_GPIO_DM_HIGHZ,     ETHx_RD0_PIN_MUX,      0u,      0u,    0u,       0u,       0u },
    {     ETHx_RD1_PORT,     ETHx_RD1_PIN,     0u,         CY_GPIO_DM_HIGHZ,     ETHx_RD1_PIN_MUX,      0u,      0u,    0u,       0u,       0u },
    {   ETHx_RX_ER_PORT,   ETHx_RX_ER_PIN,     0u,         CY_GPIO_DM_HIGHZ,   ETHx_RX_ER_PIN_MUX,      0u,      0u,    0u,       0u,       0u },
    {  ETHx_RX_CTL_PORT,  ETHx_RX_CTL_PIN,     0u,         CY_GPIO_DM_HIGHZ,  ETHx_RX_CTL_PIN_MUX,      0u,      0u,    0u,       0u,       0u },
    {     ETHx_MDC_PORT,     ETHx_MDC_PIN,     0u, CY_GPIO_DM_STRONG_IN_OFF,     ETHx_MDC_PIN_MUX,      0u,      0u,    0u,       0u,       0u },
    {    ETHx_MDIO_PORT,    ETHx_MDIO_PIN,     0u,        CY_GPIO_DM_STRONG,    ETHx_MDIO_PIN_MUX,      0u,      0u,    0u,       0u,       0u },
#if EMAC_INTERFACE == EMAC_MII
    {     ETHx_TD2_PORT,     ETHx_TD2_PIN,     0u, CY_GPIO_DM_STRONG_IN_OFF,     ETHx_TD2_PIN_MUX,      0u,      0u,    0u,       0u,       0u },
    {     ETHx_TD3_PORT,     ETHx_TD3_PIN,     0u, CY_GPIO_DM_STRONG_IN_OFF,     ETHx_TD3_PIN_MUX,      0u,      0u,    0u,       0u,       0u },
    {  ETHx_TX_CLK_PORT,  ETHx_TX_CLK_PIN,     0u,         CY_GPIO_DM_HIGHZ,  ETHx_TX_CLK_PIN_MUX,      0u,      0u,    0u,       0u,       0u },
    {     ETHx_RD2_PORT,     ETHx_RD2_PIN,     0u,         CY_GPIO_DM_HIGHZ,     ETHx_RD2_PIN_MUX,      0u,      0u,    0u,       0u,       0u },
    {     ETHx_RD3_PORT,     ETHx_RD3_PIN,     0u,         CY_GPIO_DM_HIGHZ,     ETHx_RD3_PIN_MUX,      0u,      0u,    0u,       0u,       0u },
    {  ETHx_RX_CLK_PORT,  ETHx_RX_CLK_PIN,     0u,         CY_GPIO_DM_HIGHZ,  ETHx_RX_CLK_PIN_MUX,      0u,      0u,    0u,       0u,       0u },
#elif (EMAC_INTERFACE == EMAC_RMII)
    { ETHx_REF_CLK_PORT, ETHx_REF_CLK_PIN,     0u,         CY_GPIO_DM_HIGHZ, ETHx_REF_CLK_PIN_MUX,      0u,      0u,    0u,       0u,       0u },
#endif

#ifndef __TJA1100__
    { ETHx_LAN_EN_PORT,   ETHx_LAN_EN_PIN,     0u, CY_GPIO_DM_STRONG_IN_OFF,   ETHx_LAN_EN_PIN_MUX,     0u,      0u,    0u,       0u,       0u },
    { ETHx_WAKEUP_IN_PORT, ETHx_WAKEUP_IN_PIN, 0u, CY_GPIO_DM_STRONG_IN_OFF,   ETHx_WAKEUP_IN_PIN_MUX,  0u,      0u,    0u,       0u,       0u },
#endif
    {   ETHx_RESET_PORT,   ETHx_RESET_PIN,     1u, CY_GPIO_DM_STRONG_IN_OFF,   ETHx_RESET_PIN_MUX,      0u,      0u,    0u,       0u,       0u }
};

/** Wrapper configuration   */
cy_str_ethif_wrapper_config stcWrapperConfig = 
{
  #if (EMAC_INTERFACE == EMAC_MII)
    .stcInterfaceSel = CY_ETHIF_CTL_MII_100,    /** 100 Mbps MII */ 
  #elif (EMAC_INTERFACE == EMAC_RMII)
    .stcInterfaceSel = CY_ETHIF_CTL_RMII_100,   /** 100 Mbps MII */ 
  #endif
    .bRefClockSource = CY_ETHIF_EXTERNAL_HSIO,  /** assigning Ref_Clk to HSIO Clock, it is recommended to use external clock coming from HSIO  */
    .u8RefClkDiv     = 1u,                      /** Divide Refclock by 1 to have 25 MHz Tx clock  */
};

/** Default Timer register values   */
CEDI_1588TimerVal stc1588TimerValue = 
{
    .secsUpper = 0u,
    .secsLower = 0ul,
    .nanosecs  = 0ul,
};

/** Increment values for each clock cycles      */
/** This Increment values are calculated for source clock of 24 MHz */
CEDI_TimerIncrement stc1588TimerIncValue =
{
    .nanoSecsInc = 41u,     /** Having source clock of 24 MHz, with each clock cycle Nanosecond counter shall be incremented by 41  */
    .subNsInc    = 43690ul, /** incrementing just Nanosecond counter will not give accuracy, so sub-nanosecond counter also must be incremented  */  
    .lsbSubNsInc = 170u,    /** 170 (0xAA) is the lower 8 bits increment value for subNanosecond and 43690 (0xAAAA) is the higher 16 bits of the counter */
    .altIncCount = 0u,      /** This example is not using alternate increment mode, also it is not recommended by IP provider       */
    .altNanoSInc = 0u,      /** When Alt increment mode is disabled, then this counter does not play any role          */
};

/** To calculate the value to write to the sub-ns register you take the decimal value of 
    the sub-nanosecond value, then multiply it by 2 to the power of 24 (16777216) and 
    convert the result to hexadecimal. For example for a sub-nanosecond value of 0.33333333 
    you would write 0x55005555.  */ 
/** TSU configuration   */
const cy_str_ethif_TSU_Config stcTSUConfig = 
{
    .pstcTimerValue         = &stc1588TimerValue,    /** start value for the counter     */
    .pstcTimerIncValue      = &stc1588TimerIncValue, /** Increment value for each clock cycle    */
    .bOneStepTxSyncEnable   = false,                 /** useful when PTP protocol is in place    */
    .enTxDescStoreTimeStamp = CEDI_TX_TS_DISABLED,
    .enRxDescStoreTimeStamp = CEDI_RX_TS_DISABLED,
    .bStoreNSinRxDesc       = false,
};

/** General Ethernet configuration  */
cy_stc_ethif_configuration_t stcENETConfig =
{
    .bintrEnable         = true,                        /** Interrupt enable  */
    .dmaDataBurstLen     = CEDI_DMA_DBUR_LEN_4, 
    .u8dmaCfgFlags       = CEDI_CFG_DMA_FRCE_TX_BRST,
    .mdcPclkDiv          = CEDI_MDC_DIV_BY_48,          /** source clock is 80 MHz and MDC must be less than 2.5MHz   */
    .u8rxLenErrDisc      = 0u,                          /** Length error frame not discarded  */
    .u8disCopyPause      = 0u,
    .u8chkSumOffEn       = 0u,                          /** Checksum for both Tx and Rx disabled    */
    .u8rx1536ByteEn      = 1u,                          /** Enable receive frame up to 1536    */
    .u8rxJumboFrEn       = 0u,
    .u8enRxBadPreamble   = 1u,
    .u8ignoreIpgRxEr     = 0u,
    .u8storeUdpTcpOffset = 0u,
    .u8aw2wMaxPipeline   = 2u,                          /** Value must be > 0   */
    .u8ar2rMaxPipeline   = 2u,                          /** Value must be > 0   */
    .u8pfcMultiQuantum   = 0u,
    .pstcWrapperConfig   = &stcWrapperConfig,
    .pstcTSUConfig       = NULL,                        /** No TSU settings */
    .btxq0enable         = true,                        /** Tx Q0 Enabled   */
    .btxq1enable         = false,                       /** Tx Q1 Disabled  */
    .btxq2enable         = false,                       /** Tx Q2 Disabled  */
    .brxq0enable         = true,                        /** Rx Q0 Enabled   */
    .brxq1enable         = false,                       /** Rx Q1 Disabled  */
    .brxq2enable         = false,                       /** Rx Q2 Disabled  */
};
    
/** Interrupt configurations    */
cy_stc_ethif_interruptconfig_t stcInterruptConfig =
{
    .btsu_time_match        = false,          /** Time stamp unit time match event */
    .bwol_rx                = false,          /** Wake on LAN event received */
    .blpi_ch_rx             = false,          /** LPI indication status bit change received */
    .btsu_sec_inc           = false,          /** TSU seconds register increment */
    .bptp_tx_pdly_rsp       = false,          /** PTP pdelay_resp frame transmitted */
    .bptp_tx_pdly_req       = false,          /** PTP pdelay_req frame transmitted */
    .bptp_rx_pdly_rsp       = false,          /** PTP pdelay_resp frame received */
    .bptp_rx_pdly_req       = false,          /** PTP pdelay_req frame received */
    .bptp_tx_sync           = false,          /** PTP sync frame transmitted */
    .bptp_tx_dly_req        = false,          /** PTP delay_req frame transmitted */
    .bptp_rx_sync           = false,          /** PTP sync frame received */
    .bptp_rx_dly_req        = false,          /** PTP delay_req frame received */
    .bext_intr              = false,          /** External input interrupt detected */
    .bpause_frame_tx        = false,          /** Pause frame transmitted */
    .bpause_time_zero       = false,          /** Pause time reaches zero or zeroq pause frame received */
    .bpause_nz_qu_rx        = false,          /** Pause frame with non-zero quantum received */
    .bhresp_not_ok          = false,          /** DMA hresp not OK */
    .brx_overrun            = true,          /** Rx overrun error */
    .bpcs_link_change_det   = false,          /** Link status change detected by PCS */
    .btx_complete           = true,          /** Frame has been transmitted successfully */
    .btx_fr_corrupt         = true,          /** Tx frame corruption */
    .btx_retry_ex_late_coll = true,          /** Retry limit exceeded or late collision */
    .btx_underrun           = true,          /** Tx underrun */
    .btx_used_read          = true,          /** Used bit set has been read in Tx descriptor list */
    .brx_used_read          = true,          /** Used bit set has been read in Rx descriptor list */
    .brx_complete           = true,          /** Frame received successfully and stored */
    .bman_frame             = false,          /** Management Frame Sent */   

    /** call back function  */
    .rxframecb              = _eth_rx_frame_callback,
    .txerrorcb              = NULL,
    .txcompletecb           = NULL,
    .tsuSecondInccb         = NULL,
};


/** Enable Ethernet Interrupts  */
const cy_stc_sysint_irq_t irq_cfg_ethx_q0 = {.sysIntSrc  = ETH_INTR_SRC,    .intIdx  = CPUIntIdx2_IRQn, .isEnabled  = true};
const cy_stc_sysint_irq_t irq_cfg_ethx_q1 = {.sysIntSrc  = ETH_INTR_SRC_Q1, .intIdx  = CPUIntIdx2_IRQn, .isEnabled  = true};
const cy_stc_sysint_irq_t irq_cfg_ethx_q2 = {.sysIntSrc  = ETH_INTR_SRC_Q2, .intIdx  = CPUIntIdx2_IRQn, .isEnabled  = true};

/********************************************************/
cy_stc_eth_filter_config_t fileter1Config = 
{
    .typeFilter      = CY_ETH_FILTER_TYPE_DESTINATION,
    .filterAddr.byte = {0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u},
    .ignoreBytes     = 0x00u,
};

uint8_t ethernet_srcMacAddr[6] = {0x05u, 0x00u, 0x00u, 0x00u, 0x00u, 0x05u};

/********************************************************/
static void _eth_rx_frame_callback(cy_pstc_eth_type_t pstcEth, uint8_t* u8RxBuffer, uint32_t u32Length)
{
    uint16_t type = 0;
    uint8_t macDstAddrBroadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint16_t msg_type = 0;

    if ((u32Length >= 18) && (u32Length <= 1518)) {
        if (memcmp(&eth_tx_frame.srcMacAddr[0], &u8RxBuffer[0], 6u) == 0) {
            type = ((uint16_t)u8RxBuffer[13] << 8) | u8RxBuffer[12];
            // UART7_Printf("eth rx %d %d\r\n", u32Length, type);
            if (type <= 1500) {
                fifo_push(&eth_rx_fifo, &u8RxBuffer[14], type);
            }
        }
        else if ((memcmp(macDstAddrBroadcast, &u8RxBuffer[0], 6u) == 0) || (memcmp(&eth_tx_frame.srcMacAddr[0], &u8RxBuffer[0], 6u) == 0))
        {
           msg_type = ((uint16_t)u8RxBuffer[17] << 8) | u8RxBuffer[16];
           if((msg_type == 0xcc01) || (msg_type == 0xcc0b))
           {
               memcpy(&eth_tx_frame.desMacAddr[0], &u8RxBuffer[6], 6);
               
               type = ((uint16_t)u8RxBuffer[13] << 8) | u8RxBuffer[12];
               if (type <= 1500) {
                  fifo_push(&eth_rx_fifo, &u8RxBuffer[14], type);
               }
           }          
        }
    }

    Cy_EthIF_ClearReleasedBuf(true, false);
}

static void _ethernet_struct_init(void)
{
    uint32_t serial_number = 0;
          
    ethernet_dev.phy_master_slave = PYH_MASTER; // PYH_MASTER PYH_SLAVE
    ethernet_dev.ethif_status = ETHIF_STATUS_DOWN;
    ethernet_dev.ethif_link = ETHIF_LINK_DOWN;

    fifo_init(&eth_rx_fifo, eth_rx_buff, ETHERNET_MAX_RXBUF_SIZE);

    memset(&eth_tx_frame, 0, sizeof(cy_stc_ether_frame_t));
    memset(&eth_tx_frame.desMacAddr[0], 0xFF, 6);
       
    serial_number = cal_get_unit_serialnum();
    ethernet_srcMacAddr[0] = serial_number & 0xFF;
    ethernet_srcMacAddr[1] = (serial_number >> 8) & 0xFF;
    ethernet_srcMacAddr[2] = (serial_number >> 16) & 0xFF;
    ethernet_srcMacAddr[3] = (serial_number >> 24) & 0xFF;
    ethernet_srcMacAddr[4] = 0x00;
    ethernet_srcMacAddr[5] = 0x28;
          
    memcpy(&eth_tx_frame.srcMacAddr[0], &ethernet_srcMacAddr[0], 6);
}

static void _ethernet_io_init(void)
{
    /** Ethernet Port Pin settings  */
    for (uint32_t i = 0ul; i < (sizeof(g_EtherPortConfigTbl)/sizeof(stc_pin_config)); i++)
    {
        Cy_GPIO_Pin_Init(g_EtherPortConfigTbl[i].portReg, g_EtherPortConfigTbl[i].pinNum, &g_EtherPortConfigTbl[i].cfg);
    }      
#ifndef __TJA1100__   
    /** MII Mode  */
    for (uint32_t i = 0ul; i < (sizeof(g_EthModeConfigTbl)/sizeof(stc_pin_config)); i++)
    {
        Cy_GPIO_Pin_Init(g_EthModeConfigTbl[i].portReg, g_EthModeConfigTbl[i].pinNum, &g_EthModeConfigTbl[i].cfg);
    }    
#endif
    Cy_GPIO_Pin_Init(g_EthLanEnTbl.portReg, g_EthLanEnTbl.pinNum, &g_EthLanEnTbl.cfg);
    
    Cy_GPIO_Clr(ETHx_RESET_PORT, ETHx_RESET_PIN);
    Cy_SysTick_DelayInUs(100ul);
    Cy_GPIO_Set(ETHx_RESET_PORT, ETHx_RESET_PIN); 
    Cy_SysTick_DelayInUs(50000ul);

#ifndef __TJA1100__   
        /** Ethernet Port Pin settings  */
    for (uint32_t i = 0ul; i < (sizeof(g_EthConfigTbl)/sizeof(stc_pin_config)); i++)
    {
        Cy_GPIO_Pin_Init(g_EthConfigTbl[i].portReg, g_EthConfigTbl[i].pinNum, &g_EthConfigTbl[i].cfg);
	// UART7_Printf("Pin: %d\r\n", Cy_GPIO_Read(g_EthConfigTbl[i].portReg, g_EthConfigTbl[i].pinNum));
    }
#endif
    
    Cy_SysInt_InitIRQ(&irq_cfg_ethx_q0);
    Cy_SysInt_SetSystemIrqVector(irq_cfg_ethx_q0.sysIntSrc, Cy_Ethx_InterruptHandler);

    Cy_SysInt_InitIRQ(&irq_cfg_ethx_q1);
    Cy_SysInt_SetSystemIrqVector(irq_cfg_ethx_q1.sysIntSrc, Cy_Ethx_InterruptHandler);

    Cy_SysInt_InitIRQ(&irq_cfg_ethx_q2);
    Cy_SysInt_SetSystemIrqVector(irq_cfg_ethx_q2.sysIntSrc, Cy_Ethx_InterruptHandler);

    NVIC_SetPriority(CPUIntIdx2_IRQn, 5ul);
    NVIC_EnableIRQ(CPUIntIdx2_IRQn);
}

static int32_t _ethernet_filter_init(void)
{
    cy_en_ethif_status_t ethStatus;

    fileter1Config.typeFilter = CY_ETH_FILTER_TYPE_DESTINATION;
    fileter1Config.ignoreBytes = 0x00u;
    
    memcpy(&fileter1Config.filterAddr.byte[0], &ethernet_srcMacAddr[0], 6);

    ethStatus = Cy_EthIf_SetFilterAddress(ETH_REG_BASE, CY_ETH_FILTER_NUM_1, &fileter1Config);

    return ethStatus;
}
#ifdef __TJA1100__
static int32_t _initphy_tja1100(void)
{
    uint32_t trycount, trycount2;
    uint8_t phy_init_state = 0;

    trycount = 0;
    while (trycount++ < 100) {
        /* Reset PHY */
        Cy_EthIf_PhyRegWrite(ETH_REG_BASE, 0x00u, 0x8000u, PHY_ADDR, CORE_FREQ_IN_MHZ);
        trycount2 = 0;
        while (trycount2++ < 100)
        {
            uint16_t read_data0 = Cy_EthIf_PhyRegRead(ETH_REG_BASE, 0x00u, PHY_ADDR, CORE_FREQ_IN_MHZ);
            if ((read_data0 & 0x8000u) == 0u) // RESET = 0
            {
                break;
            }
        }

        uint16_t read_data21 = Cy_EthIf_PhyRegRead(ETH_REG_BASE, 0x15u, PHY_ADDR, CORE_FREQ_IN_MHZ);
        if (read_data21 & 0x0800u)  // PHY_INIT_FAIL = 1
        {
            continue;
        }
        else
        {
            phy_init_state = 1;
            break;
        }
    }

    if (phy_init_state != 1) {
        return -1;
    }

    Cy_EthIf_WaitUsec(30000ul, CORE_FREQ_IN_MHZ);                                    /** Some more delay to get PHY adapted to new interface	*/      

    Cy_EthIf_PhyRegWrite(ETH_REG_BASE, 0x11u, 0x0006u, PHY_ADDR, CORE_FREQ_IN_MHZ); /* extended control tegister (reg17) CONFIG_EN=1*/

    uint32_t reg_18_value = 0x10u;
    if (ethernet_dev.phy_master_slave == PYH_MASTER) {
        reg_18_value |= 0x8000;
    }
#if EMAC_INTERFACE == EMAC_MII
    reg_18_value |= 0x800;
#else   //RMII 50MHz 0x900 , 25Mhz 0xa00
    reg_18_value |= 0x900;
#endif
    Cy_EthIf_PhyRegWrite(ETH_REG_BASE, 0x12u, reg_18_value, PHY_ADDR, CORE_FREQ_IN_MHZ);

    Cy_EthIf_PhyRegWrite(ETH_REG_BASE, 0x16u, 0xFFFFu, PHY_ADDR, CORE_FREQ_IN_MHZ); /* Enable all of interrupt */
    Cy_EthIf_PhyRegWrite(ETH_REG_BASE, 0x00u, 0x2100u, PHY_ADDR, CORE_FREQ_IN_MHZ); /** 100M, Full Duplex and Auto Negotiation OFF  */

    trycount = 0;
    while (trycount++ < 100)
    {
        Cy_EthIf_PhyRegWrite(ETH_REG_BASE, 0x11u, 0x1802u, PHY_ADDR, CORE_FREQ_IN_MHZ); // Normal mode, config disable
        Cy_EthIf_WaitUsec(4000ul, CORE_FREQ_IN_MHZ);

        uint16_t read_data17 = Cy_EthIf_PhyRegRead(ETH_REG_BASE, 0x11u, PHY_ADDR, CORE_FREQ_IN_MHZ);
        if ((read_data17 & 0x1800u) == 0x1800u) // make sure it became Normal mode.
        {
            phy_init_state = 2;
            break;
        }
    }

    if (phy_init_state != 2) {
        return -2;
    }

    /* Extended control register (Register 17) */
    Cy_EthIf_PhyRegWrite(ETH_REG_BASE, 0x11u, 0x8002u, PHY_ADDR, CORE_FREQ_IN_MHZ); // link control enabled
    // Cy_EthIf_PhyRegWrite(ETH_REG_BASE, 0x11u, 0x0002u, PHY_ADDR, CORE_FREQ_IN_MHZ); // link control disable
    // Cy_EthIf_WaitUsec(200ul, CORE_FREQ_IN_MHZ);
    // Cy_EthIf_PhyRegWrite(ETH_REG_BASE, 0x11u, 0x8002u, PHY_ADDR, CORE_FREQ_IN_MHZ); // link control enabled

    return 0;
}

static int32_t _check_tja1100_linkstatus(void)
{
    uint16_t u32ReadData = 0u;
    /** PHY register might take some time to update internal registers */    
    u32ReadData = Cy_EthIf_PhyRegRead(ETH_REG_BASE, 0x17u, PHY_ADDR, CORE_FREQ_IN_MHZ);       /** PHY status register: 0x0011 */
    if (u32ReadData & 0x8000u) {
        /** Link up */
        return 0;
    }

    /** Link down */
    return -1;
}
#else
/*******************************************************************************
* Function Name: InitPHY_LAN8770M
****************************************************************************//**
*
* \brief Dedicated to initialize PHY LAN8770M
* Function configures the PHY with 100 Mbps link speed, full duplex communication and Auto Negotiation on
*
* \Note:
* If hardware consist of any other PHY than LAN8770M, dedicated function shall be written to configure relevant 
* registers in side PHY
*******************************************************************************/
static int InitPHY_LAN8770M(void)
{
    static const struct access_ereg_val init[] = {
            /* 100M, Full Duplex and Auto Negotiation OFF */
            {PHYACC_ATTR_MODE_MODIFY, PHYACC_ATTR_BANK_SMI, 0x00,
             0x2100, 0},
             
            /* TX Amplitude = 5 */
            {PHYACC_ATTR_MODE_MODIFY, PHYACC_ATTR_BANK_AFE, 0x0B,
             0x000A, 0x001E},
            /* Clear SMI interrupts */
            {PHYACC_ATTR_MODE_READ, PHYACC_ATTR_BANK_SMI, 0x18,
             0, 0},
            /* Clear MISC interrupts */
            {PHYACC_ATTR_MODE_READ, PHYACC_ATTR_BANK_MISC, 0x08,
             0, 0},
            /* Turn on TC10 Ring Oscillator (ROSC) */
            {PHYACC_ATTR_MODE_MODIFY, PHYACC_ATTR_BANK_MISC, 0x20,
             0x0020, 0x0020},
            /* WUR Detect Length to 1.2uS, LPC Detect Length to 1.09uS */
            {PHYACC_ATTR_MODE_WRITE, PHYACC_ATTR_BANK_PCS, 0x20,
             0x283C, 0},
            /* Wake_In Debounce Length to 39uS, Wake_Out Length to 79uS */
            {PHYACC_ATTR_MODE_WRITE, PHYACC_ATTR_BANK_MISC, 0x21,
             0x274F, 0},
            /* Enable Auto Wake Forward to Wake_Out, ROSC on, Sleep,
             * and Wake_In to wake PHY
             */
            {PHYACC_ATTR_MODE_WRITE, PHYACC_ATTR_BANK_MISC, 0x20,
             0x80A7, 0},
            /* Enable WUP Auto Fwd, Enable Wake on MDI, Wakeup Debouncer
             * to 128 uS
             */
            {PHYACC_ATTR_MODE_WRITE, PHYACC_ATTR_BANK_MISC, 0x24,
             0xF110, 0},
            /* Enable HW Init */
            {PHYACC_ATTR_MODE_MODIFY, PHYACC_ATTR_BANK_SMI, 0x1A,
             0x0100, 0x0100},
            /* Enable all of interrupt */
            {PHYACC_ATTR_MODE_MODIFY, PHYACC_ATTR_BANK_SMI, 0x19,
             0xFFFF, 0xFFFF},
                                 
            /* loopback test */
//            {PHYACC_ATTR_MODE_MODIFY, PHYACC_ATTR_BANK_MISC, 0x11,
//             0x0006, 0x0006},
    };
    int rc;
    int i;
    uint32_t reg_master_slave_value = 0;
    
    /* Start manual initialization procedures in Managed Mode */
    rc = access_ereg_modify_changed(PHYACC_ATTR_BANK_SMI,
                                    0x1a, 0x0000, 0x0100);
    if (rc < 0)
    {
        return rc;
    }

    /* Soft Reset the SMI block */
    rc = access_ereg_modify_changed(PHYACC_ATTR_BANK_SMI,
                                    0x00, 0x8000, 0x8000);

    if (rc < 0)
    {
        return rc;
    }
        
    /* Check to see if the self-clearing bit is cleared */
    Cy_EthIf_WaitUsec(30000, CORE_FREQ_IN_MHZ);                                           /** Required delay of 30 ms to get PHY back to Run state after Reset	*/
    rc = access_ereg(PHYACC_ATTR_MODE_READ,
                     PHYACC_ATTR_BANK_SMI, 0x00, 0);
    if (rc < 0)
    {
        return rc;
    }
    if ((rc & 0x8000) != 0)
    {
        return -1;
    }
    
    if (ethernet_dev.phy_master_slave == PYH_MASTER) 
    {
        reg_master_slave_value |= 0x0800;
    }
    
    rc = access_ereg_modify_changed(PHYACC_ATTR_BANK_SMI,
                                    0x09, reg_master_slave_value, 0x0800);
    if (rc < 0)
    {
        return rc;
    }
    
    /* PHY Initialization */
    for (i = 0; i < sizeof(init)/sizeof(struct access_ereg_val); i++) 
    {
        if (init[i].mode == PHYACC_ATTR_MODE_MODIFY) 
        {
            rc = access_ereg_modify_changed(init[i].bank,
                                            init[i].offset,
                                            init[i].val,
                                            init[i].mask);
        } 
        else 
        {
            rc = access_ereg(init[i].mode, init[i].bank,
                             init[i].offset, init[i].val);
        }
        if (rc < 0)
        {
                return rc;
        }
    } 
    
    rc = access_ereg(PHYACC_ATTR_MODE_READ, PHYACC_ATTR_BANK_MISC,
                             0x10, 0);

    return 0;

}

/*******************************************************************************
* Function Name: Check_LAN8770M_LinkStatus
****************************************************************************//**
*
* \brief Function reads specific register from LAN8770M to learn Link status
*
* \param 
* \return 0 Link up
* \return -1 Link Down 
*
*******************************************************************************/
static int32_t Check_LAN8770M_LinkStatus (void)
{
    int ReadData = 0;
    
    /** PHY register might take some time to check link status of internal registers */
    ReadData = access_ereg(PHYACC_ATTR_MODE_READ, PHYACC_ATTR_BANK_SMI, 0x01, 0x00);    
    if (ReadData & 0x0004)
    {
        /** Link up */
        return 0;
    }
    else
    {
        /** Link down   */
        return -1;
    }   
}

/*******************************************************************************
* Function Name: Phy_LAN8770M_MDIO_validation
****************************************************************************//**
*
* \brief Function checks MDIO interface and functioning of PHY
*
* \param 
* \return true read id Passed
* \return false read id fail 
*
*******************************************************************************/
static bool Phy_LAN8770M_MDIO_validation (void)
{
    int ReadData = 0;
       
    /** check id1 of LAN8770M PHY */      
    ReadData = access_ereg(PHYACC_ATTR_MODE_READ, PHYACC_ATTR_BANK_SMI, 0x02, 0x00);       /** PHY Identifier Register #1  */
    if (ReadData != 0x0007)
    {
        /** Error   */
        return false;
    }
    
    /** check id2 of LAN8770M PHY */      
    ReadData = access_ereg(PHYACC_ATTR_MODE_READ, PHYACC_ATTR_BANK_SMI, 0x03, 0x00);       /** PHY Identifier Register #2  */
    if ((ReadData & 0xFFF0) != 0xC150)
    {
        /** Error   */
        return false;
    }
    
    return true;
}

/*******************************************************************************
* Function Name: access_ereg
****************************************************************************//**
*
* \brief Function reads or write specific register of the LAN8770M chip.
*
* \param 
* \return 0 access success
* \return -1 access failture 
* \return -2 parameter error 
*
*******************************************************************************/
static int access_ereg(uint8_t mode, uint8_t bank, uint8_t offset, uint16_t val)
{
    uint16_t ereg = 0;
    int rc = ACCESS_SUCCESS;

    if (mode > PHYACC_ATTR_MODE_WRITE || bank > PHYACC_ATTR_BANK_MAX)
    {
        return PARAMETER_ERROR;
    }

    if (bank == PHYACC_ATTR_BANK_SMI) 
    {
        if (mode == PHYACC_ATTR_MODE_WRITE)
        {
            rc = Cy_EthIf_PhyRegWrite(ETH_REG_BASE, offset, val, PHY_ADDR, CORE_FREQ_IN_MHZ);
        }
        else
        {
            rc = Cy_EthIf_PhyRegRead(ETH_REG_BASE, offset, PHY_ADDR, CORE_FREQ_IN_MHZ);
        }
        return rc;
    }

    if (mode == PHYACC_ATTR_MODE_WRITE) 
    {
        ereg = LAN87XX_EXT_REG_CTL_WR_CTL;
        rc = Cy_EthIf_PhyRegWrite(ETH_REG_BASE, LAN87XX_EXT_REG_WR_DATA, val, PHY_ADDR, CORE_FREQ_IN_MHZ);
        if (rc < ACCESS_SUCCESS)
        {
            return ACCESS_FAILTURE;
        }
    } 
    else 
    {
        ereg = LAN87XX_EXT_REG_CTL_RD_CTL;
    }

    ereg |= (bank << 8) | offset;

    rc = Cy_EthIf_PhyRegWrite(ETH_REG_BASE, LAN87XX_EXT_REG_CTL, ereg, PHY_ADDR, CORE_FREQ_IN_MHZ);
    if (rc < ACCESS_SUCCESS)
    {
        return ACCESS_FAILTURE;
    }

    if (mode == PHYACC_ATTR_MODE_READ)
    {
        rc = Cy_EthIf_PhyRegRead(ETH_REG_BASE, LAN87XX_EXT_REG_RD_DATA, PHY_ADDR, CORE_FREQ_IN_MHZ);
    }

    return rc;
}

/*******************************************************************************
* Function Name: access_ereg_modify_changed
****************************************************************************//**
*
* \brief Function modify specific register of the LAN8770M chip.
*
* \param 
* \return 0 access success
* \return -1 access failture 
* \return -2 parameter error 
*
*******************************************************************************/
static int access_ereg_modify_changed(uint8_t bank, uint8_t offset, uint16_t val, uint16_t mask)
{
    int new = 0;
    int rc = ACCESS_SUCCESS;

    if (bank > PHYACC_ATTR_BANK_MAX)
    {
        return PARAMETER_ERROR;
    }

    rc = access_ereg(PHYACC_ATTR_MODE_READ, bank, offset, val);
    if (rc < ACCESS_SUCCESS)
    {
        return rc;
    }

    new = val | (rc & (mask ^ 0xFFFF));
    rc = access_ereg(PHYACC_ATTR_MODE_WRITE, bank, offset, new);

    return rc;
}
#endif
/********************************************************/
int32_t ethernet_init(void)
{
    cy_en_ethif_status_t ethStatus;
    int32_t ret;
    uint32_t link_count = 30;

    /* ethernet struct init */
    _ethernet_struct_init();

    // UART7_Printf("eth init %d...\r\n", ethernet_dev.phy_master_slave);

    /** Configure Ethernet Port pins    */
    _ethernet_io_init();

    /** Initialize ENET MAC */
    ethStatus = Cy_EthIf_Init(ETH_REG_BASE, &stcENETConfig, &stcInterruptConfig);
    if (ethStatus != CY_ETHIF_SUCCESS) {
        return -1;
    }
#ifndef __TJA1100__   
        /* Soft Reset the SMI block */
    access_ereg_modify_changed(PHYACC_ATTR_BANK_SMI, 0x00, 0x8000, 0x8000);
#endif
    /* Disable copy all frames */
    Cy_EthIf_SetCopyAllFrames(ETH_REG_BASE, false);

    /* no Reject Broad cast frames */
    Cy_EthIf_SetNoBroadCast(ETH_REG_BASE, false);

    /* Filter settings */
    ethStatus = _ethernet_filter_init();
    if (ethStatus != CY_ETHIF_SUCCESS) {
        return -1;
    }

    ethernet_dev.ethif_status = ETHIF_STATUS_MAC_INIT_OK;

    // UART7_Printf("eth init mac ok\r\n");
#ifndef __TJA1100__    
    Phy_LAN8770M_MDIO_validation();
#endif
    /** PHY initialization  */
#ifdef __TJA1100__
    ret = _initphy_tja1100();
#else
    ret = InitPHY_LAN8770M(); 
#endif
    if (ret != 0) {
        return -2;
    }
    
    ethernet_dev.ethif_status = ETHIF_STATUS_PHY_INIT_OK;

    // UART7_Printf("eth init phy ok\r\n");
    while (link_count != 0) {
        link_count--;
        ethernet_check_link();
        if (ethernet_dev.ethif_link == ETHIF_LINK_UP) {
            break;
        }
        Cy_SysTick_DelayInUs(1000ul);
    }

    // UART7_Printf("eth init phy link status %d\r\n", ethernet_dev.ethif_link);

    return 0;
}

int32_t ethernet_check_link(void)
{
    int32_t linkstatus;
    int32_t retval = -1;

    if (ethernet_dev.ethif_status == ETHIF_STATUS_PHY_INIT_OK) {
#ifdef __TJA1100__
        linkstatus = _check_tja1100_linkstatus();
#else
        linkstatus = Check_LAN8770M_LinkStatus();
#endif
        if (!linkstatus) {
            retval = 0;
            if (ethernet_dev.ethif_link == ETHIF_LINK_DOWN) {
                ethernet_dev.ethif_link = ETHIF_LINK_UP;
                // UART7_Printf("eth link up\r\n");
            }
        } else {
            ethernet_dev.ethif_link = ETHIF_LINK_DOWN;
            // UART7_Printf("eth link down\r\n");
        }
    }
    return retval;
}

#define ETH_PACKET_MINLENGTH

int32_t ethernet_send_data(uint8_t* buf, uint16_t len)
{
    cy_en_ethif_status_t ethStatus;

    if (ethernet_dev.ethif_link == ETHIF_LINK_DOWN) {
        return -2;
    }

    if (len > 1500) {
        return -1;
    }

#ifdef ETH_PACKET_MINLENGTH
    if (len >= 46) {
        eth_tx_frame.type = ((len&0x00ff) << 8)  + ((len&0xff00) >> 8);
        memcpy(eth_tx_frame.data, buf, len);
    } else {
        memcpy(eth_tx_frame.data, buf, len);
        memset(&eth_tx_frame.data[len], 0, 46-len);
        len = 46;
        eth_tx_frame.type = ((len&0x00ff) << 8)  + ((len&0xff00) >> 8);
    }
#else
    eth_tx_frame.type = ((len&0x00ff) << 8)  + ((len&0xff00) >> 8);
    memcpy(eth_tx_frame.data, buf, len);
#endif

    ethStatus = Cy_EthIf_TransmitFrame(ETH_REG_BASE, (uint8_t*)&eth_tx_frame, len+14, CY_ETH_QS0_0, true);

    Cy_EthIF_ClearReleasedBuf(true, true);

    return ethStatus;
}

uint16_t ethernet_get_data(uint8_t* buf, uint16_t size)
{
    uint16_t len = fifo_get(&eth_rx_fifo, buf, size);

    return len;
}

