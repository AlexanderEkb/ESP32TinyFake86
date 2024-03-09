#ifndef _HARDWARE_H
#define _HARDWARE_H

#include "gbConfig.h"

#define RG_STORAGE_DRIVER 2

#define VIDEO_PIN     25
#define SPEAKER_PIN   27

#define KEYBOARD_DATA 32
#define KEYBOARD_CLK  33
#define KEYBOARD_RDY  12
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

/// [COVOX]
#define COVOX_OUTPUT_IO (4)
#endif
