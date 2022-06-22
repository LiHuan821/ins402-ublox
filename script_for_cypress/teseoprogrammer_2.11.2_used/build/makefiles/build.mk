################################################################################
#
# build.mk
#
# Build rules to generate exe file from sources
#
################################################################################

include build/makefiles/const.mk

include build/makefiles/common.mk

################################################################################
#
# Constants section
#
################################################################################

################################################################################
#
# Macros section
#
################################################################################

# Filter a source file from a list.
# As vpath is used to add c/s/asm sources paths to make search folders, it could
# happen that a file with same name is present in more folders. To avoid
# building the wrong one, this macro can filter a sources list and return the
# proper file in the list.
# 1 - source to look for (with or without path)
# 2 - target sources lists
MK_LOOKUPSRC=$(filter %/$(notdir ${1}),${2})

################################################################################
#
# Variable section
#
################################################################################

# C sources
BUILD_CSOURCES:=$(foreach src,${PROJ_CSOURCES},${PROJ_ROOTDIR}/$(src))

# C++ sources
BUILD_CPPSOURCES:=$(foreach src,${PROJ_CPPSOURCES},${PROJ_ROOTDIR}/$(src))

# Assembly sources
BUILD_ASMSOURCES:=$(foreach src,${PROJ_ASMSOURCES},${PROJ_ROOTDIR}/$(src))

# All sources
BUILD_SOURCES:=${BUILD_CSOURCES} ${BUILD_CPPSOURCES} ${BUILD_CARMSOURCES} ${BUILD_ASMSOURCES}

# All dependency files
BUILD_DEPENDS:=$(patsubst %.c,%.d,${BUILD_CSOURCES} ${BUILD_CARMSOURCES})

# All sources
PROJ_OBJS:=$(foreach obj,$(basename $(notdir ${BUILD_SOURCES})),${OBJS_FOLDER}/${obj}.o)

# Add sources paths to makefile search paths specific for each pattern
vpath %.cpp $(dir ${BUILD_CPPSOURCES})
vpath %.c $(dir ${BUILD_CSOURCES})
vpath %.s $(dir ${BUILD_ASMSOURCES})
vpath %.asm $(dir ${BUILD_ASMSOURCES})

################################################################################
#
# Targets section
#
################################################################################

#
# Folders targets
#
${TARGET_ROOTDIR}:
	$(call MK_MKDIR,"$(call MK_PATHTOWIN,$@)")

${OBJS_FOLDER}: ${TARGET_ROOTDIR}
	echo test0
	$(call MK_MKDIR,"$(call MK_PATHTOWIN,$@)")

${COMMON_BINDIR}:
	echo test1
	$(call MK_MKDIR,"$(call MK_PATHTOWIN,$@)")

#
# Via files targets
#
${SCF_FILE_NAME}: ${CONFIG_FILE} ${SCF_TEMPLATE_FILE_NAME} ${DEFSINCLIST}
	$(call MK_ECHO,Generating scatter file for ${TC_RVCT_NAME} builder)
	$(call MK_RMFILE,${SCF_FILE_NAME})
	$(call MK_TC_GENSCF,"${SCF_TEMPLATE_FILE_NAME}","${SCF_FILE_NAME}",${SCF_DEFS})

${ASMOPTS_FILE}: ${CONFIG_FILE} ${DEFSINCLIST} ${PROJ_MAKEFILE}
	$(call MK_ECHO,Generating assembler via file for ${TC_RVCT_NAME} builder)
	$(call MK_RMFILE,${ASMOPTS_FILE})
	$(call MK_APPEND,${ASMOPTS},"${ASMOPTS_FILE}")
	$(call MK_APPEND,${ASMDEFS},"${ASMOPTS_FILE}")
	$(call MK_APPEND,${ASMINCDIRS},"${ASMOPTS_FILE}")

${CPPOPTS_FILE}: ${CONFIG_FILE} ${DEFSINCLIST} ${PROJ_MAKEFILE}
	$(call MK_ECHO,Generating C++ compiler via file for ${TC_RVCT_NAME} builder)
	$(call MK_RMFILE,${CPPOPTS_FILE})
	$(call MK_APPEND,${CPPOPTS},"${CPPOPTS_FILE}")
	$(call MK_APPEND,${CDEFS},"${CPPOPTS_FILE}")
	$(call MK_APPEND,${CINCDIRS},"${CPPOPTS_FILE}")

${COPTS_FILE}: ${CONFIG_FILE} ${DEFSINCLIST} ${PROJ_MAKEFILE}
	$(call MK_ECHO,Generating C compiler via file for ${TC_RVCT_NAME} builder)
	$(call MK_RMFILE,${COPTS_FILE})
	$(call MK_APPEND,${COPTS},"${COPTS_FILE}")
	$(call MK_APPEND,${CDEFS},"${COPTS_FILE}")
	$(call MK_APPEND,${CINCDIRS},"${COPTS_FILE}")

${LOPTS_FILE}: ${CONFIG_FILE} ${DEFSINCLIST} ${PROJ_MAKEFILE}
	$(call MK_ECHO,Generating linker via file for ${TC_RVCT_NAME} builder)
	$(call MK_RMFILE,${LOPTS_FILE})
	$(call MK_APPEND,${LIBOPTS},"${LOPTS_FILE}")
	$(call MK_APPEND,$(call MK_TC_LIBDIRS,${LIBDIRS}),"${LOPTS_FILE}")
	$(call MK_APPEND,$(call MK_TC_LIBS,${LIBS}),"${LOPTS_FILE}")

#
# Compiler implicit targets
#
#${OBJS_FOLDER}/%.o: %.c ${OBJS_FOLDER}/%.d
${OBJS_FOLDER}/%.o: %.cpp ${CPPOPTS_FILE}
	$(eval SRCFILE:=$(call MK_LOOKUPSRC,$^,${BUILD_CPPSOURCES} ${BUILD_CARMSOURCES}))
	$(eval SRCMODE:=$(if $(findstring $<,${BUILD_CARMSOURCES}),${TC_TARGETARM},${TC_TARGETTHUMB}))
	$(call MK_ECHO,Compiling ${SRCFILE})
	@${TC_CPP} $(call TC_CC_VIA,${TARGET_ROOTDIR}/cppopts.via) ${SRCMODE} \
	  -o $@ \
	     ${SRCFILE}

${OBJS_FOLDER}/%.o: %.c ${COPTS_FILE}
	$(eval SRCFILE:=$(call MK_LOOKUPSRC,$^,${BUILD_CSOURCES} ${BUILD_CARMSOURCES}))
	$(eval SRCMODE:=$(if $(findstring $<,${BUILD_CARMSOURCES}),${TC_TARGETARM},${TC_TARGETTHUMB}))
	$(call MK_ECHO,Compiling ${SRCFILE})
	${TC_CC} $(call TC_CC_VIA,${TARGET_ROOTDIR}/copts.via) ${SRCMODE} \
	  -o $@ \
	     ${SRCFILE}

${OBJS_FOLDER}/%.o: %.s ${ASMOPTS_FILE}
	$(eval SRCFILE:=$(call MK_LOOKUPSRC,$^,${BUILD_ASMSOURCES}))
	$(call MK_ECHO,Assembling ${SRCFILE})
	@${TC_ASM} $(call TC_ASM_VIA,${TARGET_ROOTDIR}/asmopts.via) \
	  -o $@ \
	     ${SRCFILE}

${OBJS_FOLDER}/%.o: %.asm ${ASMOPTS_FILE}
	$(eval SRCFILE:=$(call MK_LOOKUPSRC,$^,${BUILD_ASMSOURCES}))
	$(call MK_ECHO,Assembling ${SRCFILE})
	@${TC_ASM} $(call TC_ASM_VIA,${TARGET_ROOTDIR}/asmopts.via) \
	  -o $@ \
	     ${SRCFILE}

#
# Executable target
#
TARGET_SRC_FILENAME_EXE:= sdk_upgrade_app
${TARGET_SRC_FILENAME_EXE}: ${LOPTS_FILE} ${OBJS_FOLDER} ${COMMON_BINDIR} ${PROJ_OBJS}
	$(call MK_ECHO,Linking to $(subst ${PROJ_ROOTDIR}/,,$@))
	@${TC_LINK} -o $@ ${PROJ_OBJS} \
	  $(call TC_LINK_VIA,${TARGET_ROOTDIR}/lopts.via) \
	  $(call TC_LINK_LIST,${TARGET_SRC_FILENAME_LIST})

#
# internal secondary targets
#
.SECONDARY: generatemk build postbuild clean distclean program debug

FORCE:

# pre build target
prebuild: ${TARGET_ROOTDIR}

# build makefile target
TARGET_SRC_FILENAME_EXE:= sdk_upgrade_app
build: ${TARGET_SRC_FILENAME_EXE}

# apply default configuration target
postbuild:

#
# exported targets
#
clean:
	$(call MK_ECHO,Executing clean on target ${PROJ_CFG})
	$(call MK_RMFILE,${PROJ_OBJS})
	$(call MK_RMFILE,${TARGET_SRC_FILENAME_EXE})
	$(call MK_RMFILE,${TARGET_SRC_FILENAME_LIST})

distclean:
	$(call MK_ECHO,Executing distclean on target ${PROJ_CFG})
	$(call MK_RMDIR,${TARGET_ROOTDIR})

