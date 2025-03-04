#ifndef _SERVICE_INPUTBOX_H
#define _SERVICE_INPUTBOX_H

#include "widget.h"

class InputBox_t : public widget_t
{
  public:
    InputBox_t(rect_t r);
    ~InputBox_t();
    virtual void repaint() override;
    virtual bool onKey(uint8_t scancode) override;
    virtual WidgetState_t state() override;
    char * string;
  private:
    static const uint32_t MAX_LENGTH = 128;
    uint32_t ptr;
    WidgetState_t modalState;
    void put(char c);
    void backspace();
};

#endif /* _SERVICE_INPUTBOX_H */