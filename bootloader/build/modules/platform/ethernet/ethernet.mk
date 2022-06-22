################################################################################
# ethernet.mk
# 
# HISTORY
# 17/06/2021  |                                             | Daich
################################################################################


ETHERNET_CINCDIRS:=    \
	./ethernet
	
ETHERNET_CPP_SOURCES=   \

ETHERNET_C_SOURCES+= 	\
	./ethernet/eth_process.c

ETHERNET_CDEFS:= \
	__CHAR_BITS__=8