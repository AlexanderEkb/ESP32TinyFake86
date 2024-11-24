#ifndef OSDFILE_H
#define OSDFILE_H

#include <stdint.h>
#include "../machine.h"
#include "../../host/keyboard/keyboard.h"

typedef enum OSD_RESULT_t
{
  OSD_RESULT_NONE,
  OSD_RESULT_PREPARE,
  OSD_RESULT_RETURN
} OSD_RESULT_t;

class Osd_t
{
  public:
    Osd_t(Machine_t * machine) : machine(machine) {};
    OSD_RESULT_t execute();
  private:
    void osdLeave();
    void svcDrawTableLoRes(uint32_t p);
    void svcShowColorTable(void);
    uint8_t *svcGetPalette(uint32_t p);

    void showColorMenu();

    void OSDMenuRowsDisplayScroll(const char **ptrValue, unsigned char currentId, unsigned char aMax, uint32_t width, uint32_t pos, int32_t highlight = -1);
    uint8_t ShowTinyMenu(const char *cadTitle, const char **ptrValue, unsigned char aMax, uint32_t width, uint32_t pos, int32_t highlight = -1);
    void ShowTinyDSKMenu(uint32_t drive);
    void ShowTinyCPUDelayMenu();
    void ShowTinyTimerDelayMenu();
    void ShowTinyVGApollMenu();
    void ShowTinyKeyboardPollMenu();
    void ShowTinySpeedMenu();
    void ShowTinyVideoMenu();

    Machine_t * machine;
};

#endif
