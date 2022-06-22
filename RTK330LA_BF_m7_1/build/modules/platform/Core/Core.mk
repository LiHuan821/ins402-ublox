################################################################################
# Core.mk
# 
# HISTORY
# 03/08/2021  |                                             | Daich
################################################################################


Core_CINCDIRS:=    			\
	./Core/configuration	\
	./Core/runstatus		\
	./Core/maintain_ins		\
	./Core/ucbpacket

Core_CPP_SOURCES=   \

Core_C_SOURCES+= 	\
	./Core/configuration/user_config.c	\
	./Core/maintain_ins/maintain_ins.c				\
	./Core/runstatus/run_status.c		\
	./Core/ucbpacket/ucb_packet.c

DRIVER_CDEFS:= \
	__CHAR_BITS__=8