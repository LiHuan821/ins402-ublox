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


BUILD_OUTPUT              :=  bootloader_output


# Path to APP root dir
INS401_BOOTLOADER_ROOTDIR:=$(realpath $(dir $(realpath $(lastword ${MAKEFILE_LIST})))../)

# Absolute path to makefiles
MAKEFILES_ROOTDIR:=${INS401_BOOTLOADER_ROOTDIR}/build

# Absolute path to output
OUTPUT_ROOTDIR:=${INS401_BOOTLOADER_ROOTDIR}/build/output


# including product options
#


include ${MAKEFILES_ROOTDIR}/modules/build.mk


.PHONY : all_test

all: prebuild build 


