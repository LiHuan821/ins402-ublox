/** ***************************************************************************
 * @file platform_version.h
 * @brief product and hardware version definition based on UCB serial protocol.
 *****************************************************************************/
/*******************************************************************************
Copyright 2021 ACEINNA, INC

*******************************************************************************/
#ifndef PLATFORM_VERSION_H
#define PLATFORM_VERSION_H
#include <stdint.h>

enum {
   VERSION_MAJOR =0U,
   VERSION_MINOR =0U,
   VERSION_PATCH =0U,
   VERSION_STAGE =0U,
   VERSION_BUILD =0U,
};




// WARNING !!! CHANGE VERSIONS FOR BOTH vr AND PART STRING
enum {
   VER_MAJOR_NUM =6U,
   VER_MINOR_NUM =0U,
   VER_PATCH_NUM =0U,
   VER_STAGE_NUM =0U,
   VER_BUILD_NUM =2U,
};

static uint8_t const PN_STRING_232[] = "5020-4007-01";
static uint8_t const PART_NUMBER_STRING[] = "5020-4007-01 HW v1.0";
static int32_t const SOFTWARE_PART_LEN = 50;
static int32_t const N_VERSION_STR   = 128;

#endif
