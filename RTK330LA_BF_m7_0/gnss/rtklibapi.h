#ifndef __RTKLIB_API_H_
#define __RTKLIB_API_H_

#include <stdint.h>

#define RTK_ENABLE

#ifdef RTK_ENABLE
#define RTK_RAM_CODE              	__attribute__((section("RTK_ALO_REGION")))
#define RTK_ITCM_CODE				__attribute__((section("RTK_ITCM_REGION")))
#define RTK_DTCM_DATA				__attribute__((section("RTK_DTCM_REGION")))
#else
#define RTK_RAM_CODE    	
#define RTK_ITCM_CODE	
#define RTK_DTCM_DATA
#endif // RTK_ENABLE

enum
{
    RTKSOL_RETV_NONE = 0,
    RTKSOL_RETV_ROVER_VALID = 1,
    RTKSOL_RETV_BASE_VALID = 2,
    RTKSOL_RETV_SOL_VALID = 3
};

typedef struct
{
	char pn[20];                     /* product number*/
	char sn[20];                     /* serial number*/
	char firmver[20];                /* firmware version*/
	char sys[8];                     /* system priority */
	int32_t gnss_mode;               /* position engine mode: 1:SPP, 2:RTD, 4:RTK fixed, 5:RTK float */
	int32_t ins_aid;                 /* turn on/off ins aiding rtk */
	int32_t wl_amb_fixed;            /* wide-lane fixing enabled*/
	int32_t glo_amb_fixed;           /* glonass ambiguity fixing enabled*/
	int32_t gnss_sol_fde;            /* exclude abnormal gnss sol*/
	int32_t gnss_integrity;          /* RTK integrity enabled*/
	int32_t ins_integrity;           /* RTK integrity enabled*/
	int32_t min_satellite;           /* min satellite */
	double gnss_rate;                /* gnss solution rate */
	double max_hdop;                 /* max hdop */
	double max_age;                  /* max correction age */
	double max_hor_delta_pos_thres;  /* max horizontal position jump threhold(m) */
	double max_ver_delta_pos_thres;  /* max vertical position jump threhold(m) */
	double max_hor_vel_thres;        /* max horizontal velocity threhold(m/s)  */
	double max_ver_vel_thres;        /* max vertical velocity threhold(m/s)  */
	double pos_hor_alert_limit;      /* horizontal position alert limit(m)  */
	double pos_ver_alert_limit;      /* vertical position alert limit(m)  */
	double pos_lat_alert_limit;      /* lateral position alert limit(m)  */
	double pos_lon_alert_limit;      /* longitu position alert limit(m)  */
	double vel_hor_alert_limit;      /* horizontal velocity alert limit(m/s)*/
	double vel_ver_alert_limit;      /* vertical velocity alert limit(m/s)*/
	double vel_lat_alert_limit;      /* lateral velocity alert limit(m/s) */
	double vel_lon_alert_limit;      /* longitu velocity alert limit(m/s) */
	double roll_alert_limit;         /* roll alert limit(deg)*/
	double pitch_alert_limit;        /* pitch alert limit(deg)*/
	double heading_alert_limit;      /* heading alert limit(deg)*/
} cfg_t;

typedef struct {
	int32_t week;                   /* GPS Week number */
	double tow;                     /* second of week */
	uint8_t ins_status;             /* Solution status */
	uint8_t pos_type;               /* Position type */
	double latitude;			    /* latitude - WGS84 (deg) */
	double longitude;			    /* longitude - WGS84 (deg) */
	double height;				    /* Height above mean sea level - WGS84 (m) */
    double north_velocity;		    /* velocity in a northerly direction (m/s) */
	double east_velocity;		    /* velocity in an easterly direction  (m/s) */
	double up_velocity;			    /* velocity in an up direction (m/s) */
	double roll;			        /* roll angle */
	double pitch;				    /* pitch angle */
	double heading;				    /* heading angle */	 
    float latitude_std;             /* latitude standard deviation */
	float longitude_std;            /* longitude standard deviation */
	float height_std;               /* altitude standard deviation */
	float north_velocity_std;       /* north velocity standard deviation*/
	float east_velocity_std;        /* east velocity standard deviation*/
	float up_velocity_std;          /* up velocity standard deviation*/
	float roll_std;                 /* standard deviation of roll angle */
	float pitch_std;                /* standard deviation of pitch angle */
	float heading_std;              /* standard deviation of heading angle */
} gnss_insaid_sol_t;

typedef struct {
	double flag;                     /* flag of available */
	int32_t week;                    /* GPS Week number */
	double tow;                      /* second of week */
	uint8_t fixType;                 /* 0:NGNSS  1:spp, 2:PSR,  4:fixed,5:float */
	double HDOP;                     /* Horizontal dilution of precision */
	double VDOP;                     /* Vertical dilution of precision */
	double TDOP;                     /* Time dilution of precision */
	double PDOP;                     /* Position dilution of precision */
	float sol_age;                   /* solution age */
	uint8_t nsat_view;               /* number of satellite in view */
	uint8_t nsat_use;                /* number of satellite in solution*/

	double longitude;	             /* degrees */
	double latitude;	             /* degrees */
	double height;	                 /* ellipsoidal height - WGS84 (m) */
	float  geo_sep;	                 /* Geoidal separation, meters */

	float north_vel;	             /* m/s */
	float east_vel;	                 /* m/s */
	float up_vel;		             /* m/s */

	float longitude_std;	         /* longitude standard deviation */
	float latitude_std;	             /* latitude standard deviation */
	float height_std;	             /* altitude standard deviation */

	float north_vel_std;	         /* north velocity standard deviation*/
	float east_vel_std;              /* east velocity standard deviation*/
	float up_vel_std;                /* up velocity standard deviation*/

	float hor_pos_pl;                /* horizontal position protection level */
	float ver_pos_pl;                /* vertical position protection level */
	float hor_vel_pl;                /* horizontal velocity protection level */
	float ver_vel_pl;                /* vertical velocity protection level */
	uint8_t pos_pl_status;           /* status of position protection level */
	uint8_t vel_pl_status;           /* status of velocity protection level */

	int32_t fwver;
} gnss_sol_t;

typedef struct {
   char gga[512];                    /* string of gga */
   char zda[512];                    /* string of zda */
   char sol[1024];                   /* string of solution */
   char posvel[1024];                /* string of position & velocity */
} gnss_out_t;

extern uint8_t rtkcore_initfromconfig(const cfg_t* pcfg);
extern void rtkcore_set_output(gnss_sol_t* gs_t, gnss_out_t* go_t);
extern uint8_t rtkcore_input_rover_rtcm(uint8_t* data, uint16_t length);
extern uint8_t rtkcore_input_base_rtcm(uint8_t* data, uint16_t length);
extern void rtkcore_add_inssol(gnss_insaid_sol_t* insaidsol);


#endif /* __RTKLIB_API_H_ */
