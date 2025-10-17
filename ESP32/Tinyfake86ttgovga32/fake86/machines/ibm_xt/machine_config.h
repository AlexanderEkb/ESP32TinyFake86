#ifndef __IBM_XT_CONFIG_H__
#define __IBM_XT_CONFIG_H__

/******************************************************************************/
/************************* [MACHINE CONFIG] ***********************************/
/******************************************************************************/

/**
 * @brief When CPU_V20 is defined, Fake86's CPU emulator acts like an 80186/V20.
 *        Otherwise, it acts like a true 8086/8088
 */
#define CPU_V20

/**
 * @brief Emulated video card.
 *        0 is for CGA
 *        1 is for TGA
 */
#define IBM_XT_VIDEO_DRIVER (1)

#if (IBM_XT_VIDEO_DRIVER == 0)
#define VIDEO_MEMORY_SIZE (16384)
#elif (IBM_XT_VIDEO_DRIVER == 1)
#define VIDEO_MEMORY_SIZE (32768)
#endif

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

#endif /* __IBM_XT_CONFIG_H__ */
