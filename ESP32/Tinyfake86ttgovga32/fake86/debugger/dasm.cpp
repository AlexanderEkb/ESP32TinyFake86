#include "cpu/cpu.h"
#include "dasm.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static char *regs16[8] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"} ; 
static char *regs8[8] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"} ;
static char *segreg[4] = {"es", "cs", "ss", "ds"} ;

line_t::line_t()
{
  addr.segment = 0;
  addr.offset = 0;
  length = 0;
  s.clear();
}

line_t::line_t(DBG_MEM_ADDR address)
{
  addr = address;
  length = 0;
  s.clear();
}

void line_t::print(const char * text)
{
  s.append(text);
}

int32_t operator-(DBG_MEM_ADDR& lhs, DBG_MEM_ADDR& rhs)
{
  return (lhs.linear() - rhs.linear());
}
 
void disassembler_t::parse(char *instrTemplate, char*(disassembler_t::*func)(uint32_t *))
{
	// instructionLength = 1;
	uint32_t error = 0 ;
	char *result = (this->*func)(&error) ;
	if (error)
	{
		char tmp_buffer[20] ; 
		snprintf(tmp_buffer, 20, "db %02Xh", opcode) ;
		parse_noop(tmp_buffer) ;
		return;
	}
	uint32_t t = 0 ;
	if (segment_override == NO && rm_segment_override >= 0)
	{
		t = 1; 
		rm_segment_override = NO ; 
		segment_override = NO ;
	}
	char segment[20] ;

	if (t == 1)
	{
			// switch (segment_override)
			// {
			// 	case ES: snprintf(segment, 20, "es") ; break ;
			// 	case CS: snprintf(segment, 20, "cs") ; break ;
			// 	case SS: snprintf(segment, 20, "ss") ; break ;
			// 	case DS: snprintf(segment, 20, "ds") ; break ;
      //   default: 
      //     segment[0] = "\0" ; 
      //     break ;
			// }
      // char tmp_string[41] = {0};
      // char tmp_string2[41] = {0};
      // snprintf(tmp_string, 40, instrTemplate, result);
      // snprintf(tmp_string2, 40, "%s %s", segment, tmp_string);
      // line->print(tmp_string2);
      line->print("?");
  }
  // else
  // {
    char _tmp[80];
    snprintf(_tmp, 80, instrTemplate, result) ;
    line->print(_tmp);
  // }
}

void disassembler_t::parse_noop(char *instrTemplate)
{
	if (segment_override >= 0)
	{
		switch (segment_override)
		{
			case ES: line->print("es:") ; break ;         // OUT: (3)Segment override
			case CS: line->print("cs:") ; break ;
			case SS: line->print("ss:") ; break ;
			case DS: line->print("ds:") ; break ;
      default: line->print("??:") ; break ;
		}
		segment_override = NO ;
		rm_segment_override = NO ; 
	}
  char _tmp[80];
  snprintf(_tmp, 80, "%s", instrTemplate) ;               // OUT: (4)Instruction
  line->print(_tmp);
}

DBG_MEM_ADDR disassembler_t::decode(DBG_MEM_ADDR origin, line_t *output)
{
  pointer = origin;
  line = output;
  segment_override = NO;
  rm_segment_override = NO;
  output->addr = origin;
  preserveLine = true;

  instructionLength = 0;
  // line->print("%04X:%04X ", pointer.segment, pointer.offset); // OUT: 1 - address

  while (preserveLine)
	{
    preserveLine = false;
    opcode = fetchByte();
		switch (opcode)
		{
			case 0x00: parse("add %s", &disassembler_t::rm8_r8) ; break ;
			case 0x01: parse("add %s", &disassembler_t::rm16_r16) ; break ;
			case 0x02: parse("add %s", &disassembler_t::r8_rm8) ; break ;
			case 0x03: parse("add %s", &disassembler_t::r16_rm16) ; break ;
			case 0x04: parse("add al,%s", &disassembler_t::imm8) ; break ; 
			case 0x05: parse("add ax,%s", &disassembler_t::imm16) ; break ;
			case 0x06: parse_noop("push es"); break ; 
			case 0x07: parse_noop("pop es"); break ;
			case 0x08: parse("or %s", &disassembler_t::rm8_r8) ; break ;
			case 0x09: parse("or %s", &disassembler_t::rm16_r16) ; break ;
			case 0x0A: parse("or %s", &disassembler_t::r8_rm8) ; break ;
			case 0x0B: parse("or %s", &disassembler_t::r16_rm16) ; break ;
			case 0x0C: parse("or al,%s", &disassembler_t::imm8) ; break ; 
			case 0x0D: parse("or ax,%s", &disassembler_t::imm16) ; break ;
			case 0x0E: parse_noop("push cs") ; break ;
			case 0x10: parse("adc %s", &disassembler_t::rm8_r8) ; break ;
			case 0x11: parse("adc %s", &disassembler_t::rm16_r16) ; break ;
			case 0x12: parse("adc %s", &disassembler_t::r8_rm8) ; break ;
			case 0x13: parse("adc %s", &disassembler_t::r16_rm16) ; break ;
			case 0x14: parse("adc al,%s", &disassembler_t::imm8) ; break ; 
			case 0x15: parse("adc ax,%s", &disassembler_t::imm16) ; break ;
			case 0x16: parse_noop("push ss") ; break ;
			case 0x17: parse_noop("pop ss") ; break ;
			case 0x18: parse("sbb %s", &disassembler_t::rm8_r8) ; break ;
			case 0x19: parse("sbb %s", &disassembler_t::rm16_r16) ; break ;
			case 0x1A: parse("sbb %s", &disassembler_t::r8_rm8) ; break ;
			case 0x1B: parse("sbb %s", &disassembler_t::r16_rm16) ; break ;
			case 0x1C: parse("sbb al,%s", &disassembler_t::imm8) ; break ; 
			case 0x1D: parse("sbb ax,%s", &disassembler_t::imm16) ; break ;
			case 0x1E: parse_noop("push ds") ; break ;
			case 0x1F: parse_noop("pop ds") ; break ;
			case 0x20: parse("and %s", &disassembler_t::rm8_r8) ; break ;
			case 0x21: parse("and %s", &disassembler_t::rm16_r16) ; break ;
			case 0x22: parse("and %s", &disassembler_t::r8_rm8) ; break ;
			case 0x23: parse("and %s", &disassembler_t::r16_rm16) ; break ;
			case 0x24: parse("and al,%s", &disassembler_t::imm8) ; break ; 
			case 0x25: parse("and ax,%s", &disassembler_t::imm16) ; break ;
			case 0x26: 
			{
				segment_override = ES ; 
				rm_segment_override = ES;
        preserveLine = true;
			}
			break ;
			case 0x27: parse_noop("daa") ; break ;
			case 0x28: parse("sub %s", &disassembler_t::rm8_r8) ; break ;
			case 0x29: parse("sub %s", &disassembler_t::rm16_r16) ; break ;
			case 0x2A: parse("sub %s", &disassembler_t::r8_rm8) ; break ;
			case 0x2B: parse("sub %s", &disassembler_t::r16_rm16) ; break ;
			case 0x2C: parse("sub al,%s", &disassembler_t::imm8) ; break ; 
			case 0x2D: parse("sub ax,%s", &disassembler_t::imm16) ; break ;
			case 0x2E: 
			{
				segment_override = CS ; 
				rm_segment_override = CS ;
        preserveLine = true;
      } break ;
			case 0x2F: parse_noop("das") ; break ;
			case 0x30: parse("xor %s", &disassembler_t::rm8_r8) ; break ;
			case 0x31: parse("xor %s", &disassembler_t::rm16_r16) ; break ;
			case 0x32: parse("xor %s", &disassembler_t::r8_rm8) ; break ;
			case 0x33: parse("xor %s", &disassembler_t::r16_rm16) ; break ;
			case 0x34: parse("xor al,%s", &disassembler_t::imm8) ; break ; 
			case 0x35: parse("xor ax,%s", &disassembler_t::imm16) ; break ;
			case 0x36: 
			{
				segment_override = SS ; 
				rm_segment_override = SS ;
        preserveLine = true;
      } break ;
			case 0x37: parse_noop("aaa") ; break ;
			case 0x38: parse("cmp %s", &disassembler_t::rm8_r8) ; break ;
			case 0x39: parse("cmp %s", &disassembler_t::rm16_r16) ; break ;
			case 0x3A: parse("cmp %s", &disassembler_t::r8_rm8) ; break ;
			case 0x3B: parse("cmp %s", &disassembler_t::r16_rm16) ; break ;
			case 0x3C: parse("cmp al,%s", &disassembler_t::imm8) ; break ; 
			case 0x3D: parse("cmp ax,%s", &disassembler_t::imm16) ; break ;
			case 0x3E: 
			{
				segment_override = DS ; 
				rm_segment_override = DS ;
        preserveLine = true;
      }
			break ;
			case 0x3F: parse_noop("aas") ; break ;
			case 0x40: parse_noop("inc ax") ; break ;
			case 0x41: parse_noop("inc cx") ; break ;
			case 0x42: parse_noop("inc dx") ; break ;
			case 0x43: parse_noop("inc bx") ; break ;
			case 0x44: parse_noop("inc sp") ; break ;
			case 0x45: parse_noop("inc bp") ; break ;
			case 0x46: parse_noop("inc si") ; break ;
			case 0x47: parse_noop("inc di") ; break ;
			case 0x48: parse_noop("dec ax") ; break ;
			case 0x49: parse_noop("dec cx") ; break ;
			case 0x4A: parse_noop("dec dx") ; break ;
			case 0x4B: parse_noop("dec bx") ; break ;
			case 0x4C: parse_noop("dec sp") ; break ;
			case 0x4D: parse_noop("dec bp") ; break ;
			case 0x4E: parse_noop("dec si") ; break ;
			case 0x4F: parse_noop("dec di") ; break ;
			case 0x50: parse_noop("push ax") ; break ;
			case 0x51: parse_noop("push cx") ; break ;
			case 0x52: parse_noop("push dx") ; break ;
			case 0x53: parse_noop("push bx") ; break ;
			case 0x54: parse_noop("push sp") ; break ;
			case 0x55: parse_noop("push bp") ; break ;
			case 0x56: parse_noop("push si") ; break ;
			case 0x57: parse_noop("push di") ; break ;
			case 0x58: parse_noop("pop ax") ; break ;
			case 0x59: parse_noop("pop cx") ; break ;
			case 0x5A: parse_noop("pop dx") ; break ;
			case 0x5B: parse_noop("pop bx") ; break ;
			case 0x5C: parse_noop("pop sp") ; break ;
			case 0x5D: parse_noop("pop bp") ; break ;
			case 0x5E: parse_noop("pop si") ; break ;
			case 0x5F: parse_noop("pop di") ; break ;
			case 0x70: parse("jo %s", &disassembler_t::rel8) ; break ;
			case 0x71: parse("jno %s", &disassembler_t::rel8) ; break ;
			case 0x72: parse("jc %s", &disassembler_t::rel8) ; break ;
			case 0x73: parse("jnc %s", &disassembler_t::rel8) ; break ;
			case 0x74: parse("jz %s", &disassembler_t::rel8) ; break ;
			case 0x75: parse("jnz %s", &disassembler_t::rel8) ; break ;
			case 0x76: parse("jna %s", &disassembler_t::rel8) ; break ;
			case 0x77: parse("ja %s", &disassembler_t::rel8) ; break ;
			case 0x78: parse("js %s", &disassembler_t::rel8) ; break ;
			case 0x79: parse("jns %s", &disassembler_t::rel8) ; break ;
			case 0x7A: parse("jpe %s", &disassembler_t::rel8) ; break ;
			case 0x7B: parse("jpo %s", &disassembler_t::rel8) ; break ;
			case 0x7C: parse("jl %s", &disassembler_t::rel8) ; break ;
			case 0x7D: parse("jnl %s", &disassembler_t::rel8) ; break ;
			case 0x7E: parse("jng %s", &disassembler_t::rel8) ; break ;
			case 0x7F: parse("jg %s", &disassembler_t::rel8) ; break ;
			case 0x80:
			case 0x82:
			{
				uint8_t opcode = ((lookNext() & 0x38) >> 3 ); 
				uint8_t t = 0 ;
				switch (opcode)
				{
					case 0x00: parse("add %s", &disassembler_t::rm8_imm8) ; break ;
					case 0x01: parse("or %s", &disassembler_t::rm8_imm8) ; break ;
					case 0x02: parse("adc %s", &disassembler_t::rm8_imm8) ; break ;
					case 0x03: parse("sbb %s", &disassembler_t::rm8_imm8) ; break ;
					case 0x04: parse("and %s", &disassembler_t::rm8_imm8) ; break ;
					case 0x05: parse("sub %s", &disassembler_t::rm8_imm8) ; break ;
					case 0x06: parse("xor %s", &disassembler_t::rm8_imm8) ; break ;
					case 0x07: parse("cmp %s", &disassembler_t::rm8_imm8) ; break ;
					default: t = 1; break ;
				}
				if (t) print_db(opcode) ;
			} break ;
			case 0x81:
			{
				uint8_t opcode = ((lookNext() & 0x38) >> 3 ); 
				uint8_t t = 0 ;
				switch (opcode)
				{
					case 0x00: parse("add %s", &disassembler_t::rm16_imm16) ; break ;
					case 0x01: parse("or %s", &disassembler_t::rm16_imm16) ; break ;
					case 0x02: parse("adc %s", &disassembler_t::rm16_imm16) ; break ;
					case 0x03: parse("sbb %s", &disassembler_t::rm16_imm16) ; break ;
					case 0x04: parse("and %s", &disassembler_t::rm16_imm16) ; break ;
					case 0x05: parse("sub %s", &disassembler_t::rm16_imm16) ; break ;
					case 0x06: parse("xor %s", &disassembler_t::rm16_imm16) ; break ;
					case 0x07: parse("cmp %s", &disassembler_t::rm16_imm16) ; break ;
					default: t = 1; break ;
				}
				if (t) print_db(opcode) ;
			} break ;
			case 0x83:
			{
				uint8_t opcode = ((lookNext() & 0x38) >> 3 ); 
				uint8_t t = 0 ;
				switch (opcode)
				{
					case 0x00: parse("add %s", &disassembler_t::rm16_imm8) ; break ;
					case 0x02: parse("adc %s", &disassembler_t::rm16_imm8) ; break ;
					case 0x03: parse("sbb %s", &disassembler_t::rm16_imm8) ; break ;
					case 0x05: parse("sub %s", &disassembler_t::rm16_imm8) ; break ;
					case 0x07: parse("cmp %s", &disassembler_t::rm16_imm8) ; break ;
					default: t = 1; break ;
				} 
				if (t) print_db(opcode) ;
			} break ;
			case 0x84: parse("test %s", &disassembler_t::rm8_r8) ; break ;
			case 0x85: parse("test %s", &disassembler_t::rm16_r16) ; break ;
			case 0x86: parse("xchg %s", &disassembler_t::rm8_r8) ; break ;
			case 0x87: parse("xchg %s", &disassembler_t::rm16_r16) ; break ;
			case 0x88: parse("mov %s", &disassembler_t::rm8_r8) ; break ;
			case 0x89: parse("mov %s", &disassembler_t::rm16_r16) ; break ;
			case 0x8A: parse("mov %s", &disassembler_t::r8_rm8) ; break ;
			case 0x8B: parse("mov %s", &disassembler_t::r16_rm16) ; break ;
			case 0x8C: 
			{ 
				parse("mov %s", &disassembler_t::rm16_sreg) ; break ;
			}
			case 0x8D: parse("lea %s", &disassembler_t::r16_rm16) ; break ;
			case 0x8E: 
			{
					parse("mov %s", &disassembler_t::sreg_rm16) ; break ;
			}
			case 0x8F:
			{
			  	uint8_t opcode = ((lookNext() & 0x38) >> 3 ); 
				uint8_t t = 0 ;
				switch (opcode)
				{
					case 0x00: parse("pop word %s", &disassembler_t::m16) ; break ;
					default: t = 1; break ;
				}	
				if (t) print_db(opcode) ;
			} break ;
			case 0x90: parse_noop("xchg ax,ax") ; break ;
			case 0x91: parse_noop("xchg cx,ax") ; break ;
			case 0x92: parse_noop("xchg dx,ax") ; break ;
			case 0x93: parse_noop("xchg bx,ax") ; break ;
			case 0x94: parse_noop("xchg sp,ax") ; break ;
			case 0x95: parse_noop("xchg bp,ax") ; break ;
			case 0x96: parse_noop("xchg si,ax") ; break ;
			case 0x97: parse_noop("xchg di,ax") ; break ;
			case 0x98: parse_noop("cbw") ; break ;
			case 0x99: parse_noop("cwd") ; break ;
			case 0x9A: parse("call %s", &disassembler_t::call_inter) ; break ; 
			case 0x9B: parse_noop("wait") ; break ;
			case 0x9C: parse_noop("pushf") ; break ;
			case 0x9D: parse_noop("popf") ; break ;
			case 0x9E: parse_noop("sahf") ; break ;
			case 0x9F: parse_noop("lahf") ; break ;
			case 0xA0: parse("mov al,%s", &disassembler_t::moffs16) ; break ;
			case 0xA1: parse("mov ax,%s", &disassembler_t::moffs16) ; break ;
			case 0xA2: parse("mov %s,al", &disassembler_t::moffs16) ; break ;
			case 0xA3: parse("mov %s,ax", &disassembler_t::moffs16) ; break ;
			case 0xA4: parse_noop("movsb") ; break ;
			case 0xA5: parse_noop("movsw") ; break ;
			case 0xA6: parse_noop("cmpsb") ; break ;
			case 0xA7: parse_noop("cmpsw") ; break ;
			case 0xA8: parse("test al, %s", &disassembler_t::imm8) ; break ;
			case 0xA9: parse("test ax, %s", &disassembler_t::imm16) ; break ;
			case 0xAA: parse_noop("stosb") ; break ;
			case 0xAB: parse_noop("stosw") ; break ;
			case 0xAC: parse_noop("lodsb") ; break ;
			case 0xAD: parse_noop("lodsw") ; break ;
			case 0xAE: parse_noop("scasb") ; break ;
			case 0xAF: parse_noop("scasw") ; break ;
			case 0xB0: parse("mov al,%s",&disassembler_t::imm8); break;
			case 0xB1: parse("mov cl,%s",&disassembler_t::imm8); break;
			case 0xB2: parse("mov dl,%s",&disassembler_t::imm8); break;
			case 0xB3: parse("mov bl,%s",&disassembler_t::imm8); break;
			case 0xB4: parse("mov ah,%s",&disassembler_t::imm8); break;
			case 0xB5: parse("mov ch,%s",&disassembler_t::imm8); break;
			case 0xB6: parse("mov dh,%s",&disassembler_t::imm8); break;
			case 0xB7: parse("mov bh,%s",&disassembler_t::imm8); break;
			case 0xB8: parse("mov ax,%s",&disassembler_t::imm16); break;
			case 0xB9: parse("mov cx,%s",&disassembler_t::imm16); break;
			case 0xBA: parse("mov dx,%s",&disassembler_t::imm16); break;
			case 0xBB: parse("mov bx,%s",&disassembler_t::imm16); break;
			case 0xBC: parse("mov sp,%s",&disassembler_t::imm16); break;
			case 0xBD: parse("mov bp,%s",&disassembler_t::imm16); break;
			case 0xBE: parse("mov si,%s",&disassembler_t::imm16); break;
			case 0xBF: parse("mov di,%s",&disassembler_t::imm16); break;
			case 0xC2: parse("ret %s", &disassembler_t::imm16) ; break ; 
			case 0xC3: parse_noop("ret") ; break ;
			case 0xC4: parse("les %s", &disassembler_t::r16_rm16) ; break ;
			case 0xC5: parse("lds %s", &disassembler_t::r16_rm16) ; break ;
			case 0xC6: parse("mov %s", &disassembler_t::rm16_imm8) ; break ;
			case 0xC7: parse("mov %s", &disassembler_t::rm16_imm16) ; break ;
			case 0xCA: parse("retf %s", &disassembler_t::imm16) ; break ;
			case 0xCB: parse_noop("retf") ; break ;
			case 0xCC: parse_noop("int3") ; break ;
			case 0xCD: parse("int %s", &disassembler_t::imm8) ; break ;
			case 0xCE: parse_noop("into") ; break ;
			case 0xCF: parse_noop("iret") ; break ;
			case 0xD0:
			{
				uint8_t opcode = ((lookNext() & 0x38) >> 3 ); 
				uint8_t t = 0 ;
				switch (opcode)
				{
					case 0x00: parse("rol %s,1", &disassembler_t::rm8) ; break ;
					case 0x01: parse("ror %s,1", &disassembler_t::rm8) ; break ;
					case 0x02: parse("rcl %s,1", &disassembler_t::rm8) ; break ;
					case 0x03: parse("rcr %s,1", &disassembler_t::rm8) ; break ;
					case 0x04: parse("shl %s,1", &disassembler_t::rm8) ; break ;
					case 0x05: parse("shr %s,1", &disassembler_t::rm8) ; break ;
					case 0x07: parse("sar %s,1", &disassembler_t::rm8) ; break ;
					default: t = 1; break ;
				}
				if (t) print_db(opcode) ;
			} break ;
			case 0xD1:
			{
				uint8_t opcode = ((lookNext() & 0x38) >> 3 ); 
				uint8_t t = 0 ;
				switch (opcode)
				{
					case 0x00: parse("rol %s,1", &disassembler_t::rm16) ; break ;
					case 0x01: parse("ror %s,1", &disassembler_t::rm16) ; break ;
					case 0x02: parse("rcl %s,1", &disassembler_t::rm16) ; break ;
					case 0x03: parse("rcr %s,1", &disassembler_t::rm16) ; break ;
					case 0x04: parse("shl %s,1", &disassembler_t::rm16) ; break ;
					case 0x05: parse("shr %s,1", &disassembler_t::rm16) ; break ;
					case 0x07: parse("sar %s,1", &disassembler_t::rm16) ; break ;
					default: t = 1; break ;
				}
				if (t) print_db(opcode) ;
			} break ;
			case 0xD2:
			{
				uint8_t opcode = ((lookNext() & 0x38) >> 3 ); 
				uint8_t t = 0 ;
				switch (opcode)
				{
					case 0x00: parse("rol %s,cl", &disassembler_t::rm8) ; break ;
					case 0x01: parse("ror %s,cl", &disassembler_t::rm8) ; break ;
					case 0x02: parse("rcl %s,cl", &disassembler_t::rm8) ; break ;
					case 0x03: parse("rcr %s,cl", &disassembler_t::rm8) ; break ;
					case 0x04: parse("shl %s,cl", &disassembler_t::rm8) ; break ;
					case 0x05: parse("shr %s,cl", &disassembler_t::rm8) ; break ;
					case 0x07: parse("sar %s,cl", &disassembler_t::rm8) ; break ;
					default: t = 1; break ;
				}
				if (t) print_db(opcode) ;
			} break ;
			case 0xD3:
			{
				uint8_t opcode = ((lookNext() & 0x38) >> 3 ); 
				uint8_t t = 0 ;
				switch (opcode)
				{
					case 0x00: parse("rol %s,cl", &disassembler_t::rm16) ; break ;
					case 0x01: parse("ror %s,cl", &disassembler_t::rm16) ; break ;
					case 0x02: parse("rcl %s,cl", &disassembler_t::rm16) ; break ;
					case 0x03: parse("rcr %s,cl", &disassembler_t::rm16) ; break ;
					case 0x04: parse("shl %s,cl", &disassembler_t::rm16) ; break ;
					case 0x05: parse("shr %s,cl", &disassembler_t::rm16) ; break ;
					case 0x07: parse("sar %s,cl", &disassembler_t::rm16) ; break ;
					default: t = 1; break ;
				}
				if (t) print_db(opcode) ;
			} break ;
			case 0xD4: parse_noop("aam") ; break ;
			case 0xD5: parse_noop("aad") ; break ;
			case 0xD7: parse_noop("xlatb") ; break ;
			/*D8-DF => ESC0-7*/
			case 0xE0: parse("loopne %s", &disassembler_t::rel8) ; break ;
			case 0xE1: parse("loope %s", &disassembler_t::rel8) ; break ;
			case 0xE2: parse("loop %s", &disassembler_t::rel8) ; break ;
			case 0xE3: parse("jcxz %s", &disassembler_t::rel8) ; break ;
			case 0xE4: parse("in al,%s", &disassembler_t::imm8) ; break ;
			case 0xE5: parse("in ax,%s", &disassembler_t::imm8) ; break ;
			case 0xE6: parse("out %s,al", &disassembler_t::imm8) ; break ;
			case 0xE7: parse("out %s,ax", &disassembler_t::imm8) ; break ;
			case 0xE8: parse("call %s", &disassembler_t::rel16) ; break ;
			case 0xE9: parse("jmp %s", &disassembler_t::rel16) ; break ;
			case 0xEA: parse("jmp %s", &disassembler_t::call_inter) ; break ;
			case 0xEB: parse("jmp short %s", &disassembler_t::rel8) ; break ;
			case 0xEC: parse_noop("in al,dx") ; break ;
			case 0xED: parse_noop("in ax,dx") ; break ;
			case 0xEE: parse_noop("out dx,al") ; break ;
			case 0xEF: parse_noop("out dx,ax") ; break ;
			case 0xF0: line->print("lock ") ; preserveLine = true; break ;
			case 0xF2: line->print("repne ") ; preserveLine = true; break ;
			case 0xF3: line->print("rep ") ; preserveLine = true; break ;
			case 0xF4: parse_noop("hlt") ; break ;
			case 0xF5: parse_noop("cmc") ; break ;
			case 0xF6:
			{
				uint8_t opcode = ((lookNext() & 0x38) >> 3 ); 
				uint8_t t = 0 ;
				switch (opcode)
				{
					case 0x00: parse("test %s", &disassembler_t::rm8_imm8) ; break ;
					case 0x02: parse("not %s", &disassembler_t::rm8) ; break ;
					case 0x03: parse("neg %s", &disassembler_t::rm8) ; break ;
					case 0x04: parse("mul %s", &disassembler_t::rm8) ; break ;
					case 0x05: parse("imul %s", &disassembler_t::rm8) ; break ;
					case 0x06: parse("div %s", &disassembler_t::rm8) ; break ;
					case 0x07: parse("idiv %s", &disassembler_t::rm8) ; break ;
					default: t = 1; break ;
				}
				if (t) print_db(opcode) ;
			} break ;
			case 0xF7:
			{
				uint8_t opcode = ((lookNext() & 0x38) >> 3 ); 
				uint8_t t = 0 ;
				switch (opcode)
				{
					case 0x00: parse("test %s", &disassembler_t::rm16_imm16) ; break ;
					case 0x02: parse("not %s", &disassembler_t::rm16) ; break ;
					case 0x03: parse("neg %s", &disassembler_t::rm16) ; break ;
					case 0x04: parse("mul %s", &disassembler_t::rm16) ; break ;
					case 0x05: parse("imul %s", &disassembler_t::rm16) ; break ;
					case 0x06: parse("div %s", &disassembler_t::rm16) ; break ;
					case 0x07: parse("idiv %s", &disassembler_t::rm16) ; break ;
					default: t = 1; break ;
				}
				if (t) print_db(opcode) ;
			} break ;
			case 0xF8: parse_noop("clc") ; break ;
			case 0xF9: parse_noop("stc") ; break ;
			case 0xFA: parse_noop("cli") ; break ;
			case 0xFB: parse_noop("sti") ; break ;
			case 0xFC: parse_noop("cld") ; break ;
			case 0xFD: parse_noop("std") ; break ;
			case 0xFE:
			{
				uint8_t opcode = ((lookNext() & 0x38) >> 3 ); 
				uint8_t t = 0 ;	
				switch (opcode)
				{
					case 0x00: parse("inc %s", &disassembler_t::rm8) ; break ;
					case 0x01: parse("dec %s", &disassembler_t::rm8) ; break ;
					default: t = 1; break ;
				}
				if (t) print_db(opcode) ;
			} break ;
			case 0xFF:
			{
				uint8_t opcode = ((lookNext() & 0x38) >> 3 ); 
				uint8_t t = 0 ;
				switch (opcode)
				{
					case 0x00: parse("inc %s", &disassembler_t::rm16) ; break ;
					case 0x01: parse("dec %s", &disassembler_t::rm16) ; break ;
					case 0x02: parse("call near %s", &disassembler_t::rm16) ; break ;
					case 0x03: parse("call far %s", &disassembler_t::rm16) ; break ;
					case 0x04: parse("jmp near %s", &disassembler_t::rm16) ; break ;
					case 0x05: parse("jmp far %s", &disassembler_t::rm16) ; break ;
					case 0x06: parse("push %s", &disassembler_t::rm16) ; break ;
					default: t = 1; break ;
				}
				if (t) print_db(opcode);
			} break ;
			default: 
        print_db(opcode);
		}
	}

  output->length = instructionLength;
  return pointer;
}

void disassembler_t::print_db(uint8_t byte)
{
  char tmp_buffer[20] = {0};
  snprintf(tmp_buffer, 20, "db %02Xh", byte);
  parse_noop(tmp_buffer);
}

char str[255] ; 

char * disassembler_t::moffs16(uint32_t *err)
{
	char segment[10] = "";
	if (segment_override >= 0)
	{
		switch (segment_override)
		{
			case ES: snprintf(segment, 10, "es:") ; break ;
			case CS: snprintf(segment, 10, "cs:") ; break ;
			case SS: snprintf(segment, 10, "ss:") ; break ;
			case DS: snprintf(segment, 10, "ds:") ; break ;
      default: snprintf(segment, 10, "%02X:", static_cast<int32_t>(segment_override)); break ;
		}
		segment_override = NO ;
	}
	uint8_t low = fetchByte(); 
	uint8_t high = fetchByte(); 
	uint16_t imm16 = ((high << 8) + low) ;
	snprintf(str, 10, "[%s%02Xh]", segment, imm16) ;
	return str ;
}

char * disassembler_t::rm8(uint32_t *err)
{
	char *s =  rm(8)  ;
	snprintf(str, 20, "%s", s) ; 
	return str ;
}

char * disassembler_t::rm16(uint32_t *err)
{
	char *s =  rm(16)  ;
	snprintf(str, 20, "%s", s) ; 
	return str ;
}

char * disassembler_t::call_inter(uint32_t *err)
{
	uint8_t offset_low = fetchByte(); 
	uint8_t offset_high = fetchByte(); 
	uint8_t seg_low = fetchByte(); 
	uint8_t seg_high = fetchByte(); 
	uint16_t offset = ((offset_high << 8) + offset_low) ; 
	uint16_t seg = ((seg_high << 8) + seg_low) ;
	snprintf(str, 20, "%04X:%04X", seg, offset) ;
	return str ;	
}

char * disassembler_t::m16(uint32_t *err)
{
	char *s =  rm(16)  ;
	snprintf(str, 20, "%s", s) ;
	return str ;
}

/*
8E  - MOV 
06  - 00.000.110
A2
04
*/
char * disassembler_t::sreg_rm16(uint32_t *error) 
{
	uint8_t reg = ((lookNext() & 0x38) >> 3) ;
	char *s =  rm(16)  ;
	if (reg < 4)
	{
		char *sreg = segreg[reg] ;
		snprintf(str, 20, "%s,%s", sreg, s) ;
		*error = 0 ;
	} else *error = 1 ; 
  // {
  //   char *sreg = segreg[reg & 0x03];
  //   snprintf(str, 20, "%s?,%s", sreg, s);
  //   *error = 0;
  // }
  return str ;
}

char * disassembler_t::rm16_sreg(uint32_t *error)
{
	uint8_t reg = ((lookNext() & 0x38) >> 3) ;
	char *s =  rm(16)  ;
	if (reg < 4)
	{
		char *sreg = segreg[reg] ;
		snprintf(str, 20, "%s,%s", s, sreg) ;
		*error = 0 ;
	} else *error = 1 ; 
	return str ;
}

char * disassembler_t::rm16_imm8(uint32_t *err)
{
	char *s = rm(16) ;
	int8_t imm8 = fetchByte(); 
	char sign = '+' ;
	if (imm8 < 0) 
	{
		sign = '-' ;
		imm8 = -imm8 ;
	}
	snprintf(str, 20, "%s,byte %c%02Xh", s, sign, imm8) ;
	return str ;
}

char * disassembler_t::rm16_imm16(uint32_t *err)
{
	char *s = rm(16) ;
	uint8_t low = fetchByte(); 
	uint8_t high = fetchByte(); 
	uint16_t imm16 = ((high << 8) + low) ; 
	snprintf(str, 20, "%s,%04Xh", s, imm16) ;
	return str ;
}

char * disassembler_t::rm8_imm8(uint32_t *err)
{
	char *s = rm(8) ;
	uint8_t imm8 = fetchByte(); 
	snprintf(str, 20, "%s,%02Xh", s, imm8) ;
	return str ;  
}

char * disassembler_t::rel16(uint32_t *err)
{
	uint8_t rel_low = fetchByte(); 
	uint8_t rel_high = fetchByte();
	int16_t rel = ((rel_high << 8) + rel_low) ;
	uint16_t result = pointer.offset + rel;
	snprintf(str, 20, "%04Xh", result) ; 
	return str ;
}

char * disassembler_t::rel8(uint32_t *err)
{
	int8_t rel = fetchByte(); 
	uint16_t result = pointer.offset + rel;
	snprintf(str, 20, "%02Xh", result) ; 
	return str ;
}

char * disassembler_t::imm8(uint32_t *err)
{
	uint8_t imm8 = fetchByte(); 
	snprintf(str, 20, "%02Xh", imm8) ; 
	return str ; 
}

char * disassembler_t::imm16(uint32_t *err)
{
	uint8_t low = fetchByte(); 
	uint8_t high = fetchByte(); 
	uint16_t imm16 = ((high << 8) + low) ;
	snprintf(str, 20, "%04Xh", imm16); 
	return str ;
}

char * disassembler_t::r16_rm16(uint32_t *err)
{
	uint8_t reg = ((lookNext() & 0x38) >> 3 ); 
	char *s = rm(16) ;
	char *reg16 = regs16[reg] ; 
	snprintf(str, 20, "%s,%s", reg16, s) ;
	return str ;
}

char * disassembler_t::rm8_r8(uint32_t *err)
{
	uint8_t reg = ((lookNext() & 0x38) >> 3 ); 
	char *s = rm(8) ;
	char *reg8 = regs8[reg] ; 
	snprintf(str, 20, "%s,%s", s, reg8) ;
	return str ;
}

char * disassembler_t::r8_rm8(uint32_t *err)
{
	uint8_t reg = ((lookNext() & 0x38) >> 3 ); 
	char *s = rm(8) ;
	char *reg8 = regs8[reg] ;
	snprintf(str, 20, "%s,%s", reg8, s) ; 
	return str ;
}

char * disassembler_t::rm16_r16(uint32_t *err)
{
	uint8_t reg = ((lookNext() & 0x38) >> 3 ); 
	char *s = rm(16) ;
	char *reg16 = regs16[reg] ; 
	snprintf(str, 20, "%s,%s", s, reg16) ;
	return str ;
}

char rm_str[255] ; 

char * disassembler_t::rm(uint8_t type)
{
	uint8_t rm_byte = fetchByte(); 
	uint8_t mod = (rm_byte >> 6) ; 
	uint8_t rm8 = (rm_byte & 7) ; 
	char segment[10] = {'\0'};
	if (segment_override >= 0)
	{
		switch (segment_override)
		{
			case ES: snprintf(segment, 10, "es:") ; break ;
			case CS: snprintf(segment, 10, "cs:") ; break ;
			case SS: snprintf(segment, 10, "ss:") ; break ;
			case DS: snprintf(segment, 10, "ds:") ; break ;
		}
	}
	char displacement[255] = {'\0'};
	switch (mod)
	{
		case 0x0:
		{
      if (rm8 == 0x06)
      {
        uint8_t low   = fetchByte();
        uint8_t high  = fetchByte(); 
        uint16_t disp = ((high << 8) + low) ; 
        snprintf(displacement, 40, "+%Xh", disp) ;
      }
      else snprintf(displacement, 1, "") ;
		} break ; 
		case 0x01:
		{
			int16_t disp = fetchByte(); 
			char sign = '+' ; 
			if (disp < 0) 
			{
				sign = '-' ;
				disp = ~disp ;
				disp++ ;
			}
			snprintf(displacement, 10, "%c%Xh", sign, disp) ; 
		} break ;
		case 0x02:
		{
			uint8_t low   = fetchByte();
			uint8_t high  = fetchByte();
			uint16_t disp = ((high << 8) + low) ; 
			snprintf(displacement, 10, "+%Xh", disp) ;
		} break ;
		case 0x03:
		{
			if (type == 8)
			{
 				return regs8[rm8] ;
			}
			if (type == 16)
			{
				return regs16[rm8]; 
			}
		} break ;
	}
	switch (rm8)
	{
		case 0x00: snprintf(rm_str, 20, "[%sbx+si%s]", segment, displacement) ; break ;
		case 0x01: snprintf(rm_str, 20, "[%sbx+di%s]", segment, displacement) ; break ;
		case 0x02: snprintf(rm_str, 20, "[%sbp+si%s]", segment, displacement) ; break ;
		case 0x03: snprintf(rm_str, 20, "[%sbp+di%s]", segment, displacement) ; break ;
		case 0x04: snprintf(rm_str, 20, "[%ssi%s]", segment, displacement) ; break ;
		case 0x05: snprintf(rm_str, 20, "[%sdi%s]", segment, displacement) ; break ;
		case 0x06: snprintf(rm_str, 20, "[%sbp%s]", segment, displacement) ; break ; 
		case 0x07: snprintf(rm_str, 20, "[%sbx%s]", segment, displacement) ; break ;
	}
	return rm_str ; 
}

uint8_t disassembler_t::lookNext()
{
  return read86(pointer.linear());
};
