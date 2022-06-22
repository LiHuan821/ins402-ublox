################################################################################
# eeprom.mk
# 
# HISTORY
# 17/06/2021  |                                             | Daich
################################################################################


EEPROM_CINCDIRS:=    	\
	./eeprom			\
	./eeprom/include
	
EEPROM_CPP_SOURCES=   \

EEPROM_C_SOURCES+= 	\
	./eeprom/src/eeprom.c

EEPROM_CDEFS:= \
	__CHAR_BITS__=8