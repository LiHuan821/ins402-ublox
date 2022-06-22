################################################################################
# utility.mk
# 
# HISTORY
# 17/06/2021  |                                             | Daich
################################################################################


UTILITY_CINCDIRS:=    \
	./utility/include
	
UTILITY_CPP_SOURCES=   \

UTILITY_C_SOURCES+= 				\
	./utility/src/crc16.c			\
	./utility/src/fifo.c
UTILITY_CDEFS:= 