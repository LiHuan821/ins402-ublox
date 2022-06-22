################################################################################
# ins.mk
# 
# HISTORY
# 17/06/2021  |                                             | Daich
################################################################################


INS_CINCDIRS:=    		\
	./ins				\
	./ins/include

INS_C_SOURCES+= 					\
	./ins/src/fw_ins.c


INSLIBS:= ${INS401_INS_APP_ROOTDIR}/ins/ins_lib.a