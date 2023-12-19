#ifndef SIMPLE_DEBUGGER_H
#define SIMPLE_DEBUGGER_H

#include "service/service.h"
#include "registers.h"
#include "memory.h"


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

    static const uint32_t BROWSER_COUNT = 2;
    regBrowser_t regBrowser;
    memBrowser_t memBrowser;
    browser_t * const browsers[BROWSER_COUNT] = {&regBrowser, &memBrowser};

    DBG_MEM_ADDR memPosition;
    DBG_MEM_ADDR codePosition;

    debugger_t();
    static debugger_t instance;
    void onEnter();
};

#endif /* SIMPLE_DEBUGGER_H */