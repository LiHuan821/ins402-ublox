/** ***************************************************************************
 * @file platform_version.h
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
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
#ifndef PLATFORM_VERSION_H
#define PLATFORM_VERSION_H

// DO NOT CHANGE THESE NUMBERS FROM ZERO!  CAUSES A CONFLICT WITH
//   IMUTest RESULTING IN ACCELEROMETER VALUES THAT ARE FLIPPED (WHAT
//   SHOULD BE POSITIVE BECOMES NEGATIVE).
enum {
   VERSION_MAJOR =0U,
   VERSION_MINOR =0U,
   VERSION_PATCH =0U,
   VERSION_STAGE =0U,
   VERSION_BUILD =0U,
};

// WARNING !!! CHANGE VERSIONS FOR BOTH vr AND PART STRING
enum {
   VER_MAJOR_NUM =0U,
   VER_MINOR_NUM =0U,
   VER_PATCH_NUM =0U,
   VER_STAGE_NUM =0U,
   VER_BUILD_NUM =2U,
};

static int32_t const SOFTWARE_PART_LEN = 50;
static int32_t const N_VERSION_STR   = 128;

#endif
