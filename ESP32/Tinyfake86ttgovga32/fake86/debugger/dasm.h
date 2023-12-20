#ifndef _DEBUGGER_DASM_H_
#define _DEBUGGER_DASM_H_

#include "debugger.h"

class line_t
{
  public:
    DBG_MEM_ADDR addr;
    uint8_t length;
    uint8_t prefixes;
    line_t();
    int32_t print(...) {return 0;};
};

class code_t
{
  public:
    code_t();
    void clear();
    line_t * newLine(const DBG_MEM_ADDR addr) {return nullptr;};
  private:
};

class disassembler_t
{
  public:
    disassembler_t();
    void decode(DBG_MEM_ADDR origin, int32_t length);
    code_t _code;
  private:
    typedef enum segment_registers : int32_t {NO = -1, ES=0, CS, SS, DS} ; 
    uint32_t length;
    segment_registers segment_override = NO;
    segment_registers rm_segment_override = NO;
    uint8_t opcode;
    uint32_t bytesToPrint = 0;
    DBG_MEM_ADDR pointer;
    line_t * line;

    uint8_t fetchByte() {bytesToPrint++; uint8_t result = read86(pointer.linear()); pointer++; return result;};
    uint8_t lookByte()  {return read86(pointer.linear());};
    uint8_t lookNext()  {return read86(pointer.linear() + 1);};
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
};

#endif /* _DEBUGGER_DASM_H_ */