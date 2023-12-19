#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ASSERT(EXPR)                                                                     \
  if (!(EXPR))                                                                           \
  {                                                                                      \
    fprintf(stderr, "Assert failed [%s():%d]: if (%s) ..\n", __func__, __LINE__, #EXPR); \
    *(volatile int *)0 = 0;                                                              \
  }
enum op_mode : uint8_t
{
  REGISTER,
  MEMORY,
  IMMEDIATE,
  DIRECT_ADDRESS
};

struct operand
{
  char *value;
  enum op_mode mode;
  uint16_t disp;
  uint16_t data;
  uint16_t direct_address;
};

struct instruction
{
  char *name; // TODO: static buffer?

  /* width
   *
   * 0: 8-bit (byte)
   * 1: 16-bit (word)
   */
  uint8_t w;

  /* direction
   *
   * 0: REG is source
   * 1: REG is destination
   */
  uint8_t d;

  /* signed bit extension
   *
   * 0: no sign extension
   * 1: sign extend 8-bit immediate data to 16-bits if w=1
   */
  uint8_t s;

  /* shift/rotate
   *
   * 0: shift/rotate count is 1
   * 1: shift/rotate count in CL register
   */
  uint8_t v;

  /* repeat/loop
   *
   * 0: repeat/loop while zero flag is clear
   * 1: repeat/loop while zero flag is set
   */
  uint8_t z;

  uint8_t mod;
  uint8_t reg;
  uint8_t rm;

  uint16_t disp;
  uint16_t data;

  struct operand operands[2];
};

static char *registers[][2] = {
    [0b000] = {"al", "ax"},
    [0b001] = {"cl", "cx"},
    [0b010] = {"dl", "dx"},
    [0b011] = {"bl", "bx"},
    [0b100] = {"ah", "sp"},
    [0b101] = {"ch", "bp"},
    [0b110] = {"dh", "si"},
    [0b111] = {"bh", "di"},
};
static char *eac_table[] = {
    [0b000] = "bx + si",
    [0b001] = "bx + di",
    [0b010] = "bp + si",
    [0b011] = "bp + di",
    [0b100] = "si",
    [0b101] = "di",
    [0b110] = "bp",
    [0b111] = "bx",
};

static void instruction_print(struct instruction *inst)
{
  char *separators[2] = {", ", "\n"};

  fprintf(fp, "%s ", inst->name);
  for (int i = 0; i < 2; i++)
  {
    struct operand *op = &inst->operands[i];

    if (op->mode == REGISTER)
    {
      fprintf(fp, "%s", op->value);
    }
    else if (op->mode == MEMORY)
    {
      if (inst->w)
      {
        fprintf(fp, "word ");
      }
      else
      {
        fprintf(fp, "byte ");
      }

      fprintf(fp, "[%s", op->value);
      if (op->disp)
      {
        // fprintf (fp, " + %d", (int16_t) op->disp);
        fprintf(fp, " + %d", op->disp);
      }
      fprintf(fp, "]");
    }
    else if (op->mode == IMMEDIATE)
    {
      ASSERT(op->data);

      fprintf(fp, "%d", (int16_t)op->data);
    }
    else if (op->mode == DIRECT_ADDRESS)
    {
      fprintf(fp, "word [%d]", (int16_t)op->direct_address);
    }

    fprintf(fp, "%s", separators[i]);
  }
}

static uint8_t decode_displacement(uint8_t *buf, struct instruction *inst)
{
  uint8_t i = 0;

  switch (inst->mod)
  {
  case 0b00:
  {
    /**
     * Memory mode, no displacements follows
     * (Except when R/M = 110, then 16-bit
     * displacement follows)
     */
    if (inst->rm == 0b110)
    {
      uint8_t disp_low = buf[i++];
      uint8_t disp_high = buf[i++];
      inst->disp = (disp_high << 8) | disp_low;
    }
  }
  break;
  case 0b01:
  {
    /**
     * Memory mode, 8-bit displacement follows
     */
    inst->disp = buf[i++];
    // TODO: do we need to sign-extend?
    // Page 4-20:
    // If the displacement is only a single byte, the 8086
    // or 8088 automatically sign-extends this quantity to 16-bits
    // before using the information in further address calculations.
    //            if (inst->disp)
    //            {
    //                inst->disp |= 0xFF << 8;
    //            }
  }
  break;
  case 0b10:
  {
    /**
     * Memory mode, 16-bit displacement follows
     */
    uint8_t disp_low = buf[i++];
    uint8_t disp_high = buf[i++];
    inst->disp = (disp_high << 8) | disp_low;
  }
  break;
  case 0b11:
  {
    /**
     * Register mode (no displacement)
     */
  }
  break;
  }

  return i;
}

static void operand_set(struct instruction *inst, struct operand *op, uint8_t mode, uint8_t register_index)
{
  op->mode = static_cast<op_mode>(mode);
  if (op->mode == REGISTER)
  {
    op->value = registers[register_index][inst->w];
  }
  else if (op->mode == MEMORY)
  {
    op->value = eac_table[inst->rm];

    if (inst->disp)
    {
      op->disp = inst->disp;
    }
  }
  else if (op->mode == DIRECT_ADDRESS)
  {
    // TODO: direct_adddress may not be needed
    op->direct_address = inst->disp;
    op->disp = inst->disp;
  }
}

// TODO: maybe split this into separate decode_dst() and decode_src() ??
// -- or maybe even decode_mem/reg/imm/direct() ??
static void decode_operands(struct instruction *inst)
{
  switch (inst->mod)
  {
  case 0b00:
  case 0b01:
  case 0b10:
  {
    uint8_t dst_mode;
    uint8_t src_mode = inst->d ? MEMORY : REGISTER;
    uint8_t register_index = inst->reg;

    if (inst->mod == 0b00 &&
        inst->rm == 0b110)
    {
      dst_mode = DIRECT_ADDRESS;
      register_index = 0;
    }
    else
    {
      dst_mode = inst->d ? REGISTER : MEMORY;
    }

    operand_set(inst, &inst->operands[0], dst_mode, register_index);
    operand_set(inst, &inst->operands[1], src_mode, register_index);
  }
  break;
  case 0b11:
  {
    uint8_t dst_index = inst->d ? inst->reg : inst->rm;
    uint8_t src_index = inst->d ? inst->rm : inst->reg;

    operand_set(inst, &inst->operands[0], REGISTER, dst_index);
    operand_set(inst, &inst->operands[1], REGISTER, src_index);
  }
  }
}

static uint8_t decode_mov_rm2r(uint8_t *buf)
{
  uint8_t i = 0;
  struct instruction inst = {.name = "mov"};
  uint8_t b0 = buf[i++];
  uint8_t b1 = buf[i++];

  ASSERT((b0 >> 2) == 0b100010);

  inst.d = (b0 & 0b00000010) >> 1;
  inst.w = (b0 & 0b00000001);

  inst.mod = (b1 & 0b11000000) >> 6;
  inst.reg = (b1 & 0b00111000) >> 3;
  inst.rm = (b1 & 0b00000111);

  i += decode_displacement(&buf[i], &inst);
  inst.disp = (int16_t)inst.disp; // sign-extension
  decode_operands(&inst);

  instruction_print(&inst);

  return i;
}

static uint8_t decode_mov_i2rm(uint8_t *buf)
{
  printf("ERROR-not-implemented: decoding mov (immediate-to-reg/mem)\n");
  return 0;
}

static uint8_t decode_mov_i2r(uint8_t *buf)
{
  uint8_t i = 0;
  uint8_t b0 = buf[i++];

  uint8_t W = (b0 & 0b1000) >> 3;
  uint8_t REG = (b0 & 0b0111);
  uint16_t data = buf[i++];

  if (W == 1)
  {
    data |= buf[i++] << 8;
  }

  fprintf(fp, "mov %s, %u\n", registers[REG][W], data);

  return i;
}

static uint8_t decode_add_r2r(uint8_t *buf)
{
  struct instruction inst = {.name = "add"};
  uint8_t i = 0;
  uint8_t b0 = buf[i++];
  uint8_t b1 = buf[i++];

  inst.d = (b0 & 0b10) != 0; // 0000 0010
  inst.w = (b0 & 0b01) != 0; // 0000 0001

  inst.mod = (b1 >> 6) & 0b11;  // 1100 0000
  inst.reg = (b1 >> 3) & 0b111; // 0011 1000
  inst.rm = b1 & 0b111;         // 0000 0111

  i += decode_displacement(&buf[i], &inst);
  decode_operands(&inst);

  instruction_print(&inst);

  return i;
}

static uint8_t decode_add_i2rm(uint8_t *buf)
{
  struct instruction inst = {.name = "add"};
  uint8_t i = 0;

  uint8_t b0 = buf[i++];
  uint8_t b1 = buf[i++];

  inst.s = (b0 & 0b10) != 0; // 0000 0010
  inst.w = (b0 & 0b01) != 0; // 0000 0001

  inst.mod = (b1 >> 6) & 0b11;  // 1100 0000
  inst.reg = (b1 >> 3) & 0b111; // 0011 1000
  inst.rm = b1 & 0b111;         // 0000 0111

  i += decode_displacement(&buf[i], &inst);
  decode_operands(&inst);

  struct operand *src = &inst.operands[1];
  src->mode = IMMEDIATE;
  src->data = buf[i++];
  if (inst.s == 0 && inst.w == 1)
  {
    src->data |= 0xFF << 8; // TODO: should be buf[i++] << 8
  }
  instruction_print(&inst);

  return i;
}

static uint8_t decode_add_i2a(uint8_t *buf)
{
  struct instruction inst = {.name = "add"};
  uint8_t i = 0;

  uint8_t b0 = buf[i++];

  ASSERT((b0 >> 1) == 0b0000010);

  inst.w = (b0 & 0b01) != 0; // 0000 0001
  i += decode_displacement(&buf[i], &inst);

  struct operand *dst = &inst.operands[0];
  dst->value = registers[0][inst.w];
  dst->mode = REGISTER;

  struct operand *src = &inst.operands[1];
  src->mode = IMMEDIATE;
  src->data = buf[i++];
  if (inst.w)
  {
    src->data |= buf[i++] << 8;
  }

  instruction_print(&inst);

  return i;
}

static uint8_t decode_sub_r2r(uint8_t *buf)
{
  struct instruction inst = {.name = "sub"};
  uint8_t i = 0;

  uint8_t b0 = buf[i++];
  uint8_t b1 = buf[i++];

  ASSERT((b0 >> 2) == 0b001010);

  inst.d = (b0 & 0b00000010) >> 1;
  inst.w = (b0 & 0b00000001);

  inst.mod = (b1 & 0b11000000) >> 6;
  inst.reg = (b1 & 0b00111000) >> 3;
  inst.rm = (b1 & 0b00000111);

  i += decode_displacement(&buf[i], &inst);
  decode_operands(&inst);

  instruction_print(&inst);

  return i;
}

static uint8_t decode_sub_ifrm(uint8_t *buf)
{
  struct instruction inst = {.name = "sub"};
  uint8_t i = 0;

  uint8_t b0 = buf[i++];
  uint8_t b1 = buf[i++];

  inst.s = (b0 & 0b10) != 0; // 0000 0010
  inst.w = (b0 & 0b01) != 0; // 0000 0001

  inst.mod = (b1 >> 6) & 0b11;  // 1100 0000
  inst.reg = (b1 >> 3) & 0b111; // 0011 1000
  inst.rm = b1 & 0b111;         // 0000 0111

  i += decode_displacement(&buf[i], &inst);
  decode_operands(&inst);

  struct operand *src = &inst.operands[1];
  src->mode = IMMEDIATE;
  src->data = buf[i++];
  if (inst.s == 0 && inst.w == 1)
  {
    src->data |= 0xFF << 8; // TODO: should be buf[i++] << 8
  }

  instruction_print(&inst);

  return i;
}

static uint8_t decode_sub_ifa(uint8_t *buf)
{
  struct instruction inst = {.name = "sub"};
  uint8_t i = 0;

  uint8_t b0 = buf[i++];

  ASSERT((b0 >> 1) == 0b0010110);

  inst.w = (b0 & 0b01) != 0; // 0000 0001
  i += decode_displacement(&buf[i], &inst);

  struct operand *dst = &inst.operands[0];
  dst->value = registers[0][inst.w];
  dst->mode = REGISTER;

  struct operand *src = &inst.operands[1];
  src->mode = IMMEDIATE;
  src->data = buf[i++];
  if (inst.w)
  {
    src->data |= buf[i++] << 8;
  }

  instruction_print(&inst);

  return i;
}

static uint8_t decode_cmp_rmnr(uint8_t *buf)
{
  struct instruction inst = {.name = "cmp"};
  uint8_t i = 0;

  uint8_t b0 = buf[i++];
  uint8_t b1 = buf[i++];

  ASSERT((b0 >> 2) == 0b001110);

  inst.d = (b0 & 0b00000010) >> 1;
  inst.w = (b0 & 0b00000001);

  inst.mod = (b1 & 0b11000000) >> 6;
  inst.reg = (b1 & 0b00111000) >> 3;
  inst.rm = (b1 & 0b00000111);

  i += decode_displacement(&buf[i], &inst);
  decode_operands(&inst);

  instruction_print(&inst);

  return i;
}

static uint8_t decode_cmp_iwrm(uint8_t *buf)
{
  struct instruction inst = {.name = "cmp"};
  uint8_t i = 0;

  uint8_t b0 = buf[i++];
  uint8_t b1 = buf[i++];

  ASSERT((b0 >> 2) == 0b100000);

  inst.s = (b0 & 0b10) != 0; // 0000 0010
  inst.w = (b0 & 0b01) != 0; // 0000 0001

  inst.mod = (b1 >> 6) & 0b11;  // 1100 0000
  inst.reg = (b1 >> 3) & 0b111; // 0011 1000
  inst.rm = b1 & 0b111;         // 0000 0111

  i += decode_displacement(&buf[i], &inst);

  // expect: cmp ax, 1000
  // actual: cmp (null), 232

  decode_operands(&inst);

  struct operand *src = &inst.operands[1];
  src->mode = IMMEDIATE;
  src->data = buf[i++];
  if (inst.s == 1 && inst.w == 1)
  {
    src->data = (uint8_t)src->data;
  }

  instruction_print(&inst);

  return i;
}

static uint8_t decode_cmp_iwa(uint8_t *buf)
{
  struct instruction inst = {.name = "cmp"};
  uint8_t i = 0;

  uint8_t b0 = buf[i++];

  ASSERT((b0 >> 1) == 0b0011110);

  inst.w = (b0 & 0b1);
  inst.d = 1;       // implied
  inst.reg = 0b000; // implied

  decode_operands(&inst);

  struct operand *src = &inst.operands[i];
  src->mode = IMMEDIATE;
  src->data = buf[i++];
  if (inst.w == 1)
  {
    src->data |= buf[i++] << 8;
  }

  instruction_print(&inst);

  return i;
}

static uint8_t decode_shared_100000xx(uint8_t *buf)
{
  uint8_t i = 0;
  uint8_t b0 = buf[i++];

  ASSERT((b0 >> 2) == 0b100000);

  uint8_t b1 = buf[i++];
  uint8_t reg = (b1 & 0b00111000) >> 3;

  if (reg == 0b000)
  {
    i = decode_add_i2rm(buf);
  }
  else if (reg == 0b101)
  {
    i = decode_sub_ifrm(buf);
  }
  else if (reg == 0b111)
  {
    i = decode_cmp_iwrm(buf);
  }

  return i;
}

typedef uint8_t(decode_f)(uint8_t *buf);

static decode_f *decode_table[] = {
    /* mov (register/memory to/from register)
     * 100010xx */
    [0b10001000] = decode_mov_rm2r,
    [0b10001001] = decode_mov_rm2r,
    [0b10001010] = decode_mov_rm2r,
    [0b10001011] = decode_mov_rm2r,
    /* mov (immediate to register/memory)
     * 1100011x */
    [0b11000110] = decode_mov_i2rm,
    [0b11000111] = decode_mov_i2rm,
    /* mov (immediate to register)
     * 1011xxxx */
    [0b10110000] = decode_mov_i2r,
    [0b10110001] = decode_mov_i2r,
    [0b10110010] = decode_mov_i2r,
    [0b10110011] = decode_mov_i2r,
    [0b10110100] = decode_mov_i2r,
    [0b10110101] = decode_mov_i2r,
    [0b10110110] = decode_mov_i2r,
    [0b10110111] = decode_mov_i2r,
    [0b10111000] = decode_mov_i2r,
    [0b10111001] = decode_mov_i2r,
    [0b10111010] = decode_mov_i2r,
    [0b10111011] = decode_mov_i2r,
    [0b10111100] = decode_mov_i2r,
    [0b10111101] = decode_mov_i2r,
    [0b10111110] = decode_mov_i2r,
    [0b10111111] = decode_mov_i2r,

    /* add (reg/memory with reg to either)
     * 000000xx */
    [0b00000000] = decode_add_r2r,
    [0b00000001] = decode_add_r2r,
    [0b00000010] = decode_add_r2r,
    [0b00000011] = decode_add_r2r,
    /* add (immediate to reg/memory)
     * 100000xx */
    [0b10000000] = decode_shared_100000xx,
    [0b10000001] = decode_shared_100000xx,
    [0b10000010] = decode_shared_100000xx,
    [0b10000011] = decode_shared_100000xx,
    /* add (immediate to accumulator)
     * 0000010x */
    [0b00000100] = decode_add_i2a,
    [0b00000101] = decode_add_i2a,

    /* sub (reg/memory and register to either)
     * 001010xx */
    [0b00101000] = decode_sub_r2r,
    [0b00101001] = decode_sub_r2r,
    [0b00101010] = decode_sub_r2r,
    [0b00101011] = decode_sub_r2r,
    /* sub (immediate from reg/memory)
     * 100000xx */
    [0b10000000] = decode_shared_100000xx,
    [0b10000001] = decode_shared_100000xx,
    [0b10000010] = decode_shared_100000xx,
    [0b10000011] = decode_shared_100000xx,
    /* sub (immediate from accumulator)
     * 0010110x */
    [0b00101100] = decode_sub_ifa,
    [0b00101101] = decode_sub_ifa,

    /* cmp (reg/memory and register)
     * 001110xx */
    [0b00111000] = decode_cmp_rmnr,
    [0b00111001] = decode_cmp_rmnr,
    [0b00111010] = decode_cmp_rmnr,
    [0b00111011] = decode_cmp_rmnr,

    /* cmp (immediate and reg/memory)
     * 100000xx */
    [0b10000000] = decode_shared_100000xx,
    [0b10000001] = decode_shared_100000xx,
    [0b10000010] = decode_shared_100000xx,
    [0b10000011] = decode_shared_100000xx,

    [0b00111100] = decode_cmp_iwa,
    [0b00111101] = decode_cmp_iwa,
};

static void decode(uint8_t *data, int len)
{
  int bytes_consumed = 0;

  for (int i = 0; i < len; i += bytes_consumed)
  {
    uint8_t *ptr = &data[i];
    if (decode_table[*ptr])
    {
      bytes_consumed = decode_table[*ptr](ptr);
      if (bytes_consumed == 0)
      {
        return;
      }
    }
    else
    {
      printf("decode function for [ %02X ] not found\n", *ptr);
      return;
    }
  }
}
