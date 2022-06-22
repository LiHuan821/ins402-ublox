################################################################################
# tool.mk
# 
# HISTORY
# 17/06/2021  |                                             | Daich
################################################################################


TOOL_CINCDIRS:=    \
	./tool/include
	
TOOL_CPP_SOURCES=   \

TOOL_C_SOURCES+= 	\
	./tool/src/handle_packet.c	\
	./tool/src/platform.c	\
	./tool/src/send_packet.c	
TOOL_CDEFS:= \
	__CHAR_BITS__=8