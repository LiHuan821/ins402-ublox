################################################################################
# BSP.mk
# 
# HISTORY
# 03/08/2021  |                                             | Daich
################################################################################


BSP_CINCDIRS:=    	\
	./BSP/can			\
	./BSP/eeprom		\
	./BSP/ethernet		\
	./BSP/gnss			\
	./BSP/imu			\
	./BSP/rs232			\
	./BSP/sta9100		\
	./BSP/watchdog

BSP_CPP_SOURCES=   \

BSP_C_SOURCES+= 					\
	./BSP/can/car_data.c			\
	./BSP/eeprom/eeprom.c			\
	./BSP/ethernet/eth_process.c	\
	./BSP/gnss/gnss.c				\
	./BSP/imu/Imu330Eth.c			\
	./BSP/imu/imu330za.c			\
	./BSP/rs232/rs232.c				\
	./BSP/sta9100/sta9100_control.c	\
	./BSP/sta9100/sta9100_upgrade.c	\
	./BSP/watchdog/watchdog.c



BSP_CDEFS:= \
	__CHAR_BITS__=8