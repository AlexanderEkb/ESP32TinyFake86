#ifndef SIMPLE_DEBUGGER_H
#define SIMPLE_DEBUGGER_H

#include "service/list.h"
#include "service/service.h"
#include "service/widget.h"
#include "registers.h"
#include "memory.h"
#include "code.h"

class debugger_t
{
  public:
    static debugger_t &getInstance() { return instance; };
    void execute();
    bool onKey(uint8_t scancode);
  private : 
    static const uint32_t HEADER_HEIGHT = 2;
    static const uint32_t FOOTER_HEIGHT = 2;
    static const uint32_t REG_WINDOW_WIDTH = 10;
    static const uint32_t CODE_WINDOW_WIDTH = 20;

    static const uint32_t BROWSER_COUNT = 3;
    widget_t screen;
    regBrowser_t regBrowser;
    memBrowser_t memBrowser;
    codeBrowser_t codeBrowser;

    DBG_MEM_ADDR memPosition;
    DBG_MEM_ADDR codePosition;
    bool isRunning;

    debugger_t() :
      screen(nullptr),
      regBrowser(regBrowser_t(&screen)),
      memBrowser(memBrowser_t(&screen)),
      codeBrowser(codeBrowser_t(&screen)),
      isRunning(false)
      {
        screen.setArea(rect_t(0, 0, 336, 240));
        screen.add(&memBrowser);
        screen.add(&regBrowser);
        screen.add(&codeBrowser);
        memPosition = DBG_MEM_ADDR(0, 0);
        codePosition = DBG_MEM_ADDR(0, 0);
      };
    static debugger_t instance;
    void nextBrowser();
    void doSingleStep();
    void onEnter();
};

#endif /* SIMPLE_DEBUGGER_H */