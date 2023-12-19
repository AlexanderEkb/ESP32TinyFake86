#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "dasm.h"

static char *regs16[8] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"} ; 
static char *regs8[8] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"} ;
static char *segreg[4] = {"es", "cs", "ss", "ds"} ;

 
void disassembler_t::parse(char *instrTemplate, char*(disassembler_t::*func)(uint32_t *))
{
	bytesToPrint = 1;
	uint32_t temp_j = pointer; 
	uint32_t error = 0 ;
	char *result = (this->*func)(&error) ;
	if (error)
	{
		char tmp_buffer[20] ; 
		sprintf(tmp_buffer, "db 0x%X\n", code[pointer]) ;
		parse_noop(tmp_buffer) ;
		return;
	}
	uint32_t spacesToFill = 16 ;
	uint32_t t = 0 ;
	if (segment_override == NO && rm_segment_override >= 0)
	{
		t = 1; 
		spacesToFill -= 2 ;
		switch (rm_segment_override)
		{
			case ES: printf("%02X", 0x26) ;	 break ;
			case CS: printf("%02X", 0x2E) ;	 break ;
			case SS: printf("%02X", 0x36) ;	 break ;
			case DS: printf("%02X", 0x3E) ;	 break ;
		} 
		rm_segment_override = NO ; 
		segment_override = NO ;
	}
	char segment[20] ;

	if (t == 1)
	{
			memset(segment, '\0', 20) ;
			switch (segment_override)
			{
				case ES: sprintf(segment, "es") ; break ;
				case CS: sprintf(segment, "cs") ; break ;
				case SS: sprintf(segment, "ss") ; break ;
				case DS: sprintf(segment, "ds") ; break ;
			}
	}
	if (segment_override >= 0 )
	{
		spacesToFill = spacesToFill - 2 ;
		switch (segment_override)
		{
			case ES: printf("%02X", 0x26) ;	 break ;
			case CS: printf("%02X", 0x2E) ;	 break ;
			case SS: printf("%02X", 0x36) ;	 break ;
			case DS: printf("%02X", 0x3E) ;	 break ;
		}
		segment_override = NO ;
		rm_segment_override = NO ; 
	}
	for (uint32_t i=0; i < bytesToPrint; i++)
	{
		uint8_t byte = code[temp_j+i]  ; 
		printf("%02X", byte) ;	
	}
	spacesToFill = (spacesToFill - (bytesToPrint*2))  ; 
	for (uint32_t i=0; i < spacesToFill; i++) printf(" ") ;
	if (t == 1)
	{
		char tmp_string[255] ; 
		char tmp_string2[255] ; 
		memset(tmp_string, '\0', 255) ;
		memset(tmp_string2, '\0', 255) ;
		sprintf(tmp_string, instrTemplate, result) ; 
		sprintf(tmp_string2, "%s %s", segment, tmp_string) ; 
		printf("%s", tmp_string2) ;
	}
	else printf(instrTemplate, result) ; 
}

uint32_t disassembler_t::parse_noop(char *instrTemplate)
{
	uint32_t spacesToFill = 12; 
	if (segment_override >= 0) 
	{
			switch (segment_override)               // OUT: 2. Bytes
			{
				case ES: printf("%02X", 0x26) ;	 break ;
				case CS: printf("%02X", 0x2E) ;	 break ;
				case SS: printf("%02X", 0x36) ;	 break ;
				case DS: printf("%02X", 0x3E) ;	 break ;
			}
			spacesToFill -= 2 ; 
	}
	uint8_t tmp_char = code[pointer] ; 
	printf("%02X", tmp_char) ;                  // OUT: 2. Bytes

	for (uint32_t i=0; i < spacesToFill; i++) printf(" ") ;

	if (segment_override >= 0)
	{
		switch (segment_override)
		{
			case ES: printf("es") ; break ;         // OUT: (3)Segment override
			case CS: printf("cs") ; break ;
			case SS: printf("ss") ; break ;
			case DS: printf("ds") ; break ;
		}
		segment_override = NO ;
		rm_segment_override = NO ; 
	}
  printf("%s", instrTemplate) ;               // OUT: (4)Instruction
}

/*
1         2           3  4
00000000  3E51        ds push cx
0000C0DE  
*/
void disassembler_t::decode(uint8_t *buffer, uint32_t length)
{
  code = buffer;
  this->length = length;
  segment_override = NO;
  rm_segment_override = NO;
  bytesToPrint = 0;
	pointer = 0; 

	while (pointer < length)
	{
		if (segment_override == NO)
			printf("%08X  ", pointer);  // OUT: 1 - address
		switch (code[pointer])
		{
			case 0x00: parse("add %s\n", &disassembler_t::rm8_r8) ; break ;
			case 0x01: parse("add %s\n", &disassembler_t::rm16_r16) ; break ;
			case 0x02: parse("add %s\n", &disassembler_t::r8_rm8) ; break ;
			case 0x03: parse("add %s\n", &disassembler_t::r16_rm16) ; break ;
			case 0x04: parse("add al,%s\n", &disassembler_t::imm8) ; break ; 
			case 0x05: parse("add ax,%s\n", &disassembler_t::imm16) ; break ;
			case 0x06: parse_noop("push es\n"); break ; 
			case 0x07: parse_noop("pop es\n"); break ;
			case 0x08: parse("or %s\n", &disassembler_t::rm8_r8) ; break ;
			case 0x09: parse("or %s\n", &disassembler_t::rm16_r16) ; break ;
			case 0x0A: parse("or %s\n", &disassembler_t::r8_rm8) ; break ;
			case 0x0B: parse("or %s\n", &disassembler_t::r16_rm16) ; break ;
			case 0x0C: parse("or al,%s\n", &disassembler_t::imm8) ; break ; 
			case 0x0D: parse("or ax,%s\n", &disassembler_t::imm16) ; break ;
			case 0x0E: parse_noop("push cs\n") ; break ;
			case 0x10: parse("adc %s\n", &disassembler_t::rm8_r8) ; break ;
			case 0x11: parse("adc %s\n", &disassembler_t::rm16_r16) ; break ;
			case 0x12: parse("adc %s\n", &disassembler_t::r8_rm8) ; break ;
			case 0x13: parse("adc %s\n", &disassembler_t::r16_rm16) ; break ;
			case 0x14: parse("adc al,%s\n", &disassembler_t::imm8) ; break ; 
			case 0x15: parse("adc ax,%s\n", &disassembler_t::imm16) ; break ;
			case 0x16: parse_noop("push ss\n") ; break ;
			case 0x17: parse_noop("pop ss\n") ; break ;
			case 0x18: parse("sbb %s\n", &disassembler_t::rm8_r8) ; break ;
			case 0x19: parse("sbb %s\n", &disassembler_t::rm16_r16) ; break ;
			case 0x1A: parse("sbb %s\n", &disassembler_t::r8_rm8) ; break ;
			case 0x1B: parse("sbb %s\n", &disassembler_t::r16_rm16) ; break ;
			case 0x1C: parse("sbb al,%s\n", &disassembler_t::imm8) ; break ; 
			case 0x1D: parse("sbb ax,%s\n", &disassembler_t::imm16) ; break ;
			case 0x1E: parse_noop("push ds\n") ; break ;
			case 0x1F: parse_noop("pop ds\n") ; break ;
			case 0x20: parse("and %s\n", &disassembler_t::rm8_r8) ; break ;
			case 0x21: parse("and %s\n", &disassembler_t::rm16_r16) ; break ;
			case 0x22: parse("and %s\n", &disassembler_t::r8_rm8) ; break ;
			case 0x23: parse("and %s\n", &disassembler_t::r16_rm16) ; break ;
			case 0x24: parse("and al,%s\n", &disassembler_t::imm8) ; break ; 
			case 0x25: parse("and ax,%s\n", &disassembler_t::imm16) ; break ;
			case 0x26: 
			{
				segment_override = ES ; 
				rm_segment_override = ES;
			}
			break ;
			case 0x27: parse_noop("daa\n") ; break ;
			case 0x28: parse("sub %s\n", &disassembler_t::rm8_r8) ; break ;
			case 0x29: parse("sub %s\n", &disassembler_t::rm16_r16) ; break ;
			case 0x2A: parse("sub %s\n", &disassembler_t::r8_rm8) ; break ;
			case 0x2B: parse("sub %s\n", &disassembler_t::r16_rm16) ; break ;
			case 0x2C: parse("sub al,%s\n", &disassembler_t::imm8) ; break ; 
			case 0x2D: parse("sub ax,%s\n", &disassembler_t::imm16) ; break ;
			case 0x2E: 
			{
				segment_override = CS ; 
				rm_segment_override = CS ;
			} break ;
			case 0x2F: parse_noop("das\n") ; break ;
			case 0x30: parse("xor %s\n", &disassembler_t::rm8_r8) ; break ;
			case 0x31: parse("xor %s\n", &disassembler_t::rm16_r16) ; break ;
			case 0x32: parse("xor %s\n", &disassembler_t::r8_rm8) ; break ;
			case 0x33: parse("xor %s\n", &disassembler_t::r16_rm16) ; break ;
			case 0x34: parse("xor al,%s\n", &disassembler_t::imm8) ; break ; 
			case 0x35: parse("xor ax,%s\n", &disassembler_t::imm16) ; break ;
			case 0x36: 
			{
				segment_override = SS ; 
				rm_segment_override = SS ; 
			} break ;
			case 0x37: parse_noop("aaa\n") ; break ;
			case 0x38: parse("cmp %s\n", &disassembler_t::rm8_r8) ; break ;
			case 0x39: parse("cmp %s\n", &disassembler_t::rm16_r16) ; break ;
			case 0x3A: parse("cmp %s\n", &disassembler_t::r8_rm8) ; break ;
			case 0x3B: parse("cmp %s\n", &disassembler_t::r16_rm16) ; break ;
			case 0x3C: parse("cmp al,%s\n", &disassembler_t::imm8) ; break ; 
			case 0x3D: parse("cmp ax,%s\n", &disassembler_t::imm16) ; break ;
			case 0x3E: 
			{
				segment_override = DS ; 
				rm_segment_override = DS ; 
			}
			break ;
			case 0x3F: parse_noop("aas\n") ; break ;
			case 0x40: parse_noop("inc ax\n") ; break ;
			case 0x41: parse_noop("inc cx\n") ; break ;
			case 0x42: parse_noop("inc dx\n") ; break ;
			case 0x43: parse_noop("inc bx\n") ; break ;
			case 0x44: parse_noop("inc sp\n") ; break ;
			case 0x45: parse_noop("inc bp\n") ; break ;
			case 0x46: parse_noop("inc si\n") ; break ;
			case 0x47: parse_noop("inc di\n") ; break ;
			case 0x48: parse_noop("dec ax\n") ; break ;
			case 0x49: parse_noop("dec cx\n") ; break ;
			case 0x4A: parse_noop("dec dx\n") ; break ;
			case 0x4B: parse_noop("dec bx\n") ; break ;
			case 0x4C: parse_noop("dec sp\n") ; break ;
			case 0x4D: parse_noop("dec bp\n") ; break ;
			case 0x4E: parse_noop("dec si\n") ; break ;
			case 0x4F: parse_noop("dec di\n") ; break ;
			case 0x50: parse_noop("push ax\n") ; break ;
			case 0x51: parse_noop("push cx\n") ; break ;
			case 0x52: parse_noop("push dx\n") ; break ;
			case 0x53: parse_noop("push bx\n") ; break ;
			case 0x54: parse_noop("push sp\n") ; break ;
			case 0x55: parse_noop("push bp\n") ; break ;
			case 0x56: parse_noop("push si\n") ; break ;
			case 0x57: parse_noop("push di\n") ; break ;
			case 0x58: parse_noop("pop ax\n") ; break ;
			case 0x59: parse_noop("pop cx\n") ; break ;
			case 0x5A: parse_noop("pop dx\n") ; break ;
			case 0x5B: parse_noop("pop bx\n") ; break ;
			case 0x5C: parse_noop("pop sp\n") ; break ;
			case 0x5D: parse_noop("pop bp\n") ; break ;
			case 0x5E: parse_noop("pop si\n") ; break ;
			case 0x5F: parse_noop("pop di\n") ; break ;
			case 0x70: parse("jo %s\n", &disassembler_t::rel8) ; break ;
			case 0x71: parse("jno %s\n", &disassembler_t::rel8) ; break ;
			case 0x72: parse("jc %s\n", &disassembler_t::rel8) ; break ;
			case 0x73: parse("jnc %s\n", &disassembler_t::rel8) ; break ;
			case 0x74: parse("jz %s\n", &disassembler_t::rel8) ; break ;
			case 0x75: parse("jnz %s\n", &disassembler_t::rel8) ; break ;
			case 0x76: parse("jna %s\n", &disassembler_t::rel8) ; break ;
			case 0x77: parse("ja %s\n", &disassembler_t::rel8) ; break ;
			case 0x78: parse("js %s\n", &disassembler_t::rel8) ; break ;
			case 0x79: parse("jns %s\n", &disassembler_t::rel8) ; break ;
			case 0x7A: parse("jpe %s\n", &disassembler_t::rel8) ; break ;
			case 0x7B: parse("jpo %s\n", &disassembler_t::rel8) ; break ;
			case 0x7C: parse("jl %s\n", &disassembler_t::rel8) ; break ;
			case 0x7D: parse("jnl %s\n", &disassembler_t::rel8) ; break ;
			case 0x7E: parse("jng %s\n", &disassembler_t::rel8) ; break ;
			case 0x7F: parse("jg %s\n", &disassembler_t::rel8) ; break ;
			case 0x80:
			{
				uint8_t opcode = ((code[pointer+1] & 0x38) >> 3 ); 
				uint8_t t = 0 ;
				switch (opcode)
				{
					case 0x00: parse("add %s\n", &disassembler_t::rm8_imm8) ; break ;
					case 0x01: parse("or %s\n", &disassembler_t::rm8_imm8) ; break ;
					case 0x02: parse("adc %s\n", &disassembler_t::rm8_imm8) ; break ;
					case 0x03: parse("sbb %s\n", &disassembler_t::rm8_imm8) ; break ;
					case 0x04: parse("and %s\n", &disassembler_t::rm8_imm8) ; break ;
					case 0x05: parse("sub %s\n", &disassembler_t::rm8_imm8) ; break ;
					case 0x06: parse("xor %s\n", &disassembler_t::rm8_imm8) ; break ;
					case 0x07: parse("cmp %s\n", &disassembler_t::rm8_imm8) ; break ;
					default: t = 1; break ;
				}
				if (t) goto print_symbol ;
			} break ;
			case 0x81:
			{
				uint8_t opcode = ((code[pointer+1] & 0x38) >> 3 ); 
				uint8_t t = 0 ;
				switch (opcode)
				{
					case 0x00: parse("add %s\n", &disassembler_t::rm16_imm16) ; break ;
					case 0x01: parse("or %s\n", &disassembler_t::rm16_imm16) ; break ;
					case 0x02: parse("adc %s\n", &disassembler_t::rm16_imm16) ; break ;
					case 0x03: parse("sbb %s\n", &disassembler_t::rm16_imm16) ; break ;
					case 0x04: parse("and %s\n", &disassembler_t::rm16_imm16) ; break ;
					case 0x05: parse("sub %s\n", &disassembler_t::rm16_imm16) ; break ;
					case 0x06: parse("xor %s\n", &disassembler_t::rm16_imm16) ; break ;
					case 0x07: parse("cmp %s\n", &disassembler_t::rm16_imm16) ; break ;
					default: t = 1; break ;
				}
				if (t) goto print_symbol ;
			} break ;
			case 0x83:
			{
				uint8_t opcode = ((code[pointer+1] & 0x38) >> 3 ); 
				uint8_t t = 0 ;
				switch (opcode)
				{
					case 0x00: parse("add %s\n", &disassembler_t::rm16_imm8) ; break ;
					case 0x02: parse("adc %s\n", &disassembler_t::rm16_imm8) ; break ;
					case 0x03: parse("sbb %s\n", &disassembler_t::rm16_imm8) ; break ;
					case 0x05: parse("sub %s\n", &disassembler_t::rm16_imm8) ; break ;
					case 0x07: parse("cmp %s\n", &disassembler_t::rm16_imm8) ; break ;
					default: t = 1; break ;
				} 
				if (t) goto print_symbol ;
			} break ;
			case 0x84: parse("test %s\n", &disassembler_t::rm8_r8) ; break ;
			case 0x85: parse("test %s\n", &disassembler_t::rm16_r16) ; break ;
			case 0x86: parse("xchg %s\n", &disassembler_t::rm8_r8) ; break ;
			case 0x87: parse("xchg %s\n", &disassembler_t::rm16_r16) ; break ;
			case 0x88: parse("mov %s\n", &disassembler_t::rm8_r8) ; break ;
			case 0x89: parse("mov %s\n", &disassembler_t::rm16_r16) ; break ;
			case 0x8A: parse("mov %s\n", &disassembler_t::r8_rm8) ; break ;
			case 0x8B: parse("mov %s\n", &disassembler_t::r16_rm16) ; break ;
			case 0x8C: 
			{ 
				parse("mov %s\n", &disassembler_t::rm16_sreg) ; break ;
			}
			case 0x8D: parse("lea %s\n", &disassembler_t::r16_rm16) ; break ;
			case 0x8E: 
			{
					parse("mov %s\n", &disassembler_t::sreg_rm16) ; break ;
			}
			case 0x8F:
			{
			  	uint8_t opcode = ((code[pointer+1] & 0x38) >> 3 ); 
				uint8_t t = 0 ;
				switch (opcode)
				{
					case 0x00: parse("pop word %s\n", &disassembler_t::m16) ; break ;
					default: t = 1; break ;
				}	
				if (t) goto print_symbol ;
			} break ;
			case 0x90: parse_noop("xchg ax,ax\n") ; break ;
			case 0x91: parse_noop("xchg cx,ax\n") ; break ;
			case 0x92: parse_noop("xchg dx,ax\n") ; break ;
			case 0x93: parse_noop("xchg bx,ax\n") ; break ;
			case 0x94: parse_noop("xchg sp,ax\n") ; break ;
			case 0x95: parse_noop("xchg bp,ax\n") ; break ;
			case 0x96: parse_noop("xchg si,ax\n") ; break ;
			case 0x97: parse_noop("xchg di,ax\n") ; break ;
			case 0x98: parse_noop("cbw\n") ; break ;
			case 0x99: parse_noop("cwd\n") ; break ;
			case 0x9A: parse("call %s\n", &disassembler_t::call_inter) ; break ; 
			case 0x9B: parse_noop("wait\n") ; break ;
			case 0x9C: parse_noop("pushf\n") ; break ;
			case 0x9D: parse_noop("popf\n") ; break ;
			case 0x9E: parse_noop("sahf\n") ; break ;
			case 0x9F: parse_noop("lahf\n") ; break ;
			case 0xA0: parse("mov al,%s\n", &disassembler_t::moffs16) ; break ;
			case 0xA1: parse("mov ax,%s\n", &disassembler_t::moffs16) ; break ;
			case 0xA2: parse("mov %s,al\n", &disassembler_t::moffs16) ; break ;
			case 0xA3: parse("mov %s,ax\n", &disassembler_t::moffs16) ; break ;
			case 0xA4: parse_noop("movsb\n") ; break ;
			case 0xA5: parse_noop("movsw\n") ; break ;
			case 0xA6: parse_noop("cmpsb\n") ; break ;
			case 0xA7: parse_noop("cmpsw\n") ; break ;
			case 0xA8: parse("test al, %s\n", &disassembler_t::imm8) ; break ;
			case 0xA9: parse("test ax, %s\n", &disassembler_t::imm16) ; break ;
			case 0xAA: parse_noop("stosb\n") ; break ;
			case 0xAB: parse_noop("stosw\n") ; break ;
			case 0xAC: parse_noop("lodsb\n") ; break ;
			case 0xAD: parse_noop("lodsw\n") ; break ;
			case 0xAE: parse_noop("scasb\n") ; break ;
			case 0xAF: parse_noop("scasw\n") ; break ;
			case 0xB0: parse("mov al,%s\n",&disassembler_t::imm8); break;
			case 0xB1: parse("mov cl,%s\n",&disassembler_t::imm8); break;
			case 0xB2: parse("mov dl,%s\n",&disassembler_t::imm8); break;
			case 0xB3: parse("mov bl,%s\n",&disassembler_t::imm8); break;
			case 0xB4: parse("mov ah,%s\n",&disassembler_t::imm8); break;
			case 0xB5: parse("mov ch,%s\n",&disassembler_t::imm8); break;
			case 0xB6: parse("mov dh,%s\n",&disassembler_t::imm8); break;
			case 0xB7: parse("mov bh,%s\n",&disassembler_t::imm8); break;
			case 0xB8: parse("mov ax,%s\n",&disassembler_t::imm16); break;
			case 0xB9: parse("mov cx,%s\n",&disassembler_t::imm16); break;
			case 0xBA: parse("mov dx,%s\n",&disassembler_t::imm16); break;
			case 0xBB: parse("mov bx,%s\n",&disassembler_t::imm16); break;
			case 0xBC: parse("mov sp,%s\n",&disassembler_t::imm16); break;
			case 0xBD: parse("mov bp,%s\n",&disassembler_t::imm16); break;
			case 0xBE: parse("mov si,%s\n",&disassembler_t::imm16); break;
			case 0xBF: parse("mov di,%s\n",&disassembler_t::imm16); break;
			case 0xC2: parse("ret %s\n", &disassembler_t::imm16) ; break ; 
			case 0xC3: parse_noop("ret\n") ; break ;
			case 0xC4: parse("les %s\n", &disassembler_t::r16_rm16) ; break ;
			case 0xC5: parse("lds %s\n", &disassembler_t::r16_rm16) ; break ;
			case 0xC6: parse("mov %s\n", &disassembler_t::rm16_imm8) ; break ;
			case 0xC7: parse("mov %s\n", &disassembler_t::rm16_imm16) ; break ;
			case 0xCA: parse("retf %s\n", &disassembler_t::imm16) ; break ;
			case 0xCB: parse_noop("retf\n") ; break ;
			case 0xCC: parse_noop("int3\n") ; break ;
			case 0xCD: parse("int %s\n", &disassembler_t::imm8) ; break ;
			case 0xCE: parse_noop("into\n") ; break ;
			case 0xCF: parse_noop("iret\n") ; break ;
			case 0xD0:
			{
				uint8_t opcode = ((code[pointer+1] & 0x38) >> 3 ); 
				uint8_t t = 0 ;
				switch (opcode)
				{
					case 0x00: parse("rol %s,1\n", &disassembler_t::rm8) ; break ;
					case 0x01: parse("ror %s,1\n", &disassembler_t::rm8) ; break ;
					case 0x02: parse("rcl %s,1\n", &disassembler_t::rm8) ; break ;
					case 0x03: parse("rcr %s,1\n", &disassembler_t::rm8) ; break ;
					case 0x04: parse("shl %s,1\n", &disassembler_t::rm8) ; break ;
					case 0x05: parse("shr %s,1\n", &disassembler_t::rm8) ; break ;
					case 0x07: parse("sar %s,1\n", &disassembler_t::rm8) ; break ;
					default: t = 1; break ;
				}
				if (t) goto print_symbol ;
			} break ;
			case 0xD1:
			{
				uint8_t opcode = ((code[pointer+1] & 0x38) >> 3 ); 
				uint8_t t = 0 ;
				switch (opcode)
				{
					case 0x00: parse("rol %s,1\n", &disassembler_t::rm16) ; break ;
					case 0x01: parse("ror %s,1\n", &disassembler_t::rm16) ; break ;
					case 0x02: parse("rcl %s,1\n", &disassembler_t::rm16) ; break ;
					case 0x03: parse("rcr %s,1\n", &disassembler_t::rm16) ; break ;
					case 0x04: parse("shl %s,1\n", &disassembler_t::rm16) ; break ;
					case 0x05: parse("shr %s,1\n", &disassembler_t::rm16) ; break ;
					case 0x07: parse("sar %s,1\n", &disassembler_t::rm16) ; break ;
					default: t = 1; break ;
				}
				if (t) goto print_symbol ;
			} break ;
			case 0xD2:
			{
				uint8_t opcode = ((code[pointer+1] & 0x38) >> 3 ); 
				uint8_t t = 0 ;
				switch (opcode)
				{
					case 0x00: parse("rol %s,cl\n", &disassembler_t::rm8) ; break ;
					case 0x01: parse("ror %s,cl\n", &disassembler_t::rm8) ; break ;
					case 0x02: parse("rcl %s,cl\n", &disassembler_t::rm8) ; break ;
					case 0x03: parse("rcr %s,cl\n", &disassembler_t::rm8) ; break ;
					case 0x04: parse("shl %s,cl\n", &disassembler_t::rm8) ; break ;
					case 0x05: parse("shr %s,cl\n", &disassembler_t::rm8) ; break ;
					case 0x07: parse("sar %s,cl\n", &disassembler_t::rm8) ; break ;
					default: t = 1; break ;
				}
				if (t) goto print_symbol ;
			} break ;
			case 0xD3:
			{
				uint8_t opcode = ((code[pointer+1] & 0x38) >> 3 ); 
				uint8_t t = 0 ;
				switch (opcode)
				{
					case 0x00: parse("rol %s,cl\n", &disassembler_t::rm16) ; break ;
					case 0x01: parse("ror %s,cl\n", &disassembler_t::rm16) ; break ;
					case 0x02: parse("rcl %s,cl\n", &disassembler_t::rm16) ; break ;
					case 0x03: parse("rcr %s,cl\n", &disassembler_t::rm16) ; break ;
					case 0x04: parse("shl %s,cl\n", &disassembler_t::rm16) ; break ;
					case 0x05: parse("shr %s,cl\n", &disassembler_t::rm16) ; break ;
					case 0x07: parse("sar %s,cl\n", &disassembler_t::rm16) ; break ;
					default: t = 1; break ;
				}
				if (t) goto print_symbol ;
			} break ;
			case 0xD4: parse_noop("aam\n") ; break ;
			case 0xD5: parse_noop("aad\n") ; break ;
			case 0xD7: parse_noop("xlatb\n") ; break ;
			/*D8-DF => ESC0-7*/
			case 0xE0: parse("loopne %s\n", &disassembler_t::rel8) ; break ;
			case 0xE1: parse("loope %s\n", &disassembler_t::rel8) ; break ;
			case 0xE2: parse("loop %s\n", &disassembler_t::rel8) ; break ;
			case 0xE3: parse("jcxz %s\n", &disassembler_t::rel8) ; break ;
			case 0xE4: parse("in al,%s\n", &disassembler_t::imm8) ; break ;
			case 0xE5: parse("in ax,%s\n", &disassembler_t::imm8) ; break ;
			case 0xE6: parse("out %s,al\n", &disassembler_t::imm8) ; break ;
			case 0xE7: parse("out %s,ax\n", &disassembler_t::imm8) ; break ;
			case 0xE8: parse("call %s\n", &disassembler_t::rel16) ; break ;
			case 0xE9: parse("jmp %s\n", &disassembler_t::rel16) ; break ;
			case 0xEA: parse("jmp %s\n", &disassembler_t::call_inter) ; break ;
			case 0xEB: parse("jmp short %s\n", &disassembler_t::rel8) ; break ;
			case 0xEC: parse_noop("in al,dx\n") ; break ;
			case 0xED: parse_noop("in ax,dx\n") ; break ;
			case 0xEE: parse_noop("out dx,al\n") ; break ;
			case 0xEF: parse_noop("out dx,ax\n") ; break ;
			case 0xF0: printf("lock ") ; break ;
			case 0xF2: printf("repne ") ; break ;
			case 0xF3: printf("rep ") ; break ;
			case 0xF4: parse_noop("hlt\n") ; break ;
			case 0xF5: parse_noop("cmc\n") ; break ;
			case 0xF6:
			{
				uint8_t opcode = ((code[pointer+1] & 0x38) >> 3 ); 
				uint8_t t = 0 ;
				switch (opcode)
				{
					case 0x00: parse("test %s\n", &disassembler_t::rm8_imm8) ; break ;
					case 0x02: parse("not %s\n", &disassembler_t::rm8) ; break ;
					case 0x03: parse("neg %s\n", &disassembler_t::rm8) ; break ;
					case 0x04: parse("mul %s\n", &disassembler_t::rm8) ; break ;
					case 0x05: parse("imul %s\n", &disassembler_t::rm8) ; break ;
					case 0x06: parse("div %s\n", &disassembler_t::rm8) ; break ;
					case 0x07: parse("idiv %s\n", &disassembler_t::rm8) ; break ;
					default: t = 1; break ;
				}
				if (t) goto print_symbol ;
			} break ;
			case 0xF7:
			{
				uint8_t opcode = ((code[pointer+1] & 0x38) >> 3 ); 
				uint8_t t = 0 ;
				switch (opcode)
				{
					case 0x00: parse("test %s\n", &disassembler_t::rm16_imm16) ; break ;
					case 0x02: parse("not %s\n", &disassembler_t::rm16) ; break ;
					case 0x03: parse("neg %s\n", &disassembler_t::rm16) ; break ;
					case 0x04: parse("mul %s\n", &disassembler_t::rm16) ; break ;
					case 0x05: parse("imul %s\n", &disassembler_t::rm16) ; break ;
					case 0x06: parse("div %s\n", &disassembler_t::rm16) ; break ;
					case 0x07: parse("idiv %s\n", &disassembler_t::rm16) ; break ;
					default: t = 1; break ;
				}
				if (t) goto print_symbol ;
			} break ;
			case 0xF8: parse_noop("clc\n") ; break ;
			case 0xF9: parse_noop("stc\n") ; break ;
			case 0xFA: parse_noop("cli\n") ; break ;
			case 0xFB: parse_noop("sti\n") ; break ;
			case 0xFC: parse_noop("cld\n") ; break ;
			case 0xFD: parse_noop("std\n") ; break ;
			case 0xFE:
			{
				uint8_t opcode = ((code[pointer+1] & 0x38) >> 3 ); 
				uint8_t t = 0 ;	
				switch (opcode)
				{
					case 0x00: parse("inc %s\n", &disassembler_t::rm8) ; break ;
					case 0x01: parse("dec %s\n", &disassembler_t::rm8) ; break ;
					default: t = 1; break ;
				}
				if (t) goto print_symbol ;
			} break ;
			case 0xFF:
			{
				uint8_t opcode = ((code[pointer+1] & 0x38) >> 3 ); 
				uint8_t t = 0 ;
				switch (opcode)
				{
					case 0x00: parse("inc %s\n", &disassembler_t::rm16) ; break ;
					case 0x01: parse("dec %s\n", &disassembler_t::rm16) ; break ;
					case 0x02: parse("call near %s\n", &disassembler_t::rm16) ; break ;
					case 0x03: parse("call far %s\n", &disassembler_t::rm16) ; break ;
					case 0x04: parse("jmp near %s\n", &disassembler_t::rm16) ; break ;
					case 0x05: parse("jmp far %s\n", &disassembler_t::rm16) ; break ;
					case 0x06: parse("push %s\n", &disassembler_t::rm16) ; break ;
					default: t = 1; break ;
				}
				if (t) goto print_symbol ;
			} break ;
			print_symbol:
			default: 
			{
				char tmp_buffer[20] ; 
				memset(tmp_buffer, '\0', 20) ;
				sprintf(tmp_buffer, "db 0x%X\n", code[pointer]) ;
				parse_noop(tmp_buffer) ;
				break ;
			}
		}
		pointer++ ;
	}
}

char str[255] ; 

char * disassembler_t::moffs16(uint32_t *err)
{
	memset(str, '\0', 255) ;
	char segment[10] ;
	memset(segment, '\0', 10) ;
	if (segment_override >= 0)
	{
		switch (segment_override)
		{
			case ES: sprintf(segment, "es:") ; break ;
			case CS: sprintf(segment, "cs:") ; break ;
			case SS: sprintf(segment, "ss:") ; break ;
			case DS: sprintf(segment, "ds:") ; break ;
		}
		segment_override = NO ;
	}
	pointer++ ; 
	bytesToPrint++ ;
	uint8_t low = code[pointer] ; 
	pointer++ ;
	bytesToPrint++ ; 
	uint8_t high = code[pointer] ; 
	uint16_t imm16 = ((high << 8) + low) ;
	sprintf(str, "[%s0x%x]", segment, imm16) ;
	return str ;
}

char * disassembler_t::rm8(uint32_t *err)
{
	memset(str, '\0', 255) ;
	char *s =  rm(8)  ;
	sprintf(str, "%s", s) ; 
	return str ;
}

char * disassembler_t::rm16(uint32_t *err)
{
	memset(str, '\0', 255) ;
	char *s =  rm(16)  ;
	sprintf(str, "%s", s) ; 
	return str ;
}

char * disassembler_t::call_inter(uint32_t *err)
{
	memset(str, '\0', 255) ;
	pointer++ ;
	bytesToPrint++ ; 
	uint8_t offset_low = code[pointer] ; 
	pointer++ ;
	bytesToPrint++ ;
	uint8_t offset_high = code[pointer] ; 
	pointer++ ;
	bytesToPrint++ ;
	uint8_t seg_low = code[pointer] ; 
	pointer++ ;
	bytesToPrint++ ;
	uint8_t seg_high = code[pointer] ; 
	uint16_t offset = ((offset_high << 8) + offset_low) ; 
	uint16_t seg = ((seg_high << 8) + seg_low) ;
	sprintf(str,"0x%x:0x%x", seg, offset) ;
	return str ;	
}

char * disassembler_t::m16(uint32_t *err)
{
	memset(str, '\0', 255) ;
	char *s =  rm(16)  ;
	sprintf(str,"%s", s) ;
	return str ;
}

char * disassembler_t::sreg_rm16(uint32_t *error) 
{
	memset(str, '\0', 255) ;
	uint8_t reg = ((code[pointer+1] & 0x38) >> 3) ;
	char *s =  rm(16)  ;
	if (reg < 4)
	{
		char *sreg = segreg[reg] ;
		sprintf(str,"%s,%s", sreg, s) ;
		*error = 0 ;
	} else *error = 1 ; 
	return str ;
}

char * disassembler_t::rm16_sreg(uint32_t *error)
{
	memset(str, '\0', 255) ;
	uint8_t reg = ((code[pointer+1] & 0x38) >> 3) ;
	char *s =  rm(16)  ;
	if (reg < 4)
	{
		char *sreg = segreg[reg] ;
		sprintf(str,"%s,%s", s, sreg) ;
		*error = 0 ;
	} else *error = 1 ; 
	return str ;
}

char * disassembler_t::rm16_imm8(uint32_t *err)
{
	memset(str, '\0', 255) ;
	char *s = rm(16) ;
	pointer++; 
	bytesToPrint++ ;
	int8_t imm8 = code[pointer] ; 
	char sign = '+' ;
	if (imm8 < 0) 
	{
		sign = '-' ;
		imm8 = -imm8 ;
	}
	sprintf(str, "%s,byte %c0x%x", s, sign, imm8) ;
	return str ;
}

char * disassembler_t::rm16_imm16(uint32_t *err)
{
	memset(str, '\0', 255) ;
	char *s = rm(16) ;
	pointer++;
	bytesToPrint++ ;
	uint8_t low = code[pointer] ; 
	pointer++ ;
	bytesToPrint++ ;
	uint8_t high = code[pointer] ; 
	uint16_t imm16 = ((high << 8) + low) ; 
	sprintf(str, "%s,0x%x", s, imm16) ;
	return str ;
}

char * disassembler_t::rm8_imm8(uint32_t *err)
{
	memset(str, '\0', 255) ;
	char *s = rm(8) ;
	pointer++;
	bytesToPrint++ ;
	uint8_t imm8 = code[pointer] ; 
	sprintf(str, "%s,0x%x", s, imm8) ;
	return str ;  
}

char * disassembler_t::rel16(uint32_t *err)
{
	memset(str, '\0', 255) ;
	pointer++ ;
	bytesToPrint++ ;
	uint8_t rel_low = code[pointer] ; 
	pointer++ ;
	bytesToPrint++ ; 
	uint8_t rel_high = code[pointer] ; 
	int16_t rel = ((rel_high << 8) + rel_low) ;
	uint16_t result = pointer + rel + 1 ;
	sprintf(str, "0x%x", result) ; 
	return str ;
}

char * disassembler_t::rel8(uint32_t *err)
{
	memset(str, '\0', 255) ;
	pointer++ ;
	bytesToPrint++ ;
	int8_t rel = code[pointer] ; 
	uint16_t result = pointer + rel + 1 ;
	sprintf(str, "0x%x", result) ; 
	return str ;
}

char * disassembler_t::imm8(uint32_t *err)
{
	memset(str, '\0', 255) ;
	pointer++ ;
	bytesToPrint++ ;
	uint8_t imm8 = code[pointer] ; 
	sprintf(str, "0x%x", imm8) ; 
	return str ; 
}

char * disassembler_t::imm16(uint32_t *err)
{
	memset(str, '\0', 255) ;
	pointer++ ; 
	bytesToPrint++ ;
	uint8_t low = code[pointer] ; 
	pointer++ ;
	bytesToPrint++ ;
	uint8_t high = code[pointer] ; 
	uint16_t imm16 = ((high << 8) + low) ;
	sprintf(str, "0x%x", imm16); 
	return str ;
}

char * disassembler_t::r16_rm16(uint32_t *err)
{
	memset(str, '\0', 255) ;
	uint8_t reg = ((code[pointer+1] & 0x38) >> 3 ); 
	char *s = rm(16) ;
	char *reg16 = regs16[reg] ; 
	sprintf(str, "%s,%s", reg16, s) ;
	return str ;
}

char * disassembler_t::rm8_r8(uint32_t *err)
{
	memset(str, '\0', 255) ;
	uint8_t reg = ((code[pointer+1] & 0x38) >> 3 ); 
	char *s = rm(8) ;
	char *reg8 = regs8[reg] ; 
	sprintf(str, "%s,%s", s, reg8) ;
	return str ;
}

char * disassembler_t::r8_rm8(uint32_t *err)
{
	memset(str, '\0', 255) ;
	uint8_t reg = ((code[pointer+1] & 0x38) >> 3 ); 
	char *s = rm(8) ;
	char *reg8 = regs8[reg] ;
	sprintf(str, "%s,%s", reg8, s) ; 
	return str ;
}

char * disassembler_t::rm16_r16(uint32_t *err)
{
	memset(str, '\0', 255) ; 
	uint8_t reg = ((code[pointer+1] & 0x38) >> 3 ); 
	char *s = rm(16) ;
	char *reg16 = regs16[reg] ; 
	sprintf(str, "%s,%s", s, reg16) ;
	return str ;
}

char rm_str[255] ; 

char * disassembler_t::rm(uint8_t type)
{
	bytesToPrint++ ;
	uint8_t rm_byte = code[++pointer] ; 
	uint8_t mod = (rm_byte >> 6) ; 
	uint8_t rm8 = (rm_byte & 7) ; 
	char segment[10] = {'\0'};
	memset(rm_str, '\0', 255) ; 
	if (segment_override >= 0)
	{
		switch (segment_override)
		{
			case ES: sprintf(segment, "es:") ; break ;
			case CS: sprintf(segment, "cs:") ; break ;
			case SS: sprintf(segment, "ss:") ; break ;
			case DS: sprintf(segment, "ds:") ; break ;
		}
	}
	char displacement[255] = {'\0'};
	switch (mod)
	{
		case 0x0:
		{
      if (rm8 == 0x06)
      {
        bytesToPrint += 2;
        uint8_t low   = code[++pointer] ;
        uint8_t high  = code[++pointer] ; 
        uint16_t disp = ((high << 8) + low) ; 
        sprintf(displacement, "+%Xh", disp) ;
      }
      else sprintf(displacement, "") ;
		} break ; 
		case 0x01:
		{
			int16_t disp = code[++pointer]; 
			bytesToPrint++ ;
			char sign = '+' ; 
			if (disp < 0) 
			{
				sign = '-' ;
				disp = ~disp ;
				disp++ ;
			}
			sprintf(displacement, "%c%Xh", sign, disp) ; 
		} break ;
		case 0x02:
		{
			bytesToPrint += 2;
			uint8_t low   = code[++pointer] ;
			uint8_t high  = code[++pointer] ;
			uint16_t disp = ((high << 8) + low) ; 
			sprintf(displacement, "+%Xh", disp) ;
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
		case 0x00: sprintf(rm_str, "[%sbx+si%s]", segment, displacement) ; break ;
		case 0x01: sprintf(rm_str, "[%sbx+di%s]", segment, displacement) ; break ;
		case 0x02: sprintf(rm_str, "[%sbp+si%s]", segment, displacement) ; break ;
		case 0x03: sprintf(rm_str, "[%sbp+di%s]", segment, displacement) ; break ;
		case 0x04: sprintf(rm_str, "[%ssi%s]", segment, displacement) ; break ;
		case 0x05: sprintf(rm_str, "[%sdi%s]", segment, displacement) ; break ;
		case 0x06: sprintf(rm_str, "[%sbp%s]", segment, displacement) ; break ; 
		case 0x07: sprintf(rm_str, "[%sbx%s]", segment, displacement) ; break ;
	}
	return rm_str ; 
}
