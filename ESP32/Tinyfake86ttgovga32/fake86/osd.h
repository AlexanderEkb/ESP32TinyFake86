#ifndef OSDFILE_H
#define OSDFILE_H

typedef enum OSD_RESULT_t
{
  OSD_RESULT_NONE,
  OSD_RESULT_PREPARE,
  OSD_RESULT_RETURN
} OSD_RESULT_t;

OSD_RESULT_t do_tinyOSD(void);

#endif
