################################################################################
#
# macros.mk
#
# This makefiles defines all variables and macros used by build makefiles
#
################################################################################

################################################################################
#
# Constants section
#
################################################################################

ifeq (${MACROS_MAKE},)
#$(warning First call)
MACROS_MAKE:=included

# Build process constants
MK_COMMA:= ,
MK_EMPTY:=
MK_SPACE:= ${MK_EMPTY} ${MK_EMPTY}

################################################################################
#
# Macros section
#
################################################################################

# convert path to win path
# 1 - Paths to convert
MK_PATHTOWIN=$(subst /,\,${1})

# convert path to unix path
# 1 - Paths to convert
MK_PATHTOUNX=$(subst \,/,${1})

# Convert spaces to commas
# 1 - string to convert
MK_SPCTOCOM=$(subst ${MK_SPACE},${MK_COMMA},${1})

# Busybox executable
#
MK_BUSYBOX=$(call MK_PATHTOWIN,${PROJ_TOOLSDIR}/busybox.exe)

# Convert non alphanumeric characters to underlines, remove trailing _ and convert to lower case
# 1 - string to convert

ifneq "$(PLATFORM_OS)" "LINUX"
MK_FMTNAME=$(shell @${MK_BUSYBOX} echo ${1} | @${MK_BUSYBOX} sed "s/[^a-zA-Z0-9_]/_/g" | @${MK_BUSYBOX} sed "s/_*$$//g" | @${MK_BUSYBOX} tr "[:upper:]" "[:lower:])
else
MK_FMTNAME=$(shell  echo ${1} |  sed "s/[^a-zA-Z0-9_]/_/g" |  sed "s/_*$$//g" |  tr "[:upper:]" "[:lower:])
endif
# Convert a windows path to 8.3 format
# 1 - string to convert
MK_SHORTNAME=$(shell ${PROJ_TOOLSDIR}/pathtoshort.bat ${1})

# check if file exists
# 1 - File to check
MK_FILEEXIST=$(wildcard $(subst ", ,${1}))

# Echo a string
# 1 - files to type/append
ifneq "$(PLATFORM_OS)" "LINUX"
MK_ECHO=$(if ${1},@${MK_BUSYBOX} echo ${1})
else
MK_ECHO=$(if ${1}, echo ${1})
endif

# type files/append files to another one
# 1 - files to type/append
# 2 (opt) - destination file
ifneq "$(PLATFORM_OS)" "LINUX"
MK_CAT=$(if $(and ${1},${2}),@${MK_BUSYBOX} cat ${1} >> ${2})
else
MK_CAT=$(if $(and ${1},${2}), cat ${1} >> ${2})
endif
# create a folder if it does not exist
# 1 - folder name
ifneq "$(PLATFORM_OS)" "LINUX"
MK_MKDIR=$(if ${1},@${MK_BUSYBOX} mkdir -p $(1))
else
MK_MKDIR=$(if ${1}, mkdir -p $(subst \,/,${1}))
endif

# remove files
# 1 - files to remove
ifneq "$(PLATFORM_OS)" "LINUX"
MK_RMFILE=$(if ${1},@${MK_BUSYBOX} rm -f ${1})
else
MK_RMFILE=$(if ${1}, rm -f ${1})
endif

# remove directory
# 1 - dir to remove
ifneq "$(PLATFORM_OS)" "LINUX"
MK_RMDIR=$(if ${1},@${MK_BUSYBOX} rm -rf ${1})
else
MK_RMDIR=$(if ${1}, rm -rf ${1})
endif

# copy files
# 1 - source file(s)
# 2 - destination file/folder
ifneq "$(PLATFORM_OS)" "LINUX"
MK_CP=$(if $(and ${1},${2}),@${MK_BUSYBOX} cp ${1} ${2})
else

endif
# append text to a file
# 1 - text to append
# 2 - destination file
ifneq "$(PLATFORM_OS)" "LINUX"
MK_APPEND=$(if $(and ${1},${2}),@${MK_BUSYBOX} echo ${1}>>${2})
else
MK_APPEND=$(if $(and ${1},${2}), echo ${1}>>${2})
endif
# convert text to lowercase
# 1 - text to convert
ifneq "$(PLATFORM_OS)" "LINUX"
MK_LOWER=$(if ${1},$(shell @${MK_BUSYBOX} echo ${1} | ${MK_BUSYBOX} tr '[:upper:]' '[:lower:]' ))
else
MK_LOWER=$(if ${1},$(shell  echo ${1} |  tr '[:upper:]' '[:lower:]' ))
endif
# convert text to uppercase
# 1 - text to convert
ifneq "$(PLATFORM_OS)" "LINUX"
MK_UPPER=$(if ${1},$(shell @${MK_BUSYBOX} echo ${1} | ${MK_BUSYBOX} tr '[:lower:]' '[:upper:]' ))else
else
MK_UPPER=$(if ${1},$(shell  echo ${1} |  tr '[:lower:]' '[:upper:]' ))
endif

else
#$(warning Nested call)
endif

