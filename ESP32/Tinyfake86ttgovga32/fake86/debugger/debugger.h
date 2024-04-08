#ifndef SIMPLE_DEBUGGER_H
#define SIMPLE_DEBUGGER_H

#include "service/service.h"
#include "service/widget.h"
#include "registers.h"
#include "memory.h"
#include "code.h"

class debugger_t : public widget_t
{
public:
    static debugger_t &getInstance() { return instance; };
    void execute();
    virtual bool onKey(uint8_t scancode) override;
  private : 
    static const uint32_t HEADER_HEIGHT = 2;
    static const uint32_t FOOTER_HEIGHT = 2;
    static const uint32_t REG_WINDOW_WIDTH = 10;
    static const uint32_t CODE_WINDOW_WIDTH = 20;

    static const uint32_t BROWSER_COUNT = 3;
    regBrowser_t regBrowser;
    memBrowser_t memBrowser;
    codeBrowser_t codeBrowser;

    browser_t * browser;

    DBG_MEM_ADDR memPosition;
    DBG_MEM_ADDR codePosition;
    bool isRunning;

    debugger_t();
    static debugger_t instance;
    void nextBrowser();
    void doSingleStep();
    void onEnter();
};

#endif /* SIMPLE_DEBUGGER_H */