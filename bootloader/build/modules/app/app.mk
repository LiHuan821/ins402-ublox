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
	./app/main_cm0.c


APP_CDEFS:= 				\
	tviibh8m				\
	CYT4BFBCHS				\
	CY_USE_PSVP=0			\
	CY_MCU_rev_b			\
	CPU_BOARD_REVA			\
	__ICCARM__				\
	__CORE__=__ARM6M__
