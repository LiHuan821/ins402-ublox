#
# compiler defs for MINGW toolchan
#

include build/makefiles/macros.mk

################################################################################
#
# This part should be modified only if needed
#
################################################################################

#
# This is the default installation path for GAE. Change it if different
#
TC_PATH_INST_MINGW:=C:\STGNSSTools\compilers\mingw32

#
# GAE related section
#
TC_PATH_BIN:=$(call MK_SHORTNAME,"${TC_PATH_INST_MINGW}\bin")
TC_PATH_INC:=$(call MK_SHORTNAME,"${TC_PATH_INST_MINGW}\include")
TC_PATH_LIB:=$(call MK_SHORTNAME,"${TC_PATH_INST_MINGW}\lib")

#
# toolchain executables
#
#TC_MAKEDEP:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/gcc.exe -MM)
#TC_CC:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/g++.exe -x c)
#TC_CPP:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/g++.exe  -x c++)
#TC_LINK:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/g++.exe)

TC_MAKEDEP:=gcc -MM -std=gun99 -std=c99
TC_CC:=g++ -x c
TC_CPP:=g++  -x c++
TC_LINK:=g++


#
# toolchain switches macros
#
TC_ASM_VIA=@${1}
TC_CC_VIA=@${1}
TC_LINK_VIA=@${1}
TC_LINK_LIBUSE=
TC_LINK_ENTRY=-e ${1}
TC_LINK_SCF=-T ${1}
TC_LINK_LIST=-Wl,-Map=${1}

#
# constants
#

# toolchain identifiers
TC_NAME:=mingw
TC_SUFFIX:=exe

# Assembly opcodes type
TC_ASMTYPE:=gnu

# Assembly compiler options
ifneq "$(PLATFORM_OS)" "LINUX"
TC_ASMOPTS:=\
  -c                    \
  -std=c99				\
  -O2
else
TC_ASMOPTS:=\
   -c			\
   -O2   
endif
# Assembly compiler defines
TC_ASMDEFS:=

# C compiler options

ifneq "$(PLATFORM_OS)" "LINUX"
TC_COPTS:=\
  -c                        \
  -std=c99					\
  -O2 
else
TC_COPTS:=\
  -c			\
  -O2
endif
# C compiler defines

ifneq "$(PLATFORM)" "LINUX"
TC_CDEFS:= WINDOWS
else
TC_CDEFS:=LINUX
endif
# Scatter file extension
TC_SCFEXT:=map
TC_LIBEXT:=gcc4x

# Linker options
TC_LIBOPTS:=\
  -O2                       \
  -g                        \
  -static-libgcc            \
  -static-libstdc++         \
  -Wl,--cref                \
  -Wl,--no-warn-mismatch    \
  -Wl,--gc-section          

# Linker libraries
TC_LIBNAMES:=\
  m                     \
  stdc++

#
# toolchain macros
#

# convert PATH to toolchain friendly path
MK_TC_PATH=$(call MK_PATHTOUNX,${1})

# command to generate list of linker directory search paths
MK_TC_LIBDIRS=$(if ${1},$(foreach dir,${1},-L${dir}))

# command to generate list of linker directory search paths
MK_TC_LIBS=$(if ${1},$(foreach lib,${1},-l${lib}))

