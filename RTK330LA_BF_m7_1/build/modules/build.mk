################################################################################
# build.mk
# 
# HISTORY
# 17/06/2021  |                                             | Daich
################################################################################

include ${MAKEFILES_ROOTDIR}/modules/common/common.mk

include ${MAKEFILES_ROOTDIR}/modules/common/compile.mk



# lib version
VERSION_INS_MAJOR      :=  1
VERSION_INS_MINOR      :=  0
VERSION_INS_PATCH      :=  0
VERSION_INS   :=  ${VERSION_INS_MAJOR}.${VERSION_INS_MINOR}.${VERSION_INS_PATCH}



# C thumb sources
BUILD_CSOURCES:=$(foreach src,${PLATFORM_C_SOURCES} ${APP_C_SOURCES} ${INS_C_SOURCES} ,${INS401_INS_APP_ROOTDIR}/$(src))
BUILD_CPPSOURCES:=$(foreach src,${PROJ_CPPSOURCES},${INS401_INS_APP_ROOTDIR}/$(src))

# C ARM sources
BUILD_CARMSOURCES:=$(foreach src,${CYT4BFBCHD_M7_1_ASMSOURCES},${INS401_INS_APP_ROOTDIR}/$(src))

# Assembly sources
BUILD_ASMSOURCES:=$(foreach asm,${CYT4BFBCHD_M7_1_ASMSOURCES},${INS401_INS_APP_ROOTDIR}/platform/$(asm))

# All sources
BUILD_CSOURCES:=${BUILD_CSOURCES} 
BUILD_CPPSOURCES:=${BUILD_CPPSOURCES} 


# All sources
PROJ_COBJS:=$(foreach obj,$(basename $(notdir ${BUILD_CSOURCES})),${OBJS_FOLDER}/${obj}.o)
PROJ_CPPOBJS:=$(foreach obj,$(basename $(notdir ${BUILD_CPPSOURCES})),${OBJS_FOLDER}/${obj}.o)
PROJ_ASMOBJS:=$(foreach obj,$(basename $(notdir ${BUILD_CARMSOURCES})),${OBJS_FOLDER}/${obj}.o)

GCCFLAGS = ${COPTS}  ${CINCDIRS}  ${CDEFS} ${ALLLIBS}

GPPFLAGS = ${CPPOPTS}  ${CINCDIRS}  ${CDEFS} ${ALLLIBS}

ASMFLAGS = ${ASMOPTS}  ${ASMINCDIRS}  ${ASMDEFS} 
################################################################################
#
# Targets section
#
################################################################################

# Via files targets
#

${COPTS_FILE}:  
	$(call MK_ECHO,Generating C compiler via file for ${PROJ_TC} builder)
	$(call MK_ECHO,${CDEFS})
	$(call MK_ECHO,${CINCDIRS})
	$(call MK_APPEND,${COPTS},"${COPTS_FILE}")
	$(call MK_APPEND,${CINCDIRS},"${COPTS_FILE}")
	$(call MK_APPEND,${CDEFS},"${COPTS_FILE}")

#
# Compiler implicit targets
#



# compile time

COMPILE_TIME = $(shell powershell Get-Date -Format "yyyy/MM/dd/HH/mm/ss")
COMPILER_AUTHOR = $(shell powershell Get-WmiObject -Class Win32_ComputerSystem \| Select-object -ExpandProperty UserName)

ifneq (${VERSION_INS},)
VERSION_CDEFS+=VERSION_INS_APP=${VERSION_INS}
VERSION_CDEFS+=TIME_INS_APP=${COMPILE_TIME}
VERSION_CDEFS+=AUTHOR_INS_APP=${COMPILER_AUTHOR}
CDEFS+= $(foreach def,$(strip ${VERSION_CDEFS} ),-D${def})
${info ins app version = ${VERSION_INS}}
endif
-include ${OBJS_FOLDER}/%.d

vpath %.c $(dir ${BUILD_CSOURCES} ${BUILD_CARMSOURCES})
#$(PROJ_COBJS):${BUILD_CSOURCES}
${OBJS_FOLDER}/%.o: %.c 
	$(eval SRCFILE:=${filter %/${patsubst %.o,%.c,${notdir $@}} ,${BUILD_CSOURCES} })
	@$(call MK_ECHO,Compiling ${SRCFILE})
	@${TC_CC} ${SRCFILE} ${GCCFLAGS}  -o $@
#@$(call MK_ECHO,flags ${GCCFLAGS} )


$(PROJ_CPPOBJS):${BUILD_CPPSOURCES}
#	${info ${PROJ_COBJS} ${PROJ_CPPOBJS} }
	$(eval SRCFILE:=${filter %${patsubst %.o,%.cpp,${notdir $@}} ,${BUILD_CPPSOURCES} })
	$(call MK_ECHO,Compiling ${SRCFILE} )
	${TC_CPP}  ${GCCFLAGS} ${SRCFILE}  -o $@


${PROJ_ASMOBJS}: ${BUILD_ASMSOURCES}
	$(eval SRCFILE:=${filter %${patsubst %.o,%.s,${notdir $@}} ,${BUILD_ASMSOURCES} })
	$(call MK_ECHO,Assembling ${SRCFILE})
	${TC_ASM} ${SRCFILE} ${ASMFLAGS} -o $@

ASMLIBS:= 	${MAKEFILES_ROOTDIR}/libs/dl7M_tln.a	\
			${MAKEFILES_ROOTDIR}/libs/m7M_tlv.a		\
			${MAKEFILES_ROOTDIR}/libs/rt7M_tl.a		\
			${MAKEFILES_ROOTDIR}/libs/shb_l.a

APP_MAPS:= ${TARGET_ROOTDIR}/ins.map
LINK_FILE:= ${INS401_INS_APP_ROOTDIR}/../linker/linker_directives_tviibh.icf

${TARGET}:  ${PROJ_COBJS}  ${PROJ_CPPOBJS} ${PROJ_ASMOBJS}
	$(call MK_ECHO,Linking to $(subst ${INS401_INS_APP_ROOTDIR}/,,$@))
	@${TC_LINK} ${PROJ_COBJS} ${PROJ_CPPOBJS} ${PROJ_ASMOBJS}  \
		${ASMLIBS}				\
		${INSLIBS}				\
	   	--no_out_extension -o	\
      	$@						\
		${LINK_CDEFSS}			\
       	--map					\
       	${APP_MAPS}				\
       	--config ${LINK_FILE}	\
		--semihosting --entry __iar_program_start --vfe --text_out locale
	@${TC_BIN} --bin  ${TARGET} ${TARGET_BIN}
ifeq "$(APPEND_COMPILE_MESSAGE)" "TRUE"
	$(call MK_APPEND,comiple by: ${COMPILER_AUTHOR},"${APP_MESSAGE_FILE}")
	$(call MK_APPEND,comiple time: ${COMPILE_TIME},"${APP_MESSAGE_FILE}")
	$(call MK_APPEND,verion: ins_app_${VERSION_INS},"${APP_MESSAGE_FILE}")
	$(call MK_APPEND,,"${APP_MESSAGE_FILE}")
endif

OBJS_FOLDER_WIN:= $(subst /,\,${OBJS_FOLDER})

prebuild:	
ifeq ($(call MK_FILEEXIST, $(OBJS_FOLDER)) ,  )
		$(call MK_MKDIR,"$(call MK_PATHTOWIN,${OBJS_FOLDER}))
else
		$(info exist )
endif


build:	${TARGET}



.PHONY : clean

clean: 
	$(call MK_RMDIR,$(call MK_PATHTOWIN,${OBJS_FOLDER}))
	$(call MK_RMDIR,$(call MK_PATHTOWIN,${TARGET}))
	$(call MK_RMDIR,$(call MK_PATHTOWIN,${TARGET_BIN}))
	$(call MK_RMDIR,$(call MK_PATHTOWIN,${APP_MAPS}))
all_test:  
	$(prebuild)




