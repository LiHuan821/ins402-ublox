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
	./driver/src/ethernet.c	\
	./driver/src/uart.c
DRIVER_CDEFS:= \
	__CHAR_BITS__=8