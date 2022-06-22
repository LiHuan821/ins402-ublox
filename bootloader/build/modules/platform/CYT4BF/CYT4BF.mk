################################################################################
# CYT4BF.mk
# 
# HISTORY
# 17/06/2021  |                                             | Daich
################################################################################


CYT4BF_CINCDIRS:=     	\
	./CYT4BF/board								\
	./CYT4BF/board/ip							\
	./CYT4BF/board/mcureg						\
	./CYT4BF/cmsis/include						\
	./CYT4BF/hal_common							\
	./CYT4BF/hal_common/adc						\
	./CYT4BF/hal_common/canfd					\
	./CYT4BF/hal_common/crypto					\
	./CYT4BF/hal_common/dma						\
	./CYT4BF/hal_common/evtgen					\
	./CYT4BF/hal_common/flash					\
	./CYT4BF/hal_common/gpio					\
	./CYT4BF/hal_common/ipc						\
	./CYT4BF/hal_common/lin						\
	./CYT4BF/hal_common/lvd						\
	./CYT4BF/hal_common/mcwdt					\
	./CYT4BF/hal_common/mpu						\
	./CYT4BF/hal_common/prot					\
	./CYT4BF/hal_common/scb						\
	./CYT4BF/hal_common/smartio					\
	./CYT4BF/hal_common/srom					\
	./CYT4BF/hal_common/sysflt					\
	./CYT4BF/hal_common/sysint					\
	./CYT4BF/hal_common/syslib					\
	./CYT4BF/hal_common/syspm					\
	./CYT4BF/hal_common/sysreset				\
	./CYT4BF/hal_common/sysrtc					\
	./CYT4BF/hal_common/systick					\
	./CYT4BF/hal_common/syswdt					\
	./CYT4BF/hal_common/tcpwm					\
	./CYT4BF/hal_common/trigmux					\
	./CYT4BF/hal_drivers						\
	./CYT4BF/hal_drivers/audioss				\
	./CYT4BF/hal_drivers/canfd					\
	./CYT4BF/hal_drivers/cpu					\
	./CYT4BF/hal_drivers/crypto					\
	./CYT4BF/hal_drivers/ethernet				\
	./CYT4BF/hal_drivers/flexray				\
	./CYT4BF/hal_drivers/sd_host				\
	./CYT4BF/hal_drivers/smif					\
	./CYT4BF/hal_drivers/sysclk					\
	./CYT4BF/hal_drivers/sysflt					\
	./CYT4BF/hal_drivers/sysreghc				\
	./CYT4BF/interrupts							\
	./CYT4BF/startp/include						\
	./CYT4BF/system/include						\
	./CYT4BF/startup/include

CYT4BF_CPP_SOURCES=  \

CYT4BF_C_SOURCES+=														\
	./CYT4BF/hal_common/adc/cy_adc.c									\
	./CYT4BF/hal_common/canfd/cy_canfd.c								\
	./CYT4BF/hal_common/crypto/cy_crypto_config.c						\
	./CYT4BF/hal_common/dma/cy_mdma.c									\
	./CYT4BF/hal_common/dma/cy_pdma.c									\
	./CYT4BF/hal_common/evtgen/cy_evtgen.c								\
	./CYT4BF/hal_common/flash/cy_flash.c								\
	./CYT4BF/hal_common/gpio/cy_gpio.c									\
	./CYT4BF/hal_common/ipc/cy_ipc_drv.c								\
	./CYT4BF/hal_common/ipc/cy_ipc_pipe.c								\
	./CYT4BF/hal_common/ipc/cy_ipc_sema.c								\
	./CYT4BF/hal_common/lin/cy_lin.c									\
	./CYT4BF/hal_common/lvd/cy_lvd.c									\
	./CYT4BF/hal_common/mcwdt/cy_mcwdt.c								\
	./CYT4BF/hal_common/mpu/cy_mpu.c									\
	./CYT4BF/hal_common/prot/cy_prot.c									\
	./CYT4BF/hal_common/scb/cy_scb_common.c								\
	./CYT4BF/hal_common/scb/cy_scb_ezi2c.c								\
	./CYT4BF/hal_common/scb/cy_scb_i2c.c								\
	./CYT4BF/hal_common/scb/cy_scb_spi.c								\
	./CYT4BF/hal_common/scb/cy_scb_uart.c								\
	./CYT4BF/hal_common/smartio/cy_smartio.c							\
	./CYT4BF/hal_common/srom/cy_srom.c									\
	./CYT4BF/hal_common/sysflt/cy_sysflt.c								\
	./CYT4BF/hal_common/sysint/cy_sysint.c								\
	./CYT4BF/hal_common/syslib/cy_syslib.c								\
	./CYT4BF/hal_common/syspm/cy_syspm.c								\
	./CYT4BF/hal_common/sysreset/cy_sysreset.c							\
	./CYT4BF/hal_common/sysrtc/cy_sysrtc.c								\
	./CYT4BF/hal_common/systick/cy_systick.c							\
	./CYT4BF/hal_common/syswdt/cy_syswdt.c								\
	./CYT4BF/hal_common/tcpwm/cy_tcpwm.c								\
	./CYT4BF/hal_common/tcpwm/cy_tcpwm_counter.c						\
	./CYT4BF/hal_common/tcpwm/cy_tcpwm_pwm.c							\
	./CYT4BF/hal_common/tcpwm/cy_tcpwm_quaddec.c						\
	./CYT4BF/hal_common/tcpwm/cy_tcpwm_sr.c								\
	./CYT4BF/hal_common/trigmux/cy_trigmux.c							\
	./CYT4BF/hal_drivers/audioss/cy_i2s.c								\
	./CYT4BF/hal_drivers/cpu/cy_cpu.c									\
	./CYT4BF/hal_drivers/ethernet/cy_ethif.c							\
	./CYT4BF/hal_drivers/ethernet/edd.c									\
	./CYT4BF/hal_drivers/ethernet/edd_rx.c								\
	./CYT4BF/hal_drivers/ethernet/edd_tx.c								\
	./CYT4BF/hal_drivers/flexray/api/cy_fr_api_control_service.c		\
	./CYT4BF/hal_drivers/flexray/api/cy_fr_api_init.c					\
	./CYT4BF/hal_drivers/flexray/api/cy_fr_api_interrupt_service.c		\
	./CYT4BF/hal_drivers/flexray/api/cy_fr_api_rx_handler.c				\
	./CYT4BF/hal_drivers/flexray/api/cy_fr_api_status_service.c			\
	./CYT4BF/hal_drivers/flexray/api/cy_fr_api_time_service.c			\
	./CYT4BF/hal_drivers/flexray/api/cy_fr_api_timer_service.c			\
	./CYT4BF/hal_drivers/flexray/api/cy_fr_api_tx_handler.c				\
	./CYT4BF/hal_drivers/flexray/ccal/cy_fr_ccal_control_service.c		\
	./CYT4BF/hal_drivers/flexray/ccal/cy_fr_ccal_init.c					\
	./CYT4BF/hal_drivers/flexray/ccal/cy_fr_ccal_interrupt_service.c	\
	./CYT4BF/hal_drivers/flexray/ccal/cy_fr_ccal_rx_handler.c			\
	./CYT4BF/hal_drivers/flexray/ccal/cy_fr_ccal_status_service.c		\
	./CYT4BF/hal_drivers/flexray/ccal/cy_fr_ccal_time_service.c			\
	./CYT4BF/hal_drivers/flexray/ccal/cy_fr_ccal_timer_service.c		\
	./CYT4BF/hal_drivers/flexray/ccal/cy_fr_ccal_tx_handler.c			\
	./CYT4BF/hal_drivers/sd_host/cy_sd_host.c							\
	./CYT4BF/hal_drivers/smif/cy_smif.c									\
	./CYT4BF/hal_drivers/smif/cy_smif_memslot.c							\
	./CYT4BF/hal_drivers/sysclk/cy_sysclk.c								\
	./CYT4BF/hal_drivers/sysreghc/cy_sysreghc.c							\
	./CYT4BF/interrupts/cy_interrupt_map.c								\
	./CYT4BF/startup/src/startup.c										\
	./CYT4BF/system/src/system_tviibh8m_cm0plus.c
	

CYT4BFBCHD_M0_ASMSOURCES:=							\
	./CYT4BF/startup/src/startup_cm0plus.s			\
	./CYT4BF/hal_common/syslib/iar/cy_syslib_iar.s	\
	
CYT4BFBCHD_M7_1_ASMINCDIRS:=					\
	./CYT4BF/startp/include						\
	./CYT4BF/hal_common/syslib
CYT4BFBCHD_M7_1_ASMDEFS:=

CYT4BFBCHD_M7_1_LIBS:=							\
	./CYT4BF/hal_drivers/crypto/libcrypto_server_cm0plus_iar.a
	
CYT4BF_CDEFS:= \
	__CHAR_BITS__=8
