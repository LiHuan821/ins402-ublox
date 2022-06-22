################################################################################
# app.mk
# 
# HISTORY
# 17/06/2021  |                                             | Daich
################################################################################


APP_CINCDIRS:=    		\
	./app/include		\
	./app/user

APP_C_SOURCES+= 					\
	./app/taskINS.c					\
	./app/main_cm7_1.c


APP_CDEFS:= 			\
	tviibh8m			\
	char_t=char			\
	float32_t=float		\
	float64_t=double	\
	CYT4BFBCHS			\
	CY_USE_PSVP=0		\
	CY_MCU_rev_b		\
	CPU_BOARD_REVA		\
	CY_CORE_CM7_1		\
	BT_ENABLE=0			\
	GT_PACKET=1			\
	_LINK_flash_=0		\
	__ICACHE_PRESENT=1	\
	__DCACHE_PRESENT=1
