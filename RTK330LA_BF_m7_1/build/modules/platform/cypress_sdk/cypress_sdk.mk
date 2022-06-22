################################################################################
# cypress_sdk.mk
# 
# HISTORY
# 17/06/2021  |                                             | Daich
################################################################################


CYPRESS_SDK_CINCDIRS:=     	\
	./cypress_sdk/flash		\
	./cypress_sdk/power		\
	./cypress_sdk/sw_timer

CYPRESS_SDK_CPP_SOURCES=   \

CYPRESS_SDK_C_SOURCES+= 	\
	./cypress_sdk/flash/cy_mw_flash.c	\
	./cypress_sdk/power/cy_power.c		\
	./cypress_sdk/sw_timer/cy_sw_tmr.c

CYPRESS_SDK_CDEFS:= \
	__CHAR_BITS__=8
