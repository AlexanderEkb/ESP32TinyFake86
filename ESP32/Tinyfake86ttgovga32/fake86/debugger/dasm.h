#ifndef _DEBUGGER_DASM_H_
#define _DEBUGGER_DASM_H_

#include "service/service.h"
#include <string>

class line_t
{
  public:
    line_t();
    line_t(DBG_MEM_ADDR address);
    void print(const char *text);
    DBG_MEM_ADDR addr;
    uint32_t length;
    std::string s;
};

class disassembler_t
{
  public:
    disassembler_t() {
      preserveLine = true;
      length       = 0;
      opcode       = 0;
      line         = nullptr;
    };
    DBG_MEM_ADDR decode(DBG_MEM_ADDR origin, line_t * output);
  private:
    typedef enum segment_registers : int8_t {NO = -1, ES=0, CS, SS, DS} ; 
    uint32_t length;
    segment_registers segment_override = NO;
    segment_registers rm_segment_override = NO;
    uint8_t opcode;
    uint32_t instructionLength = 0;
    DBG_MEM_ADDR pointer;
    bool preserveLine;
    line_t * line;

    uint8_t fetchByte() {instructionLength++; uint8_t result = read86(pointer.linear()); pointer++; return result;};
    uint8_t lookNext();
    void parse(char *instrTemplate, char*(disassembler_t::*func)(uint32_t *));
    char *rm(uint8_t type);
    void parse_noop(char *s);
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

    void print_db(uint8_t byte);

    void printLine(char * fmt);
};

#endif /* _DEBUGGER_DASM_H_ */