#ifndef RTKCMN_H
#define RTKCMN_H

#include "rtklib_core.h"
#include <time.h>
#include <string.h>
#include <stdint.h>
#ifdef WIN32
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/* time and string functions -------------------------------------------------*/
int     str2time(const char *s, int i, int n, gtime_t *t);
void    time2str(gtime_t t, char *str, int n);
RTK_RAM_CODE gtime_t epoch2time(const double *ep);
RTK_RAM_CODE void    time2epoch(gtime_t t, double *ep);
RTK_RAM_CODE gtime_t gpst2time(int week, double sec);
RTK_RAM_CODE double  time2gpst(gtime_t t, int *week);
RTK_RAM_CODE gtime_t gst2time(int week, double sec);
RTK_RAM_CODE double  time2gst(gtime_t t, int *week);
RTK_RAM_CODE gtime_t bdt2time(int week, double sec);
RTK_RAM_CODE double  time2bdt(gtime_t t, int *week);
char    *time_str(gtime_t t, int n);
RTK_RAM_CODE gtime_t timeadd(gtime_t t, double sec);
RTK_RAM_CODE double  timediff(gtime_t t1, gtime_t t2);
RTK_RAM_CODE gtime_t gpst2utc(gtime_t t);
RTK_RAM_CODE gtime_t utc2gpst(gtime_t t);
RTK_RAM_CODE gtime_t gpst2bdt(gtime_t t);
RTK_RAM_CODE gtime_t bdt2gpst(gtime_t t);

int adjgpsweek(gtime_t * time, int week);
int adjbdtweek(gtime_t * time, int week);
void adjday_glot(gtime_t * time, double tod);
void adjweek(gtime_t *time, double tow);

unsigned int rtk_crc24q(const unsigned char *buff, int len);
/* satellites, systems, codes functions --------------------------------------*/

int  satno(int sys, int prn);
int  satsys(int sat, int *prn);

extern int cmpsat(const void* p1, const void* p2);

/* multiply matrix -----------------------------------------------------------*/
RTK_RAM_CODE void matmul(const char* tr, uint32_t n, uint32_t k, uint32_t m, double alpha,
	const double* A, const double* B, double beta, double* C);

extern void ecef2enu(const double* pos, const double* r, double* e);

extern void xyz2enu_(const double* pos, double* E);

extern void enu2ecef(const double* pos, const double* e, double* r);

extern void covecef(const double* pos, const double* Q, double* P);

extern void xyz2enu(const double* pos, double* E);

#ifdef __cplusplus
}
#endif

#endif
