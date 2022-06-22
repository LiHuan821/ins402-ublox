/*******************************************************************************
*                            (C) 2017 STMicroelectronics
*    Reproduction and Communication of this document is strictly prohibited
*      unless specifically authorized in writing by STMicroelectronics.
*-----------------------------------------------------------------------------
*                                  APG / CRM / SA&PD
*                   Software Development Group - SW platform & HW Specific
******************************************************************************/

#if !defined( TP_REPORT_H )
#define TP_REPORT_H

/*****************************************************************************
includes
*****************************************************************************/

#include "typedefs.h"
#include "clibs.h"

/*****************************************************************************
defines and macros (scope: module-local)
*****************************************************************************/

#if defined _USRDLL
#define TP_REPORT(instance,item,state,prog)    tp_report_status(instance,item,state,prog)
#define TP_REPORT_INIT()              tp_report_init()
#define TP_REPORT_READ(instance,item)          tp_report_read(instance,item)
#define TP_REPORT_OK(instance,item)            tp_report_status(instance,item,STATE_SUCCESS,100)
#define TP_REPORT_KO(instance,item)            tp_report_status(instance,item,STATE_FAILED,0)
#define TP_REPORT_PROGRESS(instance,item,prog) tp_report_status(instance,item,STATE_RUN,prog)
#define TP_REPORT_START(instance,item)         tp_report_status(instance,item,STATE_RUN,0)
#else
#define TP_REPORT(instance,item,state,prog)
#define TP_REPORT_INIT()
#define TP_REPORT_READ(instance,item)
#define TP_REPORT_OK(instance,item)
#define TP_REPORT_KO(instance,item)
#define TP_REPORT_PROGRESS(instance,item,prog)
#define TP_REPORT_START(instance,item)
#endif

/*****************************************************************************
typedefs and structures (scope: module-local)
*****************************************************************************/

/*****************************************************************************
exported variables
*****************************************************************************/

/*****************************************************************************
exported function prototypes
*****************************************************************************/
#if defined _USRDLL
tVoid               tp_report_init    ();
tVoid               tp_report_status  (tp_prgdev_instance_t instance, Status_item_e item, Status_e state, int progression);
tp_progression_t *  tp_report_read    (tp_prgdev_instance_t instance, Status_item_e item);
#endif

#endif /* TP_REPORT_H */
