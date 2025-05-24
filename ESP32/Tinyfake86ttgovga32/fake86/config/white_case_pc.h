#ifndef __CONFIG_WHITE_CASE_PC__
#define __CONFIG_WHITE_CASE_PC__

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

#if (KEYBOARD_DRIVER == 0)
#define KEYBOARD_DATA 35
#define KEYBOARD_CLK  34 
#elif (KEYBOARD_DRIVER == 1)
#define KEYBOARD_DATA 26
#define KEYBOARD_CLK  27
#else
#error Choose any correct keyboard driver!
#endif

/**
 * @brief Composide video output pin. Must be either 25 or 26, these pins are
 *        connected to internal DAC.
 */
#define VIDEO_PIN     25

#if RG_STORAGE_DRIVER == 1
#define RG_STORAGE_HOST             HSPI_HOST           // Used by SDSPI and SDMMC
#define SDSPI_MISO    GPIO_NUM_2
#define SDSPI_MOSI    GPIO_NUM_12
#define SDSPI_CS      GPIO_NUM_13
#define SDSPI_CLK     GPIO_NUM_14
#elif RG_STORAGE_DRIVER == 2
#define RG_STORAGE_HOST             SDMMC_HOST_SLOT_1   // Used by driver 1 and 2
#define SDIO_D0       02
#define SDIO_D1       04  // Not used
#define SDIO_D2       12  // Not used
#define SDIO_D3       13  // Not used
#define SDIO_CLK      14
#define SDIO_CMD      15
#endif

#define DISK_LED      13

#define RED_H         22         
#define RED_L         21
#define GREEN_H       19
#define GREEN_L       18
#define BLUE_H        05
#define BLUE_L        04
#define HSYNC         23
#define VSYNC         15

//=======================================================================[AUDIO]
#define AUDIO_OUTPUT_IO (33)

#endif /* __CONFIG_WHITE_CASE_PC__ */