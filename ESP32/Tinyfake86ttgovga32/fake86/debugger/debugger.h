#ifndef SIMPLE_DEBUGGER_H
#define SIMPLE_DEBUGGER_H

#include <stdint.h>

class debugger_t
{
  public:
    static debugger_t &getInstance() { return instance; };
    void execute();

private:
    static const uint32_t HEADER_HEIGHT = 2;
    static const uint32_t FOOTER_HEIGHT = 2;
    static const uint32_t REG_WINDOW_WIDTH = 10;
    static const uint32_t CODE_WINDOW_WIDTH = 20;
    debugger_t(){};
    static debugger_t instance;
    void onEnter();
    void refreshRegs();
    void refreshMem();
    void refreshCode();
    void refreshPort();
};

#endif /* SIMPLE_DEBUGGER_H */