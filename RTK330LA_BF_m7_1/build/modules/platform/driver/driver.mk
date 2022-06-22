################################################################################
# driver.mk
# 
# HISTORY
# 17/06/2021  |                                             | Daich
################################################################################


DRIVER_CINCDIRS:=    \
	./driver/include
	
DRIVER_CPP_SOURCES=   \

DRIVER_C_SOURCES+= 	\
	./driver/src/can.c					\
	./driver/src/ethernet.c				\
	./driver/src/m7_ipc.c				\
	./driver/src/mcu_temperature.c		\
	./driver/src/time_ins.c				\
	./driver/src/time_ref.c				\
	./driver/src/uart.c

DRIVER_CDEFS:= \
	__CHAR_BITS__=8