#ifndef _RTCM_H_
#define _RTCM_H_

#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "rtklib_core.h"
#include "rtkcmn.h"

#ifndef WIN32
#define ARM_MCU
#endif

//#define _DEBUG_RFS_ON_
//#define _DEBUG_EPVT_ON_
//#define _DEBUG_SIGQM2_ON_
//#define _DEBUG_OBSQM_ON_
//#define _DEBUG_POSQM_ON_
//#define _DEBUG_IONOPAR_ON_
//#define _DEBUG_STGBS_ON_
//#define _DEBUG_STGRS_ON_
//#define _DEBUG_STGST_ON_

#pragma pack(push, 1)
typedef struct e2e_raw {
	uint8_t flag;
	uint8_t header_len;
	uint8_t header[16];
	uint32_t length;
	uint8_t buff[1280];
}e2e_raw;

typedef struct e2e_header {
	uint16_t length;
	uint16_t counter;
	uint32_t data_id;
	uint32_t crc;
}e2e_header;
#pragma pack(pop)

void input_e2e_preamble(uint8_t data);
int input_e2e_data(uint8_t data);

/*-----------------------------------------------------------*/
/* from rtklib to decode RTCM3 */
#define RTCM2PREAMB 0x66 /* rtcm ver.2 frame preamble */
#define RTCM3PREAMB 0xD3 /* rtcm ver.3 frame preamble */

#define E2E_ON

typedef struct {
    /* move the observation data struct out of rtcm definiton, to save more memory for PPP only mode */
    obs_t obs[MAXSTN];
    rtcm_t rcv[MAXSTN];
    nav_t  nav;
	double time;
} gnss_rtcm_t;

RTK_RAM_CODE extern void set_week_number(int week);

RTK_RAM_CODE extern int get_week_number();

int decode_rtcm3(rtcm_t *rtcm, obs_t *obs, nav_t *nav);
int input_rtcm3_data(rtcm_t *rtcm, unsigned char data, obs_t *obs, nav_t *nav);

/* interface to GNSS db */
int input_rtcm3(unsigned char data, unsigned int stnID, gnss_rtcm_t *gnss);

unsigned int rtcm_getbitu(const unsigned char *buff, int pos, int len);

/* glo frquent number function */
extern void set_glo_frq(int prn, int frq);

extern int get_glo_frq(int prn);

int satno(int sys, int prn);

/* satellite function */
int  satsys(int sat, int *prn);
int  satidx(int sat, int *prn);
char satid (int sat, int *prn);
char sys2char(int sys);
extern double satwavelen(int sat, int code);

extern double satwavelenbyfreq(int sat, int frq);

unsigned char obs2code(int sys, const char * obs, int * freq);
unsigned char obs2coderinex(int sys, const char *obs, int *freq);

extern int code2frq(int sys, int code);

int getcodepri(int sys, unsigned char code, const char * opt);

void ecef2pos(const double *r, double *pos);
void pos2ecef(const double *pos, double *r);

void set_approximate_time(int year, int doy, rtcm_t *rtcm);

int add_obs(obsd_t* obsd, obs_t* obs);
int add_eph(eph_t* eph, nav_t* nav);
int add_geph(geph_t* eph, nav_t* nav);

int is_complete_rtcm(rtcm_t *rtcm, unsigned char data);
//int gen_rtcm3(rtcm_t* rtcm, obs_t *obs, int type, int sync);

#ifdef __cplusplus
}
#endif
#endif
