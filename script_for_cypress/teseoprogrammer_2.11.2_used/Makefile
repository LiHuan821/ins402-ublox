#
# compiler defs for MINGW toolchan
#
MINGW_HOME = /usr
ifeq (${MINGW_HOME},)
${error MINGW_HOME not defined!}
endif

ifndef PLATFORM_OS
#PLATFORM_OS = LINUX
PLATFORM_OS = WINDOWS
endif
PATH:=${PATH};${MINGW_HOME}\bin

EXT_CINCDIRS+=src
EXT_CINCDIRS+=src/teseo2
EXT_CINCDIRS+=src/teseo3
EXT_CINCDIRS+=src/teseo5

# Files to be built
CSOURCES+=src/crc32.c
CSOURCES+=src/tp_main.c
CSOURCES+=src/tp_params.c
CSOURCES+=src/tp_print.c
CSOURCES+=src/tp_tools.c
ifneq "$(PLATFORM_OS)" "LINUX"
CSOURCES+=src/bsp/tp_uart_win.c
else
CSOURCES+=src/bsp/tp_uart_linux.c
endif
CSOURCES+=src/tp_help.c

CSOURCES+=src/teseo2/tp_teseo2.c
CSOURCES+=src/teseo2/xldr_teseo2.c

CSOURCES+=src/teseo3/tp_teseo3.c
CSOURCES+=src/teseo3/xldr_teseo3.c

CSOURCES+=src/teseo5/tp_teseo5.c
CSOURCES+=src/teseo5/xldr_teseo5.c

CSOURCES+=src/fwupg/fwupg.c
CSOURCES+=src/fwupg/tp_fwupg.c

#CPPSOURCES+=src/XLoaderCLI.c

include build/makefiles/build.mk

all: prebuild build postbuild
