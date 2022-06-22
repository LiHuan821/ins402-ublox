/** ******************************************************************************
 * @file platformAPI.h API functions for Interfacing with xbow library funcctions
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 *****************************************************************************/
/*******************************************************************************
Copyright 2018 ACEINNA, INC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*******************************************************************************/


#ifndef _PLATFORM_API_H
#define _PLATFORM_API_H
#include <stdint.h>
#include "eth_process.h"

uint32_t cal_get_unit_serialnum(void);

const uint8_t* cal_get_unit_version(void);

void platformGetVersionBytes(uint8_t *bytes);

void process_user_commands(void);

void ucb_write_app_by_ethernet(uint8_t core_to_upgrade,binary_packet_format_t* pbiprx);
#endif
