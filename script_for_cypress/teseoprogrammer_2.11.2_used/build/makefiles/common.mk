################################################################################
#
# common.mk
#
# This makefiles defines all variables and macros used by build makefiles
#
################################################################################

include build/makefiles/macros.mk

################################################################################
#
# Constants section
#
################################################################################

################################################################################
#
# Variable section
#
################################################################################

#
# derived variables
#

#
# folder defs
#

# folder for sdk root
PROJ_ROOTDIR:=$(realpath $(call MK_PATHTOUNX,${PROJ_PATH}))

# folder for tools
PROJ_TOOLSDIR:=${PROJ_ROOTDIR}/build/tools

# folder for build common files
PROJ_COMMONMK:=${PROJ_ROOTDIR}/build/makefiles

# folder for common includes
COMMON_CINCDIR:=include

# folder for common includes
COMMON_LIBDIR:=libs

# folder for binary outputs
COMMON_BINDIR:=${PROJ_ROOTDIR}/bin

# folder for target files
TARGET_ROOTDIR:=${PROJ_ROOTDIR}/out

# objects folder
OBJS_FOLDER:=${TARGET_ROOTDIR}/objs

# Project related defines
TCDEFSINC:=${PROJ_COMMONMK}/tc_mingw_defs.mk

#
# include toolchains definitions
#

include ${TCDEFSINC}

#
# include specific target defs
#

# Assembler options file
ASMOPTS_FILE:=${TARGET_ROOTDIR}/asmopts.via

# C options file
COPTS_FILE:=${TARGET_ROOTDIR}/copts.via

# C++ options file
CPPOPTS_FILE:=${TARGET_ROOTDIR}/cppopts.via

# Linker options file
LOPTS_FILE:=${TARGET_ROOTDIR}/lopts.via

# definitions for target outputs
TARGET_SRC_FILENAME_EXE:= sdk_upgrade_app
TARGET_SRC_FILENAME_EXE:=${COMMON_BINDIR}/${PROJ_NAME}.${TC_SUFFIX}
TARGET_SRC_FILENAME_LIST:=${TARGET_ROOTDIR}/${PROJ_NAME}.${MOD_LNKLSTEXT}

#
# Build sources list
#
PROJ_ASMSOURCES:=${ASMSOURCES} ${EXT_ASMSOURCES}

PROJ_CSOURCES:=${CSOURCES} ${EXT_CSOURCES}

PROJ_CPPSOURCES:=${CPPSOURCES} ${EXT_CPPSOURCES}

PROJ_CARMSOURCES:=${CARMSOURCES} ${EXT_CARMSOURCES}

PROJ_SOURCES:=${PROJ_ASMSOURCES} ${PROJ_CSOURCES} ${PROJ_CARMSOURCES}

#
# assembler section
#
ASMOPTS:=$(strip ${TC_ASMOPTS} ${CORE_ASMOPTS} ${PROJ_ASMOPTS} ${EXT_ASMOPTS})

ASMDEFS:=$(strip ${TC_ASMDEFS} ${PROJ_ASMDEFS} ${OS_ASMDEFS} ${EXT_ASMDEFS})

ASMINCDIRS+=$(foreach inc,$(strip ${COMMON_CINCDIR}/${MOD_CORE}/${TC_ASMTYPE} ${TC_ASMINCDIRS} ${PROJ_ASMINCDIRS} ${OS_ASMINCDIRS} ${EXT_ASMINCDIRS}),-I$(call MK_TC_PATH,${PROJ_ROOTDIR}/${inc}))

#
# compiler section
#

COPTS:=$(strip ${TC_COPTS} ${CORE_COPTS} ${PROJ_COPTS} ${EXT_COPTS})

CPPOPTS:=$(strip ${TC_COPTS} ${CORE_COPTS} ${PROJ_COPTS} ${EXT_COPTS})

CDEFS:=$(foreach def,$(strip ${TC_CDEFS} ${PROJ_CDEFS} ${OS_CDEFS} ${EXT_CDEFS}),-D${def})

CINCDIRS+=$(foreach inc,$(strip ${COMMON_CINCDIR}/${MOD_CORE} ${TC_CINCDIRS} ${PROJ_CINCDIRS} ${OS_CINCDIRS} ${EXT_CINCDIRS}),-I$(call MK_TC_PATH,${PROJ_ROOTDIR}/${inc}))

#
# linker section
#

# build libs list to link for RVCT
LIBOPTS:=$(strip ${TC_LIBOPTS} ${EXT_LIBOPTS})

LIBDIRS:=$(foreach inc,$(strip ${COMMON_LIBDIR} ${TC_LIBDIRS} ${PROJ_LIBDIRS} ${EXT_LIBDIRS}),$(call MK_TC_PATH,${PROJ_ROOTDIR}/${inc}))

LIBS:=${EXT_LIBS}
LIBS+=$(foreach lib,${LIBNAMES},${MOD_CORE}_${MOD_ARCH}_${lib}_${TC_LIBEXT})
LIBS+=${TC_LIBNAMES}
