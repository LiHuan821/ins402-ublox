################################################################################
# platform.mk
# 
# HISTORY
# 17/06/2021  |                                             | Daich
################################################################################

PLATFORM_MAKEFILES_ROOTDIR:=${MAKEFILES_ROOTDIR}/modules/platform

include ${PLATFORM_MAKEFILES_ROOTDIR}/cypress_sdk/cypress_sdk.mk
include ${PLATFORM_MAKEFILES_ROOTDIR}/CYT4BF/CYT4BF.mk
include ${PLATFORM_MAKEFILES_ROOTDIR}/driver/driver.mk
include ${PLATFORM_MAKEFILES_ROOTDIR}/eeprom/eeprom.mk
include ${PLATFORM_MAKEFILES_ROOTDIR}/ethernet/ethernet.mk
include ${PLATFORM_MAKEFILES_ROOTDIR}/sta9100/sta9100.mk
include ${PLATFORM_MAKEFILES_ROOTDIR}/tool/tool.mk
include ${PLATFORM_MAKEFILES_ROOTDIR}/utility/utility.mk

PLATFORM_CINCDIRS_SPLIT:= ./
PLATFORM_CINCDIRS_SPLIT+=   \
							${CYPRESS_SDK_CINCDIRS} ${CYT4BF_CINCDIRS} 	\
							${DRIVER_CINCDIRS} ${EEPROM_CINCDIRS} ${ETHERNET_CINCDIRS}	\
							${STA9100_CINCDIRS} ${TOOL_CINCDIRS} ${UTILITY_CINCDIRS}
PLATFORM_CINCDIRS:=  $(foreach head_file,$(strip ${PLATFORM_CINCDIRS_SPLIT} ), ./platform/${head_file})


PLATFORM_CPP_SOURCES= 

PLATFORM_C_SOURCES_SPLIT+=  \
							${CYPRESS_SDK_C_SOURCES} ${CYT4BF_C_SOURCES} 	\
							${DRIVER_C_SOURCES} ${EEPROM_C_SOURCES}	${ETHERNET_C_SOURCES}		\
							${STA9100_C_SOURCES} ${TOOL_C_SOURCES} ${UTILITY_C_SOURCES}
PLATFORM_C_SOURCES:= $(foreach c_file,$(strip ${PLATFORM_C_SOURCES_SPLIT} ), ./platform/${c_file})


PLATFORM_CDEFS:=   			\
	${CYPRESS_SDK_CDEFS} ${CYT4BF_CDEFS} 	\
	${DRIVER_CDEFS} ${EEPROM_CDEFS} ${ETHERNET_CDEFS}			\
	${STA9100_CDEFS} ${TOOL_CDEFS} ${UTILITY_CDEFS}

PLATFORM_ASMSOURCES_SPLIT:=		${CYT4BFBCHD_M0_ASMSOURCES}
PLATFORM_ASMSOURCES:=  $(foreach asm_file,$(strip ${PLATFORM_ASMSOURCES_SPLIT} ), ./platform/${asm_file})

PLATFORM_ASMINCDIRS_SPLIT:=		${CYT4BFBCHD_M7_1_ASMINCDIRS}
PLATFORM_ASMINCDIRS:=  $(foreach asm_head_file,$(strip ${PLATFORM_ASMINCDIRS_SPLIT} ), ./platform/${asm_head_file})

PLATFORM_ASMDEFS:=				${CYT4BFBCHD_M7_1_ASMDEFS}

PLATFORM_LIBS_SPLIT:=			${CYT4BFBCHD_M7_1_LIBS}
PLATFORM_LIBS:=  $(foreach lib_file,$(strip ${PLATFORM_LIBS_SPLIT} ), ./platform/${lib_file})
