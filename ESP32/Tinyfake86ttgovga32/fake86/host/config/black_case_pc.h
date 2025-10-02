#ifndef __CONFIG_BLACK_CASE_PC__
#define __CONFIG_BLACK_CASE_PC__

/**
 * @brief SD-Card driver used in the system
 * 1 is for simple SPI driver
 * 2 is for SDIO driver
 * See below for differences in pinout for these two cases.
 */
#define RG_STORAGE_DRIVER 2

/**
 @brief Keyboard driver used in the system.
 0 is for simplified XT driver (custom keyboard, uses a kind of receive-only SPI bus).
 1 is for AT keyboard (fully-functional PS/2 one).
*/
#define KEYBOARD_DRIVER 1

/**
 * @brief Video driver used in the system/
 * 0 is for NTSC composite display on internal DAC
 */
#define VIDEO_DRIVER 0

/**
 * @brief Keyboard DATA pin
 */
#define KEYBOARD_DATA   26

/**
 * @brief Keyboard CLK pin
 */
#define KEYBOARD_CLK    27

/**
 * @brief PS/2 mouse DATA pin
 */
#define PS2_MOUSE_DATA   18

/**
 * @brief  PS/2 mouse CLK pin
 */
#define PS2_MOUSE_CLK    19

//=====================================================================[SD-Card]
#define RG_STORAGE_HOST SDMMC_HOST_SLOT_1
#define SDIO_D0         02
#define SDIO_CLK        14
#define SDIO_CMD        15

//=======================================================================[AUDIO]
/**
 * @brief Audio output pin
 * 
 */
#define AUDIO_OUTPUT_IO 33

//=======================================================================[VIDEO]
/**
 * @brief Composide video output pin. Must be either 25 or 26, these pins are
 *        connected to internal DAC.
 */
#define VIDEO_PIN       25

//========================================================================[MISC]

/**
 * @brief Disk activity LED pin.
 * 
 */
#define DISK_LED        13

#endif /* __CONFIG_BLACK_CASE_PC__ */