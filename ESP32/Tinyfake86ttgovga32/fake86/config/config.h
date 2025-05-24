#ifndef _CONFIG_H
#define _CONFIG_H

#include <stddef.h>

/******************************************************************************/
/*************************** [HOST CONFIG] ************************************/
/******************************************************************************/

#define HOST_HW_VER 3

#if (HOST_HW_VER == 1)
#include "wooden_case_pc.h"
#elif (HOST_HW_VER == 2)
#include "white_case_pc.h"
#elif (HOST_HW_VER == 3)
#include "black_case_pc.h"
#else
#error "Specify any supported version of the hardware!"
#endif

/******************************************************************************/
/************************* [MACHINE CONFIG] ***********************************/
/******************************************************************************/

/**
 * @brief When CPU_V20 is defined, Fake86's CPU emulator acts like an 80186/V20.
 *        Otherwise, it acts like a true 8086/8088
 */
#define CPU_V20

/**
 * @brief Period of polling the keyboard, milliseconds
 * 
 */
#define KEYBOARD_POLL_ms (20)

/**
 * @brief Path to the HDD image to be mounted at startup.
 * 
 */
#define DEFAULT_HDD_IMAGE "/sd/PC/HDDs/hdd0.img"

/**
 * @brief RAM size in bytes. Must be lower than 736K.
 * 
 */
#define RAM_SIZE (640 * 1024)

/**
 * @brief If defined, host generates some real-time statistics about CPU performance.
 * 
 */
//#define STATS_ON

#endif

