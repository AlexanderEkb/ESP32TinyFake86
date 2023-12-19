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

class disassembler_t
{
  public:
    disassembler_t();
    void decode(uint8_t *buffer, uint32_t linesToDo);
  private:
    typedef enum segment_registers : int32_t {NO = -1, ES=0, CS, SS, DS} ; 
    uint32_t length;
    segment_registers segment_override = NO;
    segment_registers rm_segment_override = NO;
    uint32_t bytesToPrint = 0;
    uint8_t * code;
    uint32_t pointer;
    char line[256];
    uint32_t linePtr;

    void parse(char *instrTemplate, char*(disassembler_t::*func)(uint32_t *));
    char *rm(uint8_t type);
    uint32_t parse_noop(char *s);
    char *moffs16(uint32_t *err);
    char *rm8(uint32_t *err);
    char *rm16(uint32_t *err);
    char *call_inter(uint32_t *err);
    char *m16(uint32_t *err);
    char *sreg_rm16(uint32_t *err);
    char *rm16_sreg(uint32_t *err);
    char *rm16_imm8(uint32_t *err);
    char *rm16_imm16(uint32_t *err);
    char *rm8_imm8(uint32_t *err);
    char *rel16(uint32_t *err);
    char *rel8(uint32_t *err);
    char *imm8(uint32_t *err);
    char *imm16(uint32_t *err);
    char *r16_rm16(uint32_t *err);
    char *rm8_r8(uint32_t *err);
    char *r8_rm8(uint32_t *err);
    char *rm16_r16(uint32_t *err);

    uint32_t dump(...);
};

#endif /* _DEBUGGER_DASM_H_ */