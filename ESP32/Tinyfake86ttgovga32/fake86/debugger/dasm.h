#ifndef _DEBUGGER_DASM_H_
#define _DEBUGGER_DASM_H_

#include "debugger.h"

typedef struct MEM_ADDR
{
  uint16_t segment;
  uint16_t offset;
  MEM_ADDR(uint16_t seg, uint16_t off) :
    segment(seg),
    offset(off) {}
  uint32_t linear() {return ((uint32_t)segment << 4) + offset;}
} MEM_ADDR;

class line_t
{
  public:
    MEM_ADDR addr;
    line_t();
};

class code_t
{
  public:
    code_t();
    line_t * add(uint16_t seg, uint16_t off);
  private:
};

#endif /* _DEBUGGER_DASM_H_ */