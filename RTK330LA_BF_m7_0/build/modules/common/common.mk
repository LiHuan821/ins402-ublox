################################################################################
# common.mk
# 
# HISTORY
# 17/06/2021  |                                             | Daich
################################################################################
include ${MAKEFILES_ROOTDIR}/modules/common/macros.mk
include ${MAKEFILES_ROOTDIR}/modules/app/app.mk
include ${MAKEFILES_ROOTDIR}/modules/mem_share/mem_share.mk
include ${MAKEFILES_ROOTDIR}/modules/gnss/gnss.mk
include ${MAKEFILES_ROOTDIR}/modules/platform/platform.mk


TC_COPTS:=\
  --debug                 \
  --endian=little         \
  --cpu=Cortex-M7         \
  -e                      \
  --fpu=VFPv5_D16         \
  --no_size_constraints	  \
  -Ohs

#  -std=c11               \
#  -MD                    \
  -MF                   
#  -ferror-limit=0       
#  -fbracket-depth=512   

CORE_COPTS:=            \
                     

CORE_CPPOPTS:=          \



TC_ASMOPTS:=            \
  -s+                   \
  -M\<\>                \
  -w+                   \
  -r                    \
  --cpu Cortex-M7       \
  --fpu VFPv5_d16

# folder for tools
TOOLS_DIR:=${INS401_RTK_APP_ROOTDIR}/build/tools


# folder for target files
OUTPUT_DIR:=${OUTPUT_ROOTDIR}
TARGET_ROOTDIR:=${OUTPUT_ROOTDIR}/${BUILD_OUTPUT}


# folder for common includes
COMMON_LIBDIR:=


#
# Build sources list
#

PROJ_CSOURCES:=   ${APP_C_SOURCES} ${GNSS_C_SOURCES} ${PLATFORM_C_SOURCES}


PROJ_CPPSOURCES:= ${APP_CPP_SOURCES} ${GNSS_CPP_SOURCESS} ${PLATFORM_CPP_SOURCES}

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


PROJ_CINCDIRS:= ${APP_CINCDIRS} ${GNSS_CINCDIRS} ${PLATFORM_CINCDIRS} ${MEM_SHARE_CINCDIRS}
CINCDIRS:=$(foreach inc,$(strip ${PROJ_CINCDIRS} ),-I$(call MK_TC_PATH,${INS401_RTK_APP_ROOTDIR}/${inc}))
${info ${CINCDIRS} }

ASMDEFS:=$(foreach def,$(strip ${CYT4BF_CDEFS} ),-D${def})
ASMINCDIRS:=$(foreach inc,$(strip ${CYT4BFBCHD_M7_1_ASMINCDIRS} ),-I$(call MK_TC_PATH,${INS401_RTK_APP_ROOTDIR}/${inc}))


ifeq "$(TG)" "LINUX"
ALLLIBS:= -lm
else
ALLLIBS:= \

endif
# C options file
COPTS_FILE:=${TARGET_ROOTDIR}/gccflags.txt

ifeq "$(TG)" "WINDOWS"
TARGET:=${TARGET_ROOTDIR}/rtk.hex
TARGET_BIN:=${TARGET_ROOTDIR}/rtk.bin
else
TARGET:=${TARGET_ROOTDIR}/rtk.hex
endif


# objects folder
OBJS_FOLDER:=${TARGET_ROOTDIR}/objs

RTK_LINK_CDEFS:= 			  \
	_LINK_flash_=0		    \
	_CORE_cm7_0_=0

LINK_CDEFSS:=$(foreach def,$(strip ${RTK_LINK_CDEFS} ),--config_def ${def})

