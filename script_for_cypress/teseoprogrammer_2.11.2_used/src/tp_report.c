/*******************************************************************************
*                            (C) 2017 STMicroelectronics
*    Reproduction and Communication of this document is strictly prohibited
*      unless specifically authorized in writing by STMicroelectronics.
*-----------------------------------------------------------------------------
*                                  APG / CRM / SA&PD
*                   Software Development Group - SW platform & HW Specific
******************************************************************************/

/*****************************************************************************
includes
*****************************************************************************/

#include "tp_common.h"
#include "tp_report.h"

/*****************************************************************************
external declarations
*****************************************************************************/

/*****************************************************************************
defines and macros (scope: module-local)
*****************************************************************************/

/*****************************************************************************
typedefs and structures (scope: module-local)
*****************************************************************************/

typedef struct tp_print_manager_s
{
	tPChar      string;
	tUInt       type;
	FILE *      output_handle;
	tPChar      output_filename;
} tp_print_manager_t;

/*****************************************************************************
global variable definitions  (scope: module-exported)
*****************************************************************************/

/*****************************************************************************
global variable definitions (scope: module-local)
*****************************************************************************/
 tp_general_status_t tp_general_status;
 boolean_t GeneralAbort = false;

/*****************************************************************************
function prototypes (scope: module-local)
*****************************************************************************/


/*****************************************************************************
function implementations (scope: module-local)
*****************************************************************************/

tVoid tp_report_init()
{
	_clibs_memset(&tp_general_status, 0, sizeof(tp_general_status_t));

	GeneralAbort = false;
}

tVoid tp_report_status(tp_prgdev_instance_t instance, Status_item_e item, Status_e state, int progression)
{
	// parse all previous status disable it
	for (Status_item_e _item = ITEM_INIT; _item < item; _item++)
	{
		if (tp_general_status.status[_item].used == true &&
			tp_general_status.status[item].state == STATE_RUN)
		{
			tp_general_status.status[item].state = STATE_SUCCESS;
		}
	}

	if (item < Status_item_e_SIZE)
	{	
		tp_general_status.status[item].used = true;
		tp_general_status.status[item].state = state;
		tp_general_status.status[item].progression = progression;
	}

	if (GeneralAbort == true)
	{
		tp_general_status.status[ITEM_OPERATION].used = true;
		tp_general_status.status[ITEM_OPERATION].state = STATE_ABORTED;
		tp_general_status.status[ITEM_OPERATION].progression = 100;
	}
}

tp_progression_t * tp_report_read(tp_prgdev_instance_t instance, Status_item_e item)
{
	if (item < Status_item_e_SIZE)
	{
		return &tp_general_status.status[item];
	}
	else
	{
		return &tp_general_status.status[ITEM_FLASH];
	}
}


