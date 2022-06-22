################################################################################
# gnss.mk
# 
# HISTORY
# 17/06/2021  |                                             | Daich
################################################################################


GNSS_CINCDIRS:=    		\
	./gnss/include		\
	./gnss/RTCM/include		\
	./gnss


GNSS_C_SOURCES+= 					\
	./gnss/RTCM/src/rtcm.c	\
	./gnss/RTCM/src/rtkcmn.c	\
	./gnss/src/taskGnssDataAcq.c	\
	./gnss/src/taskRTK.c


RTKLIBS:= ${INS401_RTK_APP_ROOTDIR}/gnss/gnss_lib.a
