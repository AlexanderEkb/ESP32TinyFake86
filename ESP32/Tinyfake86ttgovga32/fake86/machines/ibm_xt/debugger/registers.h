#ifndef _DEBUGGER_REGISTERS_H_
#define _DEBUGGER_REGISTERS_H_

#include "../cpu/cpu.h"
#include "../service/widget.h"
#include "../service/inputbox.h"

class regBrowser_t : public widget_t
{
  public:
    regBrowser_t(widget_t * p);
    void init();
    virtual bool onKey(uint8_t scancode) override;
    virtual void repaint() override;
protected:
    typedef struct registerDesc_t
    {
      uint16_t value;
      bool hasChanged;
    } registerDesc_t;
    registerDesc_t registers[static_cast<uint32_t>(_dbgReg__COUNT)];
    static constexpr char *regNames[static_cast<uint32_t>(_dbgReg__COUNT)] = 
      {"IP", "AX", "BX", "CX", "DX", "SP", "BP", "SI", "DI", " F", "CS", "DS", "SS", "ES"};
    uint32_t selection;
    enum {
      STATE_DISPLAY,
      STATE_EDIT
    } state;
    InputBox_t * box;
    void beginEdit();
    void updateRegister();
    bool onKeyDisplay(uint8_t scancode);
    bool onKeyEdit(uint8_t scancode);
};

#endif /* _DEBUGGER_REGISTERS_H_ */
