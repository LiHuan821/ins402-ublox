################################################################################
# Makefile
# 
# HISTORY
# 17/06/2021  |                                             | Daich
################################################################################

ifndef TG
	TG=WINDOWS
	COMPILER=IAR
endif

ifndef APPEND_COMPILE_MESSAGE
	APPEND_COMPILE_MESSAGE = FALSE
endif
BUILD_OUTPUT              :=  rtk_output


# Path to APP root dir
INS401_RTK_APP_ROOTDIR:=$(realpath $(dir $(realpath $(lastword ${MAKEFILE_LIST})))../)

# Absolute path to makefiles
MAKEFILES_ROOTDIR:=${INS401_RTK_APP_ROOTDIR}/build

# Absolute path to output
OUTPUT_ROOTDIR:=${INS401_RTK_APP_ROOTDIR}/build/output


# including product options
#


include ${MAKEFILES_ROOTDIR}/modules/build.mk


.PHONY : all_test

all: prebuild build 


