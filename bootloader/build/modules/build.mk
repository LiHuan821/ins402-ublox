################################################################################
# build.mk
# 
# HISTORY
# 17/06/2021  |                                             | Daich
################################################################################

include ${MAKEFILES_ROOTDIR}/modules/common/common.mk

include ${MAKEFILES_ROOTDIR}/modules/common/compile.mk

# C thumb sources
BUILD_CSOURCES:=$(foreach src,${PLATFORM_C_SOURCES} ${APP_C_SOURCES} ${GNSS_C_SOURCES} ,${INS401_BOOTLOADER_ROOTDIR}/$(src))
BUILD_CPPSOURCES:=$(foreach src,${PROJ_CPPSOURCES},${INS401_BOOTLOADER_ROOTDIR}/$(src))

# C ARM sources
BUILD_CARMSOURCES:=$(foreach src,${CYT4BFBCHD_M0_ASMSOURCES},${INS401_BOOTLOADER_ROOTDIR}/$(src))

# Assembly sources
BUILD_ASMSOURCES:=$(foreach asm,${CYT4BFBCHD_M0_ASMSOURCES},${INS401_BOOTLOADER_ROOTDIR}/platform/$(asm))

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




-include ${OBJS_FOLDER}/%.d

$(PROJ_COBJS):${BUILD_CSOURCES}
	$(eval SRCFILE:=${filter %/${patsubst %.o,%.c,${notdir $@}} ,${BUILD_CSOURCES} })
	@$(call MK_ECHO,Compiling ${SRCFILE})
	@${TC_CC} ${SRCFILE} ${GCCFLAGS}  -o $@
#@$(call MK_ECHO,flags ${GCCFLAGS} )


$(PROJ_CPPOBJS):${BUILD_CPPSOURCES}
	${info ${PROJ_COBJS} ${PROJ_CPPOBJS} }
	$(eval SRCFILE:=${filter %${patsubst %.o,%.cpp,${notdir $@}} ,${BUILD_CPPSOURCES} })
	$(call MK_ECHO,Compiling ${SRCFILE} )
	${TC_CPP}  ${GCCFLAGS} ${SRCFILE}  -o $@


${PROJ_ASMOBJS}: ${BUILD_ASMSOURCES}
	$(eval SRCFILE:=${filter %${patsubst %.o,%.s,${notdir $@}} ,${BUILD_ASMSOURCES} })
	$(call MK_ECHO,Assembling ${SRCFILE})
	${TC_ASM} ${SRCFILE} ${ASMFLAGS} -o $@

ASMLIBS:= 	${MAKEFILES_ROOTDIR}/libs/dl6M_tln.a	\
			${MAKEFILES_ROOTDIR}/libs/m6M_tl.a		\
			${MAKEFILES_ROOTDIR}/libs/rt6M_tl.a		\
			${MAKEFILES_ROOTDIR}/libs/shb_l.a

APP_MAPS:= ${TARGET_ROOTDIR}/bootloader.map
LINK_FILE:= ${INS401_BOOTLOADER_ROOTDIR}/../linker/linker_directives_tviibh.icf

${TARGET}:  ${PROJ_COBJS}  ${PROJ_CPPOBJS} ${PROJ_ASMOBJS}
	$(call MK_ECHO,Linking to $(subst ${INS401_BOOTLOADER_ROOTDIR}/,,$@))
	@${TC_LINK} ${PROJ_COBJS} ${PROJ_CPPOBJS} ${PROJ_ASMOBJS}  \
		${ASMLIBS}				\
	   	--no_out_extension -o	\
      	$@						\
		${LINK_CDEFSS}			\
       	--map					\
       	${APP_MAPS}				\
       	--config ${LINK_FILE}	\
		--semihosting --entry __iar_program_start --vfe --text_out locale
	@${TC_BIN} --bin  ${TARGET} ${TARGET_BIN}


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




