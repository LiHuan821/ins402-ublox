################################################################################
# platform.mk
# 
# HISTORY
# 17/06/2021  |                                             | Daich
################################################################################

PLATFORM_MAKEFILES_ROOTDIR:=${MAKEFILES_ROOTDIR}/modules/platform

include ${PLATFORM_MAKEFILES_ROOTDIR}/BSP/BSP.mk
include ${PLATFORM_MAKEFILES_ROOTDIR}/Core/Core.mk
include ${PLATFORM_MAKEFILES_ROOTDIR}/cypress_sdk/cypress_sdk.mk
include ${PLATFORM_MAKEFILES_ROOTDIR}/CYT4BF/CYT4BF.mk
include ${PLATFORM_MAKEFILES_ROOTDIR}/driver/driver.mk
include ${PLATFORM_MAKEFILES_ROOTDIR}/utility/utility.mk
include ${PLATFORM_MAKEFILES_ROOTDIR}/time_switch/time_switch.mk

PLATFORM_CINCDIRS_SPLIT:= ./
PLATFORM_CINCDIRS_SPLIT+=   \
							${BSP_CINCDIRS} ${Core_CINCDIRS} 	\
							${CYPRESS_SDK_CINCDIRS} ${CYT4BF_CINCDIRS} 	\
							${DRIVER_CINCDIRS} 		\
							${UTILITY_CINCDIRS} ${TIME_SWITCH_CINCDIRS}
PLATFORM_CINCDIRS:=  $(foreach head_file,$(strip ${PLATFORM_CINCDIRS_SPLIT} ), ./platform/${head_file})

${info 1111111111111111111111111111111111111111 ${PLATFORM_CINCDIRS}}

PLATFORM_CPP_SOURCES= 

PLATFORM_C_SOURCES_SPLIT+=  \
							${BSP_C_SOURCES} ${Core_C_SOURCES} 	\
							${CYPRESS_SDK_C_SOURCES} ${CYT4BF_C_SOURCES} 	\
							${DRIVER_C_SOURCES} 	\
							${UTILITY_C_SOURCES} ${TIME_SWITCH_C_SOURCES}
PLATFORM_C_SOURCES:= $(foreach c_file,$(strip ${PLATFORM_C_SOURCES_SPLIT} ), ./platform/${c_file})


PLATFORM_CDEFS:=   			\
							${BSP_CDEFS} ${Core_CDEFS} 	\
							${CYPRESS_SDK_CDEFS} ${CYT4BF_CDEFS} 	\
							${DRIVER_CDEFS} 	\
							${UTILITY_CDEFS} ${TIME_SWITCH_CDEFS}

PLATFORM_ASMSOURCES_SPLIT:=		${CYT4BFBCHD_M7_1_ASMSOURCES}
PLATFORM_ASMSOURCES:=  $(foreach asm_file,$(strip ${PLATFORM_ASMSOURCES_SPLIT} ), ./platform/${asm_file})

PLATFORM_ASMINCDIRS_SPLIT:=		${CYT4BFBCHD_M7_1_ASMINCDIRS}
PLATFORM_ASMINCDIRS:=  $(foreach asm_head_file,$(strip ${PLATFORM_ASMINCDIRS_SPLIT} ), ./platform/${asm_head_file})

PLATFORM_ASMDEFS:=				${CYT4BFBCHD_M7_1_ASMDEFS}

PLATFORM_LIBS_SPLIT:=			${CYT4BFBCHD_M7_1_LIBS}
PLATFORM_LIBS:=  $(foreach lib_file,$(strip ${PLATFORM_LIBS_SPLIT} ), ./platform/${lib_file})
