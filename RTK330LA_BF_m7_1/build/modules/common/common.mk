################################################################################
# common.mk
# 
# HISTORY
# 17/06/2021  |                                             | Daich
################################################################################
include ${MAKEFILES_ROOTDIR}/modules/common/macros.mk
include ${MAKEFILES_ROOTDIR}/modules/app/app.mk
include ${MAKEFILES_ROOTDIR}/modules/mem_share/mem_share.mk
include ${MAKEFILES_ROOTDIR}/modules/ins/ins.mk
include ${MAKEFILES_ROOTDIR}/modules/platform/platform.mk


TC_COPTS:=\
  --endian=little         \
  --cpu=Cortex-M7         \
  --fpu=VFPv5_D16         \
  --no_cse                \
  --no_unroll             \
  --no_inline             \
  --no_code_motion        \
  --no_tbaa               \
  --no_clustering         \
  --no_scheduling         \
  --debug                 \
  -e                      

#  -std=c11               \
#  -MD                    \
  -MF                   
#  -ferror-limit=0       
#  -fbracket-depth=512   

CORE_COPTS:=            \
                     

CORE_CPPOPTS:=          \
  -O1       


TC_ASMOPTS:=            \
  -s+                   \
  -w+                   \
  -r                    \
  --cpu Cortex-M7       \
  --fpu VFPv5_d16       \
  -M\<\>

# folder for tools
TOOLS_DIR:=${INS401_INS_APP_ROOTDIR}/build/tools


# folder for target files
OUTPUT_DIR:=${OUTPUT_ROOTDIR}
TARGET_ROOTDIR:=${OUTPUT_ROOTDIR}/${BUILD_OUTPUT}


# folder for common includes
COMMON_LIBDIR:=


#
# Build sources list
#

PROJ_CSOURCES:=   ${APP_C_SOURCES} ${INS_C_SOURCES} ${PLATFORM_C_SOURCES}

PROJ_CPPSOURCES:= ${APP_CPP_SOURCES} ${INS_CPP_SOURCESS} ${PLATFORM_CPP_SOURCES}

PROJ_SOURCES:=    ${PROJ_CSOURCES} ${PROJ_CPPSOURCES}
#
# compiler section
#

COPTS:=$(strip ${TC_COPTS} ${CORE_COPTS})
CPPOPTS:=$(strip ${TC_COPTS} ${CORE_CPPOPTS} )
ASMOPTS:=$(strip ${TC_ASMOPTS} )

ifeq "$(TG)" "WINDOWS"
PLATFORM_CDEFS:= WINDOWS
else
PLATFORM_CDEFS:= LINUX
endif

CDEFS:=$(foreach def,$(strip ${PLATFORM_CDEFS} ${APP_CDEFS} ${CONFIG_CDEFS} ),-D${def})


PROJ_CINCDIRS:= ${APP_CINCDIRS} ${INS_CINCDIRS} ${PLATFORM_CINCDIRS} ${MEM_SHARE_CINCDIRS}
CINCDIRS:=$(foreach inc,$(strip ${PROJ_CINCDIRS} ),-I$(call MK_TC_PATH,${INS401_INS_APP_ROOTDIR}/${inc}))


ASMDEFS:=$(foreach def,$(strip ${CYT4BF_CDEFS} ),-D${def})
ASMINCDIRS:=$(foreach inc,$(strip ${CYT4BFBCHD_M7_1_ASMINCDIRS} ),-I$(call MK_TC_PATH,${INS401_INS_APP_ROOTDIR}/${inc}))


ifeq "$(TG)" "LINUX"
ALLLIBS:= -lm
else
ALLLIBS:= \

#${INS401_INS_APP_ROOTDIR}/platform/${CYT4BFBCHD_M7_1_LIBS}
endif
# C options file
COPTS_FILE:=${TARGET_ROOTDIR}/gccflags.txt

ifeq "$(TG)" "WINDOWS"
TARGET:=${TARGET_ROOTDIR}/ins.hex
TARGET_BIN:=${TARGET_ROOTDIR}/ins.bin
else
TARGET:=${TARGET_ROOTDIR}/ins.hex
endif


# objects folder
OBJS_FOLDER:=${TARGET_ROOTDIR}/objs

INS_LINK_CDEFS:= 			  \
	_LINK_flash_=0		    \
	_CORE_cm7_1_=0

LINK_CDEFSS:=$(foreach def,$(strip ${INS_LINK_CDEFS} ),--config_def ${def})

