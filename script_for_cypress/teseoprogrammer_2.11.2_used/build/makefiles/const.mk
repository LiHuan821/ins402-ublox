################################################################################
#
# common.mk
#
# This makefiles defines all constants used in makefiles
#
################################################################################

ifeq (${CONST_MAKE},)
#$(warning First call)
CONST_MAKE      :=  included

# Module constants
MOD_LNKLSTEXT   :=  txt

#
# Derived constants
#

# name of project
PROJ_NAME             :=  $(notdir ${CURDIR:%/=%})

# folder for project files
PROJ_PATH             :=  $(realpath ${CURDIR:%/=%})

endif
