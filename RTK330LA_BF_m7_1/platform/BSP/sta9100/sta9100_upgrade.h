/** ***************************************************************************
 * @file   sta9100_upgrade.h
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 ******************************************************************************/
/*******************************************************************************
Copyright 2020 ACEINNA, INC

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
#ifndef _STA9100_UPGRADE_H_
#define _STA9100_UPGRADE_H_

#include <stdint.h>
#include "eth_process.h"

#define STA9100_BOOTLOADER_LEN      9744

extern int32_t upgrade_step;
void _eth_rx_ST9100_jump2Boot(binary_packet_format_t* pbiprx);
void _eth_rx_ST9100_jump2GNSS (binary_packet_format_t* pbiprx);

#endif 
