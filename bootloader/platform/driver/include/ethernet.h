#ifndef _ETHERNET_H_
#define _ETHERNET_H_

#include "cy_project.h"
#include "cy_device_headers.h"
#include "fifo.h"

/********************************************************/
#define ETH_REG_BASE         ETH0

#define ETH_INTR_SRC         (eth_0_interrupt_eth_0_IRQn)
#define ETH_INTR_SRC_Q1      (eth_0_interrupt_eth_1_IRQn)
#define ETH_INTR_SRC_Q2      (eth_0_interrupt_eth_2_IRQn)

#define ETHx_TD0_PORT        GPIO_PRT18
#define ETHx_TD0_PIN         4
#define ETHx_TD0_PIN_MUX     P18_4_ETH0_TXD0

#define ETHx_TD1_PORT        GPIO_PRT18
#define ETHx_TD1_PIN         5
#define ETHx_TD1_PIN_MUX     P18_5_ETH0_TXD1

#define ETHx_TD2_PORT        GPIO_PRT18
#define ETHx_TD2_PIN         6
#define ETHx_TD2_PIN_MUX     P18_6_ETH0_TXD2

#define ETHx_TD3_PORT        GPIO_PRT18
#define ETHx_TD3_PIN         7
#define ETHx_TD3_PIN_MUX     P18_7_ETH0_TXD3

#define ETHx_TXER_PORT       GPIO_PRT18
#define ETHx_TXER_PIN        2
#define ETHx_TXER_PIN_MUX    P18_2_ETH0_TX_ER

#define ETHx_TX_CTL_PORT     GPIO_PRT18
#define ETHx_TX_CTL_PIN      1
#define ETHx_TX_CTL_PIN_MUX  P18_1_ETH0_TX_CTL

#define ETHx_RD0_PORT        GPIO_PRT19
#define ETHx_RD0_PIN         0
#define ETHx_RD0_PIN_MUX     P19_0_ETH0_RXD0

#define ETHx_RD1_PORT        GPIO_PRT19
#define ETHx_RD1_PIN         1
#define ETHx_RD1_PIN_MUX     P19_1_ETH0_RXD1

#define ETHx_RD2_PORT        GPIO_PRT19
#define ETHx_RD2_PIN         2
#define ETHx_RD2_PIN_MUX     P19_2_ETH0_RXD2

#define ETHx_RD3_PORT        GPIO_PRT19
#define ETHx_RD3_PIN         3
#define ETHx_RD3_PIN_MUX     P19_3_ETH0_RXD3

#define ETHx_RX_CTL_PORT     GPIO_PRT21
#define ETHx_RX_CTL_PIN      5
#define ETHx_RX_CTL_PIN_MUX  P21_5_ETH0_RX_CTL

#define ETHx_RX_ER_PORT      GPIO_PRT2
#define ETHx_RX_ER_PIN       2
#define ETHx_RX_ER_PIN_MUX   P2_2_ETH0_RX_ER

#define ETHx_TX_CLK_PORT     GPIO_PRT18
#define ETHx_TX_CLK_PIN      3
#define ETHx_TX_CLK_PIN_MUX  P18_3_ETH0_TX_CLK

#define ETHx_RX_CLK_PORT     GPIO_PRT23
#define ETHx_RX_CLK_PIN      3
#define ETHx_RX_CLK_PIN_MUX  P23_3_ETH0_RX_CLK

#define ETHx_REF_CLK_PORT    GPIO_PRT18
#define ETHx_REF_CLK_PIN     0
#define ETHx_REF_CLK_PIN_MUX P18_0_ETH0_REF_CLK

#define ETHx_MDC_PORT        GPIO_PRT3
#define ETHx_MDC_PIN         1
#define ETHx_MDC_PIN_MUX     P3_1_ETH0_MDC

#define ETHx_MDIO_PORT       GPIO_PRT3
#define ETHx_MDIO_PIN        0
#define ETHx_MDIO_PIN_MUX    P3_0_ETH0_MDIO

#define ETHx_RESET_PORT      GPIO_PRT2
#define ETHx_RESET_PIN       4
#define ETHx_RESET_PIN_MUX   P2_4_GPIO

#define ETHx_LAN_EN_PORT        GPIO_PRT16
#define ETHx_LAN_EN_PIN         1
#define ETHx_LAN_EN_PIN_MUX     P16_1_GPIO

#define ETHx_WAKEUP_IN_PORT     GPIO_PRT16
#define ETHx_WAKEUP_IN_PIN      2
#define ETHx_WAKEUP_IN_PIN_MUX  P16_2_GPIO

/*******************LAN8770R***********************/
/* External Register Control Register */
#define LAN87XX_EXT_REG_CTL                     (0x14)
#define LAN87XX_EXT_REG_CTL_RD_CTL              (0x1000)
#define LAN87XX_EXT_REG_CTL_WR_CTL              (0x0800)

/* External Register Read Data Register */
#define LAN87XX_EXT_REG_RD_DATA                 (0x15)

/* External Register Write Data Register */
#define LAN87XX_EXT_REG_WR_DATA                 (0x16)

/* Interrupt Source Register */
#define LAN87XX_INTERRUPT_SOURCE                (0x18)

/* Interrupt Mask Register */
#define LAN87XX_INTERRUPT_MASK                  (0x19)
#define LAN87XX_MASK_LINK_UP                    (0x0004)
#define LAN87XX_MASK_LINK_DOWN                  (0x0002)

/* phyaccess nested types */
#define	PHYACC_ATTR_MODE_READ		0
#define	PHYACC_ATTR_MODE_WRITE		1
#define	PHYACC_ATTR_MODE_MODIFY		2

#define	PHYACC_ATTR_BANK_SMI		0
#define	PHYACC_ATTR_BANK_MISC		1
#define	PHYACC_ATTR_BANK_PCS		2
#define	PHYACC_ATTR_BANK_AFE		3
#define	PHYACC_ATTR_BANK_MAX		7

struct access_ereg_val {
	uint8_t  mode;
	uint8_t  bank;
	uint8_t  offset;
	uint16_t val;
	uint16_t mask;
};

typedef enum
{
  ACCESS_SUCCESS = 0,
  ACCESS_FAILTURE = -1,
  PARAMETER_ERROR = -2
} phy_register_access_result;

/********************************************************/
enum {
    PYH_SLAVE  = 0,
    PYH_MASTER = 1
};

enum {
    ETHIF_STATUS_DOWN           = 0,
    ETHIF_STATUS_MAC_INIT_OK    = 1,
    ETHIF_STATUS_PHY_INIT_OK    = 2
};

enum {
    ETHIF_LINK_DOWN       = 0,
    ETHIF_LINK_UP         = 1
};


/********************************************************/
#pragma pack(1)

typedef struct {
    uint8_t phy_master_slave;
	uint8_t ethif_status;
    uint8_t ethif_link;
} ethernet_dev_struct;

#pragma pack()

extern ethernet_dev_struct ethernet_dev;


int32_t ethernet_init(void);
int32_t ethernet_check_link(void);
int32_t ethernet_send_data(uint8_t* buf, uint16_t len);
uint16_t ethernet_get_data(uint8_t* buf, uint16_t size);
static int access_ereg(uint8_t mode, uint8_t bank, uint8_t offset, uint16_t val);
static int access_ereg_modify_changed(uint8_t bank, uint8_t offset, uint16_t val, uint16_t mask);
static bool Phy_LAN8770M_MDIO_validation (void);
void get_ethernet_src_mac(uint8_t* mac);

#endif
