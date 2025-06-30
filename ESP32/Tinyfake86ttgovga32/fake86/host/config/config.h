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

#endif

