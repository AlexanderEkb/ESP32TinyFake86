void opcode0x00()
{ /* 00 ADD Eb Gb */
  modregrm();
  oper1b = readrm8(rm);
  oper2b = getreg8(reg);
  op_add8();
  writerm8(rm, res8);
}

void opcode0x01()
{ /* 01 ADD Ev Gv */
  modregrm();
  oper1 = readrm16(rm);
  oper2 = getreg16(reg);
  op_add16();
  writerm16(rm, res16);
}

void opcode0x02()
{ /* 02 ADD Gb Eb */
  modregrm();
  oper1b = getreg8(reg);
  oper2b = readrm8(rm);
  op_add8();
  putreg8(reg, res8);
}

void opcode0x03()
{ /* 03 ADD Gv Ev */
  modregrm();
  oper1 = getreg16(reg);
  oper2 = readrm16(rm);
  op_add16();
  putreg16(reg, res16);
}

void opcode0x04()
{ /* 04 ADD regs.byteregs[regal] Ib */
  oper1b = regs.byteregs[regal];
  oper2b = getmem8(segregs[regcs], ip);
  StepIP(1);
  op_add8();
  regs.byteregs[regal] = res8;
}

void opcode0x05()
{ /* 05 ADD eAX Iv */
  oper1 = (getreg16(regax));
  oper2 = getmem16(segregs[regcs], ip);
  StepIP(2);
  op_add16();
  putreg16(regax, res16);
}

void opcode0x06()
{ /* 06 PUSH segregs[reges] */
  push(segregs[reges]);
}

void opcode0x07()
{ /* 07 POP segregs[reges] */
  segregs[reges] = pop();
}

void opcode0x08()
{ /* 08 OR Eb Gb */
  modregrm();
  oper1b = readrm8(rm);
  oper2b = getreg8(reg);
  op_or8();
  writerm8(rm, res8);
}

void opcode0x09()
{ /* 09 OR Ev Gv */
  modregrm();
  oper1 = readrm16(rm);
  oper2 = getreg16(reg);
  op_or16();
  writerm16(rm, res16);
}

void opcode0x0A()
{ /* 0A OR Gb Eb */
  modregrm();
  oper1b = getreg8(reg);
  oper2b = readrm8(rm);
  op_or8();
  putreg8(reg, res8);
}

void opcode0x0B()
{ /* 0B OR Gv Ev */
  modregrm();
  oper1 = getreg16(reg);
  oper2 = readrm16(rm);
  op_or16();
  if ((oper1 == 0xF802) && (oper2 == 0xF802))
  {
    sf = 0; /* cheap hack to make Wolf 3D think we're a 286 so it plays */
  }

  putreg16(reg, res16);
}

void opcode0x0C()
{ /* 0C OR regs.byteregs[regal] Ib */
  oper1b = regs.byteregs[regal];
  oper2b = getmem8(segregs[regcs], ip);
  StepIP(1);
  op_or8();
  regs.byteregs[regal] = res8;
}

void opcode0x0D()
{ /* 0D OR eAX Iv */
  oper1 = getreg16(regax);
  oper2 = getmem16(segregs[regcs], ip);
  StepIP(2);
  op_or16();
  putreg16(regax, res16);
}

void opcode0x0E()
{ /* 0E PUSH segregs[regcs] */
  push(segregs[regcs]);
}

void opcode0x0F()
{ // 0F POP CS
#ifndef CPU_V20
  segregs[regcs] = pop();
#endif
}

void opcode0x10()
{ /* 10 ADC Eb Gb */
  modregrm();
  oper1b = readrm8(rm);
  oper2b = getreg8(reg);
  op_adc8();
  writerm8(rm, res8);
}

void opcode0x11()
{ /* 11 ADC Ev Gv */
  modregrm();
  oper1 = readrm16(rm);
  oper2 = getreg16(reg);
  op_adc16();
  writerm16(rm, res16);
}

void opcode0x12()
{ /* 12 ADC Gb Eb */
  modregrm();
  oper1b = getreg8(reg);
  oper2b = readrm8(rm);
  op_adc8();
  putreg8(reg, res8);
}

void opcode0x13()
{ /* 13 ADC Gv Ev */
  modregrm();
  oper1 = getreg16(reg);
  oper2 = readrm16(rm);
  op_adc16();
  putreg16(reg, res16);
}

void opcode0x14()
{ /* 14 ADC regs.byteregs[regal] Ib */
  oper1b = regs.byteregs[regal];
  oper2b = getmem8(segregs[regcs], ip);
  StepIP(1);
  op_adc8();
  regs.byteregs[regal] = res8;
}

void opcode0x15()
{ /* 15 ADC eAX Iv */
  oper1 = getreg16(regax);
  oper2 = getmem16(segregs[regcs], ip);
  StepIP(2);
  op_adc16();
  putreg16(regax, res16);
}

void opcode0x16()
{ /* 16 PUSH segregs[regss] */
  push(segregs[regss]);
}

void opcode0x17()
{ /* 17 POP segregs[regss] */
  segregs[regss] = pop();
}

void opcode0x18()
{ /* 18 SBB Eb Gb */
  modregrm();
  oper1b = readrm8(rm);
  oper2b = getreg8(reg);
  op_sbb8();
  writerm8(rm, res8);
}

void opcode0x19()
{ /* 19 SBB Ev Gv */
  modregrm();
  oper1 = readrm16(rm);
  oper2 = getreg16(reg);
  op_sbb16();
  writerm16(rm, res16);
}

void opcode0x1A()
{ /* 1A SBB Gb Eb */
  modregrm();
  oper1b = getreg8(reg);
  oper2b = readrm8(rm);
  op_sbb8();
  putreg8(reg, res8);
}

void opcode0x1B()
{ /* 1B SBB Gv Ev */
  modregrm();
  oper1 = getreg16(reg);
  oper2 = readrm16(rm);
  op_sbb16();
  putreg16(reg, res16);
}

void opcode0x1C()
{ /* 1C SBB regs.byteregs[regal] Ib */
  oper1b = regs.byteregs[regal];
  oper2b = getmem8(segregs[regcs], ip);
  StepIP(1);
  op_sbb8();
  regs.byteregs[regal] = res8;
}

void opcode0x1D()
{ /* 1D SBB eAX Iv */
  oper1 = getreg16(regax);
  oper2 = getmem16(segregs[regcs], ip);
  StepIP(2);
  op_sbb16();
  putreg16(regax, res16);
}

void opcode0x1E()
{ /* 1E PUSH segregs[regds] */
  push(segregs[regds]);
}

void opcode0x1F()
{ /* 1F POP segregs[regds] */
  segregs[regds] = pop();
}

void opcode0x20()
{ /* 20 AND Eb Gb */
  modregrm();
  oper1b = readrm8(rm);
  oper2b = getreg8(reg);
  op_and8();
  writerm8(rm, res8);
}

void opcode0x21()
{ /* 21 AND Ev Gv */
  modregrm();
  oper1 = readrm16(rm);
  oper2 = getreg16(reg);
  op_and16();
  writerm16(rm, res16);
}

void opcode0x22()
{ /* 22 AND Gb Eb */
  modregrm();
  oper1b = getreg8(reg);
  oper2b = readrm8(rm);
  op_and8();
  putreg8(reg, res8);
}

void opcode0x23()
{ /* 23 AND Gv Ev */
  modregrm();
  oper1 = getreg16(reg);
  oper2 = readrm16(rm);
  op_and16();
  putreg16(reg, res16);
}

void opcode0x24()
{ /* 24 AND regs.byteregs[regal] Ib */
  oper1b = regs.byteregs[regal];
  oper2b = getmem8(segregs[regcs], ip);
  StepIP(1);
  op_and8();
  regs.byteregs[regal] = res8;
}

void opcode0x25()
{ /* 25 AND eAX Iv */
  oper1 = getreg16(regax);
  oper2 = getmem16(segregs[regcs], ip);
  StepIP(2);
  op_and16();
  putreg16(regax, res16);
}

void opcode0x27()
{ /* 27 DAA */
  if (((regs.byteregs[regal] & 0xF) > 9) || (af == 1))
  {
    oper1 = regs.byteregs[regal] + 6;
    regs.byteregs[regal] = oper1 & 255;
    cf = (oper1 & 0xFF00) ? 1 : 0;
    af = 1;
  }
  else
  {
    af = 0;
  }

  if (((regs.byteregs[regal] & 0xF0) > 0x90) || (cf == 1))
  {
    regs.byteregs[regal] = regs.byteregs[regal] + 0x60;
    cf = 1;
  }
  else
  {
    cf = 0;
  }

  regs.byteregs[regal] = regs.byteregs[regal] & 255;
  flag_szp8(regs.byteregs[regal]);
}

void opcode0x28()
{ /* 28 SUB Eb Gb */
  modregrm();
  oper1b = readrm8(rm);
  oper2b = getreg8(reg);
  op_sub8();
  writerm8(rm, res8);
}

void opcode0x29()
{ /* 29 SUB Ev Gv */
  modregrm();
  oper1 = readrm16(rm);
  oper2 = getreg16(reg);
  op_sub16();
  writerm16(rm, res16);
}

void opcode0x2A()
{ /* 2A SUB Gb Eb */
  modregrm();
  oper1b = getreg8(reg);
  oper2b = readrm8(rm);
  op_sub8();
  putreg8(reg, res8);
}

void opcode0x2B()
{ /* 2B SUB Gv Ev */
  modregrm();
  oper1 = getreg16(reg);
  oper2 = readrm16(rm);
  op_sub16();
  putreg16(reg, res16);
}

void opcode0x2C()
{ /* 2C SUB regs.byteregs[regal] Ib */
  oper1b = regs.byteregs[regal];
  oper2b = getmem8(segregs[regcs], ip);
  StepIP(1);
  op_sub8();
  regs.byteregs[regal] = res8;
}

void opcode0x2D()
{ /* 2D SUB eAX Iv */
  oper1 = getreg16(regax);
  oper2 = getmem16(segregs[regcs], ip);
  StepIP(2);
  op_sub16();
  putreg16(regax, res16);
}

void opcode0x2F()
{ // 2F DAS
  if (((regs.byteregs[regal] & 15) > 9) || (af == 1))
  {
    oper1 = regs.byteregs[regal] - 6;
    regs.byteregs[regal] = oper1 & 255;
    cf = (oper1 & 0xFF00) ? 1 : 0;
    af = 1;
  }
  else
  {
    af = 0;
  }

  if (((regs.byteregs[regal] & 0xF0) > 0x90) || (cf == 1))
  {
    regs.byteregs[regal] = regs.byteregs[regal] - 0x60;
    cf = 1;
  }
  else
  {
    cf = 0;
  }

  flag_szp8(regs.byteregs[regal]);
}

void opcode0x30()
{ /* 30 XOR Eb Gb */
  modregrm();
  oper1b = readrm8(rm);
  oper2b = getreg8(reg);
  op_xor8();
  writerm8(rm, res8);
}

void opcode0x31()
{ /* 31 XOR Ev Gv */
  modregrm();
  oper1 = readrm16(rm);
  oper2 = getreg16(reg);
  op_xor16();
  writerm16(rm, res16);
}

void opcode0x32()
{ /* 32 XOR Gb Eb */
  modregrm();
  oper1b = getreg8(reg);
  oper2b = readrm8(rm);
  op_xor8();
  putreg8(reg, res8);
}

void opcode0x33()
{ /* 33 XOR Gv Ev */
  modregrm();
  oper1 = getreg16(reg);
  oper2 = readrm16(rm);
  op_xor16();
  putreg16(reg, res16);
}

void opcode0x34()
{ /* 34 XOR regs.byteregs[regal] Ib */
  oper1b = regs.byteregs[regal];
  oper2b = getmem8(segregs[regcs], ip);
  StepIP(1);
  op_xor8();
  regs.byteregs[regal] = res8;
}

void opcode0x35()
{ /* 35 XOR eAX Iv */
  oper1 = getreg16(regax);
  oper2 = getmem16(segregs[regcs], ip);
  StepIP(2);
  op_xor16();
  putreg16(regax, res16);
}

void opcode0x37()
{ /* 37 AAA ASCII */
  if (((regs.byteregs[regal] & 0xF) > 9) || (af == 1))
  {
    regs.byteregs[regal] = regs.byteregs[regal] + 6;
    regs.byteregs[regah] = regs.byteregs[regah] + 1;
    af = 1;
    cf = 1;
  }
  else
  {
    af = 0;
    cf = 0;
  }

  regs.byteregs[regal] = regs.byteregs[regal] & 0xF;
}

void opcode0x38()
{ /* 38 CMP Eb Gb */
  modregrm();
  oper1b = readrm8(rm);
  oper2b = getreg8(reg);
  flag_sub8(oper1b, oper2b);
}

void opcode0x39()
{ /* 39 CMP Ev Gv */
  modregrm();
  oper1 = readrm16(rm);
  oper2 = getreg16(reg);
  flag_sub16(oper1, oper2);
}

void opcode0x3A()
{ /* 3A CMP Gb Eb */
  modregrm();
  oper1b = getreg8(reg);
  oper2b = readrm8(rm);
  flag_sub8(oper1b, oper2b);
}

void opcode0x3B()
{ /* 3B CMP Gv Ev */
  modregrm();
  oper1 = getreg16(reg);
  oper2 = readrm16(rm);
  flag_sub16(oper1, oper2);
}

void opcode0x3C()
{ /* 3C CMP regs.byteregs[regal] Ib */
  oper1b = regs.byteregs[regal];
  oper2b = getmem8(segregs[regcs], ip);
  StepIP(1);
  flag_sub8(oper1b, oper2b);
}

void opcode0x3D()
{ /* 3D CMP eAX Iv */
  oper1 = getreg16(regax);
  oper2 = getmem16(segregs[regcs], ip);
  StepIP(2);
  flag_sub16(oper1, oper2);
}

void opcode0x3F()
{ /* 3F AAS ASCII */
  if (((regs.byteregs[regal] & 0xF) > 9) || (af == 1))
  {
    regs.byteregs[regal] = regs.byteregs[regal] - 6;
    regs.byteregs[regah] = regs.byteregs[regah] - 1;
    af = 1;
    cf = 1;
  }
  else
  {
    af = 0;
    cf = 0;
  }

  regs.byteregs[regal] = regs.byteregs[regal] & 0xF;
}

void opcode0x40()
{ /* 40 INC eAX */
  oldcf = cf;
  oper1 = getreg16(regax);
  oper2 = 1;
  op_add16();
  cf = oldcf;
  putreg16(regax, res16);
}

void opcode0x41()
{ /* 41 INC eCX */
  oldcf = cf;
  oper1 = getreg16(regcx);
  oper2 = 1;
  op_add16();
  cf = oldcf;
  putreg16(regcx, res16);
}

void opcode0x42()
{ /* 42 INC eDX */
  oldcf = cf;
  oper1 = getreg16(regdx);
  oper2 = 1;
  op_add16();
  cf = oldcf;
  putreg16(regdx, res16);
}

void opcode0x43()
{ /* 43 INC eBX */
  oldcf = cf;
  oper1 = getreg16(regbx);
  oper2 = 1;
  op_add16();
  cf = oldcf;
  putreg16(regbx, res16);
}

void opcode0x44()
{ /* 44 INC eSP */
  oldcf = cf;
  oper1 = getreg16(regsp);
  oper2 = 1;
  op_add16();
  cf = oldcf;
  putreg16(regsp, res16);
}

void opcode0x45()
{ /* 45 INC eBP */
  oldcf = cf;
  oper1 = getreg16(regbp);
  oper2 = 1;
  op_add16();
  cf = oldcf;
  putreg16(regbp, res16);
}

void opcode0x46()
{ /* 46 INC eSI */
  oldcf = cf;
  oper1 = getreg16(regsi);
  oper2 = 1;
  op_add16();
  cf = oldcf;
  putreg16(regsi, res16);
}

void opcode0x47()
{ /* 47 INC eDI */
  oldcf = cf;
  oper1 = getreg16(regdi);
  oper2 = 1;
  op_add16();
  cf = oldcf;
  putreg16(regdi, res16);
}

void opcode0x48()
{ /* 48 DEC eAX */
  oldcf = cf;
  oper1 = getreg16(regax);
  oper2 = 1;
  op_sub16();
  cf = oldcf;
  putreg16(regax, res16);
}

void opcode0x49()
{ /* 49 DEC eCX */
  oldcf = cf;
  oper1 = getreg16(regcx);
  oper2 = 1;
  op_sub16();
  cf = oldcf;
  putreg16(regcx, res16);
}

void opcode0x4A()
{ /* 4A DEC eDX */
  oldcf = cf;
  oper1 = getreg16(regdx);
  oper2 = 1;
  op_sub16();
  cf = oldcf;
  putreg16(regdx, res16);
}

void opcode0x4B()
{ /* 4B DEC eBX */
  oldcf = cf;
  oper1 = getreg16(regbx);
  oper2 = 1;
  op_sub16();
  cf = oldcf;
  putreg16(regbx, res16);
}

void opcode0x4C()
{ /* 4C DEC eSP */
  oldcf = cf;
  oper1 = getreg16(regsp);
  oper2 = 1;
  op_sub16();
  cf = oldcf;
  putreg16(regsp, res16);
}

void opcode0x4D()
{ /* 4D DEC eBP */
  oldcf = cf;
  oper1 = getreg16(regbp);
  oper2 = 1;
  op_sub16();
  cf = oldcf;
  putreg16(regbp, res16);
}

void opcode0x4E()
{ /* 4E DEC eSI */
  oldcf = cf;
  oper1 = getreg16(regsi);
  oper2 = 1;
  op_sub16();
  cf = oldcf;
  putreg16(regsi, res16);
}

void opcode0x4F()
{ /* 4F DEC eDI */
  oldcf = cf;
  oper1 = getreg16(regdi);
  oper2 = 1;
  op_sub16();
  cf = oldcf;
  putreg16(regdi, res16);
}

void opcode0x50()
{ /* 50 PUSH eAX */
  push(getreg16(regax));
}

void opcode0x51()
{ /* 51 PUSH eCX */
  push(getreg16(regcx));
}

void opcode0x52()
{ /* 52 PUSH eDX */
  push(getreg16(regdx));
}

void opcode0x53()
{ /* 53 PUSH eBX */
  push(getreg16(regbx));
}

void opcode0x54()
{ /* 54 PUSH eSP */
  push(getreg16(regsp) - 2);
}

void opcode0x55()
{ /* 55 PUSH eBP */
  push(getreg16(regbp));
}

void opcode0x56()
{ /* 56 PUSH eSI */
  push(getreg16(regsi));
}

void opcode0x57()
{ /* 57 PUSH eDI */
  push(getreg16(regdi));
}

void opcode0x58()
{ /* 58 POP eAX */
  putreg16(regax, pop());
}

void opcode0x59()
{ /* 59 POP eCX */
  putreg16(regcx, pop());
}

void opcode0x5A()
{ /* 5A POP eDX */
  putreg16(regdx, pop());
}

void opcode0x5B()
{ /* 5B POP eBX */
  putreg16(regbx, pop());
}

void opcode0x5C()
{ /* 5C POP eSP */
  putreg16(regsp, pop());
}

void opcode0x5D()
{ /* 5D POP eBP */
  putreg16(regbp, pop());
}

void opcode0x5E()
{ /* 5E POP eSI */
  putreg16(regsi, pop());
}

void opcode0x5F()
{ /* 5F POP eDI */
  putreg16(regdi, pop());
}

#ifdef CPU_V20
void opcode0x60()
{ /* 60 PUSHA (80186+) */
  oldsp = getreg16(regsp);
  push(getreg16(regax));
  push(getreg16(regcx));
  push(getreg16(regdx));
  push(getreg16(regbx));
  push(oldsp);
  push(getreg16(regbp));
  push(getreg16(regsi));
  push(getreg16(regdi));
}

void opcode0x61()
{ /* 61 POPA (80186+) */
  putreg16(regdi, pop());
  putreg16(regsi, pop());
  putreg16(regbp, pop());
  dummy = pop();
  putreg16(regbx, pop());
  putreg16(regdx, pop());
  putreg16(regcx, pop());
  putreg16(regax, pop());
}

void opcode0x62()
{ /* 62 BOUND Gv, Ev (80186+) */
  modregrm();
  getea(rm);
  if (signext32(getreg16(reg)) < signext32(getmem16(ea >> 4, ea & 15)))
  {
    intcall86(5); // bounds check exception
  }
  else
  {
    ea += 2;
    if (signext32(getreg16(reg)) > signext32(getmem16(ea >> 4, ea & 15)))
    {
      intcall86(5); // bounds check exception
    }
  }
}

void opcode0x68()
{ /* 68 PUSH Iv (80186+) */
  push(getmem16(segregs[regcs], ip));
  StepIP(2);
}

void opcode0x69()
{ /* 69 IMUL Gv Ev Iv (80186+) */
  modregrm();
  temp1 = readrm16(rm);
  temp2 = getmem16(segregs[regcs], ip);
  StepIP(2);
  if ((temp1 & 0x8000L) == 0x8000L)
  {
    temp1 = temp1 | 0xFFFF0000L;
  }

  if ((temp2 & 0x8000L) == 0x8000L)
  {
    temp2 = temp2 | 0xFFFF0000L;
  }

  temp3 = temp1 * temp2;
  putreg16(reg, temp3 & 0xFFFFL);
  if (temp3 & 0xFFFF0000L)
  {
    cf = 1;
    of = 1;
  }
  else
  {
    cf = 0;
    of = 0;
  }
}

void opcode0x6A()
{ /* 6A PUSH Ib (80186+) */
  push(getmem8(segregs[regcs], ip));
  StepIP(1);
}

void opcode0x6B()
{ /* 6B IMUL Gv Eb Ib (80186+) */
  modregrm();
  temp1 = readrm16(rm);
  temp2 = signext(getmem8(segregs[regcs], ip));
  StepIP(1);
  if ((temp1 & 0x8000L) == 0x8000L)
  {
    temp1 = temp1 | 0xFFFF0000L;
  }

  if ((temp2 & 0x8000L) == 0x8000L)
  {
    temp2 = temp2 | 0xFFFF0000L;
  }

  temp3 = temp1 * temp2;
  putreg16(reg, temp3 & 0xFFFFL);
  if (temp3 & 0xFFFF0000L)
  {
    cf = 1;
    of = 1;
  }
  else
  {
    cf = 0;
    of = 0;
  }
}

void opcode0x6C()
{ /* 6E INSB */
  if (reptype && (getreg16(regcx) == 0))
  {
    return;
  }

  putmem8(useseg, getreg16(regsi), ports.read(regs.wordregs[regdx]));
  if (df)
  {
    putreg16(regsi, getreg16(regsi) - 1);
    putreg16(regdi, getreg16(regdi) - 1);
  }
  else
  {
    putreg16(regsi, getreg16(regsi) + 1);
    putreg16(regdi, getreg16(regdi) + 1);
  }

  if (reptype)
  {
    putreg16(regcx, getreg16(regcx) - 1);
  }

  totalexec++;
  loopcount++;
  if (!reptype)
  {
    return;
  }

  ip = firstip;
}

void opcode0x6D()
{ /* 6F INSW */
  if (reptype && (getreg16(regcx) == 0))
  {
    return;
  }

  putmem16(useseg, getreg16(regsi), ports.read16(regs.wordregs[regdx]));
  if (df)
  {
    putreg16(regsi, getreg16(regsi) - 2);
    putreg16(regdi, getreg16(regdi) - 2);
  }
  else
  {
    putreg16(regsi, getreg16(regsi) + 2);
    putreg16(regdi, getreg16(regdi) + 2);
  }

  if (reptype)
  {
    putreg16(regcx, getreg16(regcx) - 1);
  }

  totalexec++;
  loopcount++;
  if (!reptype)
  {
    return;
  }

  ip = firstip;
}

void opcode0x6E()
{ /* 6E OUTSB */
  if (reptype && (getreg16(regcx) == 0))
  {
    return;
  }

  // portWrite (regs.wordregs[regdx], getmem8 (useseg, getreg16 (regsi) ) );
  ports.write(regs.wordregs[regdx], getmem8(useseg, getreg16(regsi)));
  if (df)
  {
    putreg16(regsi, getreg16(regsi) - 1);
    putreg16(regdi, getreg16(regdi) - 1);
  }
  else
  {
    putreg16(regsi, getreg16(regsi) + 1);
    putreg16(regdi, getreg16(regdi) + 1);
  }

  if (reptype)
  {
    putreg16(regcx, getreg16(regcx) - 1);
  }

  totalexec++;
  loopcount++;
  if (!reptype)
  {
    return;
  }

  ip = firstip;
}

void opcode0x6F()
{ /* 6F OUTSW */
  if (reptype && (getreg16(regcx) == 0))
  {
    return;
  }

  ports.write16(regs.wordregs[regdx], getmem16(useseg, getreg16(regsi)));
  if (df)
  {
    putreg16(regsi, getreg16(regsi) - 2);
    putreg16(regdi, getreg16(regdi) - 2);
  }
  else
  {
    putreg16(regsi, getreg16(regsi) + 2);
    putreg16(regdi, getreg16(regdi) + 2);
  }

  if (reptype)
  {
    putreg16(regcx, getreg16(regcx) - 1);
  }

  totalexec++;
  loopcount++;
  if (!reptype)
  {
    return;
  }

  ip = firstip;
}
#endif

void opcode0x70()
{ /* 70 JO Jb */
  temp16 = signext(getmem8(segregs[regcs], ip));
  StepIP(1);
  if (of)
  {
    ip = ip + temp16;
  }
}

void opcode0x71()
{ /* 71 JNO Jb */
  temp16 = signext(getmem8(segregs[regcs], ip));
  StepIP(1);
  if (!of)
  {
    ip = ip + temp16;
  }
}

void opcode0x72()
{ /* 72 JB Jb */
  temp16 = signext(getmem8(segregs[regcs], ip));
  StepIP(1);
  if (cf)
  {
    ip = ip + temp16;
  }
}

void opcode0x73()
{ /* 73 JNB Jb */
  temp16 = signext(getmem8(segregs[regcs], ip));
  StepIP(1);
  if (!cf)
  {
    ip = ip + temp16;
  }
}

void opcode0x74()
{ /* 74 JZ Jb */
  temp16 = signext(getmem8(segregs[regcs], ip));
  StepIP(1);
  if (zf)
  {
    ip = ip + temp16;
  }
}

void opcode0x75()
{ /* 75 JNZ Jb */
  temp16 = signext(getmem8(segregs[regcs], ip));
  StepIP(1);
  if (!zf)
  {
    ip = ip + temp16;
  }
}

void opcode0x76()
{ /* 76 JBE Jb */
  temp16 = signext(getmem8(segregs[regcs], ip));
  StepIP(1);
  if (cf || zf)
  {
    ip = ip + temp16;
  }
}

void opcode0x77()
{ /* 77 JA Jb */
  temp16 = signext(getmem8(segregs[regcs], ip));
  StepIP(1);
  if (!cf && !zf)
  {
    ip = ip + temp16;
  }
}

void opcode0x78()
{ /* 78 JS Jb */
  temp16 = signext(getmem8(segregs[regcs], ip));
  StepIP(1);
  if (sf)
  {
    ip = ip + temp16;
  }
}

void opcode0x79()
{ /* 79 JNS Jb */
  temp16 = signext(getmem8(segregs[regcs], ip));
  StepIP(1);
  if (!sf)
  {
    ip = ip + temp16;
  }
}

void opcode0x7A()
{ /* 7A JPE Jb */
  temp16 = signext(getmem8(segregs[regcs], ip));
  StepIP(1);
  if (pf)
  {
    ip = ip + temp16;
  }
}

void opcode0x7B()
{ /* 7B JPO Jb */
  temp16 = signext(getmem8(segregs[regcs], ip));
  StepIP(1);
  if (!pf)
  {
    ip = ip + temp16;
  }
}

void opcode0x7C()
{ /* 7C JL Jb */
  temp16 = signext(getmem8(segregs[regcs], ip));
  StepIP(1);
  if (sf != of)
  {
    ip = ip + temp16;
  }
}

void opcode0x7D()
{ /* 7D JGE Jb */
  temp16 = signext(getmem8(segregs[regcs], ip));
  StepIP(1);
  if (sf == of)
  {
    ip = ip + temp16;
  }
}

void opcode0x7E()
{ /* 7E JLE Jb */
  temp16 = signext(getmem8(segregs[regcs], ip));
  StepIP(1);
  if ((sf != of) || zf)
  {
    ip = ip + temp16;
  }
}

void opcode0x7F()
{ /* 7F JG Jb */
  temp16 = signext(getmem8(segregs[regcs], ip));
  StepIP(1);
  if (!zf && (sf == of))
  {
    ip = ip + temp16;
  }
}

void opcode0x80()
{ /* 80/82 GRP1 Eb Ib */
  modregrm();
  oper1b = readrm8(rm);
  oper2b = getmem8(segregs[regcs], ip);
  StepIP(1);
  switch (reg)
  {
  case 0:
    op_add8();
    break;
  case 1:
    op_or8();
    break;
  case 2:
    op_adc8();
    break;
  case 3:
    op_sbb8();
    break;
  case 4:
    op_and8();
    break;
  case 5:
    op_sub8();
    break;
  case 6:
    op_xor8();
    break;
  case 7:
    flag_sub8(oper1b, oper2b);
    break;
  default:
    break; /* to avoid compiler warnings */
  }

  if (reg < 7)
  {
    writerm8(rm, res8);
  }
}

void opcode0x82()
{
  opcode0x80();
}

void opcode0x81()
{ /* 81 GRP1 Ev Iv */
  modregrm();
  oper1 = readrm16(rm);
  if (opcode == 0x81)
  {
    oper2 = getmem16(segregs[regcs], ip);
    StepIP(2);
  }
  else
  {
    oper2 = signext(getmem8(segregs[regcs], ip));
    StepIP(1);
  }

  switch (reg)
  {
  case 0:
    op_add16();
    break;
  case 1:
    op_or16();
    break;
  case 2:
    op_adc16();
    break;
  case 3:
    op_sbb16();
    break;
  case 4:
    op_and16();
    break;
  case 5:
    op_sub16();
    break;
  case 6:
    op_xor16();
    break;
  case 7:
    flag_sub16(oper1, oper2);
    break;
  default:
    break; /* to avoid compiler warnings */
  }

  if (reg < 7)
  {
    writerm16(rm, res16);
  }
}

void opcode0x83()
{ /* 83 GRP1 Ev Ib */
  opcode0x81();
}

void opcode0x84()
{ /* 84 TEST Gb Eb */
  modregrm();
  oper1b = getreg8(reg);
  oper2b = readrm8(rm);
  flag_log8(oper1b & oper2b);
}

void opcode0x85()
{ /* 85 TEST Gv Ev */
  modregrm();
  oper1 = getreg16(reg);
  oper2 = readrm16(rm);
  flag_log16(oper1 & oper2);
}

void opcode0x86()
{ /* 86 XCHG Gb Eb */
  modregrm();
  oper1b = getreg8(reg);
  putreg8(reg, readrm8(rm));
  writerm8(rm, oper1b);
}

void opcode0x87()
{ /* 87 XCHG Gv Ev */
  modregrm();
  oper1 = getreg16(reg);
  putreg16(reg, readrm16(rm));
  writerm16(rm, oper1);
}

void opcode0x88()
{ /* 88 MOV Eb Gb */
  modregrm();
  writerm8(rm, getreg8(reg));
}

void opcode0x89()
{ /* 89 MOV Ev Gv */
  modregrm();
  writerm16(rm, getreg16(reg));
}

void opcode0x8A()
{ /* 8A MOV Gb Eb */
  modregrm();
  putreg8(reg, readrm8(rm));
}

void opcode0x8B()
{ /* 8B MOV Gv Ev */
  modregrm();
  putreg16(reg, readrm16(rm));
}

void opcode0x8C()
{ /* 8C MOV Ew Sw */
  modregrm();
  writerm16(rm, getsegreg(reg));
}

void opcode0x8D()
{ /* 8D LEA Gv M */
  modregrm();
  getea(rm);
  putreg16(reg, ea - segbase(useseg));
}

void opcode0x8E()
{ /* 8E MOV Sw Ew */
  modregrm();
  putsegreg(reg, readrm16(rm));
}

void opcode0x8F()
{ /* 8F POP Ev */
  modregrm();
  writerm16(rm, pop());
}

void opcode0x90()
{ /* 90 NOP */
}

void opcode0x91()
{ /* 91 XCHG eCX eAX */
  oper1 = getreg16(regcx);
  putreg16(regcx, getreg16(regax));
  putreg16(regax, oper1);
}

void opcode0x92()
{ /* 92 XCHG eDX eAX */
  oper1 = getreg16(regdx);
  putreg16(regdx, getreg16(regax));
  putreg16(regax, oper1);
}

void opcode0x93()
{ /* 93 XCHG eBX eAX */
  oper1 = getreg16(regbx);
  putreg16(regbx, getreg16(regax));
  putreg16(regax, oper1);
}

void opcode0x94()
{ /* 94 XCHG eSP eAX */
  oper1 = getreg16(regsp);
  putreg16(regsp, getreg16(regax));
  putreg16(regax, oper1);
}

void opcode0x95()
{ /* 95 XCHG eBP eAX */
  oper1 = getreg16(regbp);
  putreg16(regbp, getreg16(regax));
  putreg16(regax, oper1);
}

void opcode0x96()
{ /* 96 XCHG eSI eAX */
  oper1 = getreg16(regsi);
  putreg16(regsi, getreg16(regax));
  putreg16(regax, oper1);
}

void opcode0x97()
{ /* 97 XCHG eDI eAX */
  oper1 = getreg16(regdi);
  putreg16(regdi, getreg16(regax));
  putreg16(regax, oper1);
}

void opcode0x98()
{ // 98 CBW
  regs.byteregs[regah] = ((regs.byteregs[regal] & 0x80) == 0x80) ? 0xFF : 0;
}

void opcode0x99()
{ // 99 CWD
  regs.wordregs[regdx] = ((regs.byteregs[regah] & 0x80) == 0x80) ? 0xFFFF : 0;
}

void opcode0x9A()
{ /* 9A CALL Ap */
  oper1 = getmem16(segregs[regcs], ip);
  StepIP(2);
  oper2 = getmem16(segregs[regcs], ip);
  StepIP(2);
  push(segregs[regcs]);
  push(ip);
  ip = oper1;
  segregs[regcs] = oper2;
}

void opcode0x9B()
{ /* 9B WAIT */
}

void opcode0x9C()
{ /* 9C PUSHF */
  push(makeflagsword() | 0xF800);
}

void opcode0x9D()
{ /* 9D POPF */
  temp16 = pop();
  decodeflagsword(temp16);
}

void opcode0x9E()
{ /* 9E SAHF */
  decodeflagsword((makeflagsword() & 0xFF00) | regs.byteregs[regah]);
}

void opcode0x9F()
{ /* 9F LAHF */
  regs.byteregs[regah] = makeflagsword() & 0xFF;
}

void opcode0xA0()
{ /* A0 MOV regs.byteregs[regal] Ob */
  regs.byteregs[regal] = getmem8(useseg, getmem16(segregs[regcs], ip));
  StepIP(2);
}

void opcode0xA1()
{ /* A1 MOV eAX Ov */
  oper1 = getmem16(useseg, getmem16(segregs[regcs], ip));
  StepIP(2);
  putreg16(regax, oper1);
}

void opcode0xA2()
{ /* A2 MOV Ob regs.byteregs[regal] */
  putmem8(useseg, getmem16(segregs[regcs], ip), regs.byteregs[regal]);
  StepIP(2);
}

void opcode0xA3()
{ /* A3 MOV Ov eAX */
  putmem16(useseg, getmem16(segregs[regcs], ip), getreg16(regax));
  StepIP(2);
}

void opcode0xA4()
{ /* A4 MOVSB */
  if (reptype && (getreg16(regcx) == 0))
  {
    return;
  }

  putmem8(segregs[reges], getreg16(regdi), getmem8(useseg, getreg16(regsi)));
  if (df)
  {
    putreg16(regsi, getreg16(regsi) - 1);
    putreg16(regdi, getreg16(regdi) - 1);
  }
  else
  {
    putreg16(regsi, getreg16(regsi) + 1);
    putreg16(regdi, getreg16(regdi) + 1);
  }

  if (reptype)
  {
    putreg16(regcx, getreg16(regcx) - 1);
  }

  totalexec++;
  loopcount++;
  if (!reptype)
  {
    return;
  }

  ip = firstip;
}

void opcode0xA5()
{ /* A5 MOVSW */
  if (reptype && (getreg16(regcx) == 0))
  {
    return;
  }

  putmem16(segregs[reges], getreg16(regdi), getmem16(useseg, getreg16(regsi)));
  if (df)
  {
    putreg16(regsi, getreg16(regsi) - 2);
    putreg16(regdi, getreg16(regdi) - 2);
  }
  else
  {
    putreg16(regsi, getreg16(regsi) + 2);
    putreg16(regdi, getreg16(regdi) + 2);
  }

  if (reptype)
  {
    putreg16(regcx, getreg16(regcx) - 1);
  }

  totalexec++;
  loopcount++;
  if (!reptype)
  {
    return;
  }

  ip = firstip;
}

void opcode0xA6()
{ /* A6 CMPSB */
  if (reptype && (getreg16(regcx) == 0))
  {
    return;
  }

  oper1b = getmem8(useseg, getreg16(regsi));
  oper2b = getmem8(segregs[reges], getreg16(regdi));
  if (df)
  {
    putreg16(regsi, getreg16(regsi) - 1);
    putreg16(regdi, getreg16(regdi) - 1);
  }
  else
  {
    putreg16(regsi, getreg16(regsi) + 1);
    putreg16(regdi, getreg16(regdi) + 1);
  }

  flag_sub8(oper1b, oper2b);
  if (reptype)
  {
    putreg16(regcx, getreg16(regcx) - 1);
  }

  if ((reptype == 1) && !zf)
  {
    return;
  }
  else if ((reptype == 2) && (zf == 1))
  {
    return;
  }

  totalexec++;
  loopcount++;
  if (!reptype)
  {
    return;
  }

  ip = firstip;
}

void opcode0xA7()
{ /* A7 CMPSW */
  if (reptype && (getreg16(regcx) == 0))
  {
    return;
  }

  oper1 = getmem16(useseg, getreg16(regsi));
  oper2 = getmem16(segregs[reges], getreg16(regdi));
  if (df)
  {
    putreg16(regsi, getreg16(regsi) - 2);
    putreg16(regdi, getreg16(regdi) - 2);
  }
  else
  {
    putreg16(regsi, getreg16(regsi) + 2);
    putreg16(regdi, getreg16(regdi) + 2);
  }

  flag_sub16(oper1, oper2);
  if (reptype)
  {
    putreg16(regcx, getreg16(regcx) - 1);
  }

  if ((reptype == 1) && !zf)
  {
    return;
  }

  if ((reptype == 2) && (zf == 1))
  {
    return;
  }

  totalexec++;
  loopcount++;
  if (!reptype)
  {
    return;
  }

  ip = firstip;
}

void opcode0xA8()
{ /* A8 TEST regs.byteregs[regal] Ib */
  oper1b = regs.byteregs[regal];
  oper2b = getmem8(segregs[regcs], ip);
  StepIP(1);
  flag_log8(oper1b & oper2b);
}

void opcode0xA9()
{ /* A9 TEST eAX Iv */
  oper1 = getreg16(regax);
  oper2 = getmem16(segregs[regcs], ip);
  StepIP(2);
  flag_log16(oper1 & oper2);
}

void opcode0xAA()
{ // AA STOSB
  if (reptype && (getreg16(regcx) == 0))
  {
    return;
  }

  putmem8(segregs[reges], getreg16(regdi), regs.byteregs[regal]);
  if (df)
  {
    putreg16(regdi, getreg16(regdi) - 1);
  }
  else
  {
    putreg16(regdi, getreg16(regdi) + 1);
  }

  if (reptype)
  {
    putreg16(regcx, getreg16(regcx) - 1);
  }

  totalexec++;
  loopcount++;
  if (!reptype)
  {
    return;
  }

  ip = firstip;
}

void opcode0xAB()
{ /* AB STOSW */
  if (reptype && (getreg16(regcx) == 0))
  {
    return;
  }

  putmem16(segregs[reges], getreg16(regdi), getreg16(regax));
  if (df)
  {
    putreg16(regdi, getreg16(regdi) - 2);
  }
  else
  {
    putreg16(regdi, getreg16(regdi) + 2);
  }

  if (reptype)
  {
    putreg16(regcx, getreg16(regcx) - 1);
  }

  totalexec++;
  loopcount++;
  if (!reptype)
  {
    return;
  }

  ip = firstip;
}

void opcode0xAC()
{ /* AC LODSB */
  if (reptype && (getreg16(regcx) == 0))
  {
    return;
  }

  regs.byteregs[regal] = getmem8(useseg, getreg16(regsi));
  if (df)
  {
    putreg16(regsi, getreg16(regsi) - 1);
  }
  else
  {
    putreg16(regsi, getreg16(regsi) + 1);
  }

  if (reptype)
  {
    putreg16(regcx, getreg16(regcx) - 1);
  }

  totalexec++;
  loopcount++;
  if (!reptype)
  {
    return;
  }

  ip = firstip;
}

void opcode0xAD()
{ /* AD LODSW */
  if (reptype && (getreg16(regcx) == 0))
  {
    return;
  }

  oper1 = getmem16(useseg, getreg16(regsi));
  putreg16(regax, oper1);
  if (df)
  {
    putreg16(regsi, getreg16(regsi) - 2);
  }
  else
  {
    putreg16(regsi, getreg16(regsi) + 2);
  }

  if (reptype)
  {
    putreg16(regcx, getreg16(regcx) - 1);
  }

  totalexec++;
  loopcount++;
  if (!reptype)
  {
    return;
  }

  ip = firstip;
}

void opcode0xAE()
{ /* AE SCASB */
  if (reptype && (getreg16(regcx) == 0))
  {
    return;
  }

  oper1b = getmem8(segregs[reges], getreg16(regdi));
  oper2b = regs.byteregs[regal];
  flag_sub8(oper1b, oper2b);
  if (df)
  {
    putreg16(regdi, getreg16(regdi) - 1);
  }
  else
  {
    putreg16(regdi, getreg16(regdi) + 1);
  }

  if (reptype)
  {
    putreg16(regcx, getreg16(regcx) - 1);
  }

  if ((reptype == 1) && !zf)
  {
    return;
  }
  else if ((reptype == 2) && (zf == 1))
  {
    return;
  }

  totalexec++;
  loopcount++;
  if (!reptype)
  {
    return;
  }

  ip = firstip;
}

void opcode0xAF()
{ /* AF SCASW */
  if (reptype && (getreg16(regcx) == 0))
  {
    return;
  }

  oper1 = getmem16(segregs[reges], getreg16(regdi));
  oper2 = getreg16(regax);
  flag_sub16(oper1, oper2);
  if (df)
  {
    putreg16(regdi, getreg16(regdi) - 2);
  }
  else
  {
    putreg16(regdi, getreg16(regdi) + 2);
  }

  if (reptype)
  {
    putreg16(regcx, getreg16(regcx) - 1);
  }

  if ((reptype == 1) && !zf)
  {
    return;
  }
  else if ((reptype == 2) & (zf == 1))
  {
    return;
  }

  totalexec++;
  loopcount++;
  if (!reptype)
  {
    return;
  }

  ip = firstip;
}

void opcode0xB0()
{ /* B0 MOV regs.byteregs[regal] Ib */
  regs.byteregs[regal] = getmem8(segregs[regcs], ip);
  StepIP(1);
}

void opcode0xB1()
{ /* B1 MOV regs.byteregs[regcl] Ib */
  regs.byteregs[regcl] = getmem8(segregs[regcs], ip);
  StepIP(1);
}

void opcode0xB2()
{ /* B2 MOV regs.byteregs[regdl] Ib */
  regs.byteregs[regdl] = getmem8(segregs[regcs], ip);
  StepIP(1);
}

void opcode0xB3()
{ /* B3 MOV regs.byteregs[regbl] Ib */
  regs.byteregs[regbl] = getmem8(segregs[regcs], ip);
  StepIP(1);
}

void opcode0xB4()
{ /* B4 MOV regs.byteregs[regah] Ib */
  regs.byteregs[regah] = getmem8(segregs[regcs], ip);
  StepIP(1);
}

void opcode0xB5()
{ /* B5 MOV regs.byteregs[regch] Ib */
  regs.byteregs[regch] = getmem8(segregs[regcs], ip);
  StepIP(1);
}

void opcode0xB6()
{ /* B6 MOV regs.byteregs[regdh] Ib */
  regs.byteregs[regdh] = getmem8(segregs[regcs], ip);
  StepIP(1);
}

void opcode0xB7()
{ /* B7 MOV regs.byteregs[regbh] Ib */
  regs.byteregs[regbh] = getmem8(segregs[regcs], ip);
  StepIP(1);
}

void opcode0xB8()
{ /* B8 MOV eAX Iv */
  oper1 = getmem16(segregs[regcs], ip);
  StepIP(2);
  putreg16(regax, oper1);
}

void opcode0xB9()
{ /* B9 MOV eCX Iv */
  oper1 = getmem16(segregs[regcs], ip);
  StepIP(2);
  putreg16(regcx, oper1);
}

void opcode0xBA()
{ /* BA MOV eDX Iv */
  oper1 = getmem16(segregs[regcs], ip);
  StepIP(2);
  putreg16(regdx, oper1);
}

void opcode0xBB()
{ /* BB MOV eBX Iv */
  oper1 = getmem16(segregs[regcs], ip);
  StepIP(2);
  putreg16(regbx, oper1);
}

void opcode0xBC()
{ /* BC MOV eSP Iv */
  putreg16(regsp, getmem16(segregs[regcs], ip));
  StepIP(2);
}

void opcode0xBD()
{ /* BD MOV eBP Iv */
  putreg16(regbp, getmem16(segregs[regcs], ip));
  StepIP(2);
}

void opcode0xBE()
{ /* BE MOV eSI Iv */
  putreg16(regsi, getmem16(segregs[regcs], ip));
  StepIP(2);
}

void opcode0xBF()
{ /* BF MOV eDI Iv */
  putreg16(regdi, getmem16(segregs[regcs], ip));
  StepIP(2);
}

void opcode0xC0()
{ /* C0 GRP2 byte imm8 (80186+) */
  modregrm();
  oper1b = readrm8(rm);
  oper2b = getmem8(segregs[regcs], ip);
  StepIP(1);
  writerm8(rm, op_grp2_8(oper2b));
}

void opcode0xC1()
{ /* C1 GRP2 word imm8 (80186+) */
  modregrm();
  oper1 = readrm16(rm);
  oper2 = getmem8(segregs[regcs], ip);
  StepIP(1);
  writerm16(rm, op_grp2_16((uint8_t)oper2));
}

void opcode0xC2()
{ /* C2 RET Iw */
  oper1 = getmem16(segregs[regcs], ip);
  ip = pop();
  putreg16(regsp, getreg16(regsp) + oper1);
}

void opcode0xC3()
{ /* C3 RET */
  ip = pop();
}

void opcode0xC4()
{ /* C4 LES Gv Mp */
  modregrm();
  getea(rm);
  putreg16(reg, read86(ea) + read86(ea + 1) * 256);
  segregs[reges] = read86(ea + 2) + read86(ea + 3) * 256;
}

void opcode0xC5()
{ /* C5 LDS Gv Mp */
  modregrm();
  getea(rm);
  putreg16(reg, read86(ea) + read86(ea + 1) * 256);
  segregs[regds] = read86(ea + 2) + read86(ea + 3) * 256;
}

void opcode0xC6()
{ /* C6 MOV Eb Ib */
  modregrm();
  writerm8(rm, getmem8(segregs[regcs], ip));
  StepIP(1);
}

void opcode0xC7()
{ /* C7 MOV Ev Iv */
  modregrm();
  writerm16(rm, getmem16(segregs[regcs], ip));
  StepIP(2);
}

void opcode0xC8()
{ /* C8 ENTER (80186+) */
  stacksize = getmem16(segregs[regcs], ip);
  StepIP(2);
  nestlev = getmem8(segregs[regcs], ip);
  StepIP(1);
  push(getreg16(regbp));
  frametemp = getreg16(regsp);
  if (nestlev)
  {
    for (temp16 = 1; temp16 < nestlev; temp16++)
    {
      putreg16(regbp, getreg16(regbp) - 2);
      push(getreg16(regbp));
    }

    push(getreg16(regsp));
  }

  putreg16(regbp, frametemp);
  putreg16(regsp, getreg16(regbp) - stacksize);
}

void opcode0xC9()
{ /* C9 LEAVE (80186+) */
  putreg16(regsp, getreg16(regbp));
  putreg16(regbp, pop());
}

void opcode0xCA()
{ /* CA RETF Iw */
  oper1 = getmem16(segregs[regcs], ip);
  ip = pop();
  segregs[regcs] = pop();
  putreg16(regsp, getreg16(regsp) + oper1);
}

void opcode0xCB()
{ /* CB RETF */
  ip = pop();
  ;
  segregs[regcs] = pop();
}

void opcode0xCC()
{ /* CC INT 3 */
  intcall86(3);
}

void opcode0xCD()
{ /* CD INT Ib */
  oper1b = getmem8(segregs[regcs], ip);
  StepIP(1);
  intcall86(oper1b);
}

void opcode0xCE()
{ /* CE INTO */
  if (of)
  {
    intcall86(4);
  }
}

void opcode0xCF()
{ /* CF IRET */
  ip = pop();
  segregs[regcs] = pop();
  decodeflagsword(pop());

  /*
   * if (net.enabled) net.canrecv = 1;
   */
}

void opcode0xD0()
{ /* D0 GRP2 Eb 1 */
  modregrm();
  oper1b = readrm8(rm);
  writerm8(rm, op_grp2_8(1));
}

void opcode0xD1()
{ /* D1 GRP2 Ev 1 */
  modregrm();
  oper1 = readrm16(rm);
  writerm16(rm, op_grp2_16(1));
}

void opcode0xD2()
{ /* D2 GRP2 Eb regs.byteregs[regcl] */
  modregrm();
  oper1b = readrm8(rm);
  writerm8(rm, op_grp2_8(regs.byteregs[regcl]));
}

void opcode0xD3()
{ /* D3 GRP2 Ev regs.byteregs[regcl] */
  modregrm();
  oper1 = readrm16(rm);
  writerm16(rm, op_grp2_16(regs.byteregs[regcl]));
}

void opcode0xD4()
{ /* D4 AAM I0 */
  oper1 = getmem8(segregs[regcs], ip);
  StepIP(1);
  if (!oper1)
  {
    intcall86(0);
    return;
  } /* division by zero */

  regs.byteregs[regah] = (regs.byteregs[regal] / oper1) & 255;
  regs.byteregs[regal] = (regs.byteregs[regal] % oper1) & 255;
  flag_szp16(getreg16(regax));
}

void opcode0xD5()
{ /* D5 AAD I0 */
  oper1 = getmem8(segregs[regcs], ip);
  StepIP(1);
  regs.byteregs[regal] = (regs.byteregs[regah] * oper1 + regs.byteregs[regal]) & 255;
  regs.byteregs[regah] = 0;
  flag_szp16(regs.byteregs[regah] * oper1 + regs.byteregs[regal]);
  sf = 0;
}

void opcode0xD6()
{ /* D6 XLAT on V20/V30, SALC on 8086/8088 */
#ifndef CPU_V20
  regs.byteregs[regal] = cf ? 0xFF : 0x00;
#else
  regs.byteregs[regal] = read86(useseg * 16 + (regs.wordregs[regbx]) + regs.byteregs[regal]);
#endif
}

void opcode0xD7()
{ /* D7 XLAT */
  regs.byteregs[regal] = read86(useseg * 16 + (regs.wordregs[regbx]) + regs.byteregs[regal]);
}

void opcodeSkip()
{
  modregrm();
}

void opcode0xE0()
{ /* E0 LOOPNZ Jb */
  temp16 = signext(getmem8(segregs[regcs], ip));
  StepIP(1);
  putreg16(regcx, getreg16(regcx) - 1);
  if ((getreg16(regcx)) && !zf)
  {
    ip = ip + temp16;
  }
}

void opcode0xE1()
{ /* E1 LOOPZ Jb */
  temp16 = signext(getmem8(segregs[regcs], ip));
  StepIP(1);
  putreg16(regcx, (getreg16(regcx)) - 1);
  if ((getreg16(regcx)) && (zf == 1))
  {
    ip = ip + temp16;
  }
}

void opcode0xE2()
{ /* E2 LOOP Jb */
  temp16 = signext(getmem8(segregs[regcs], ip));
  StepIP(1);
  putreg16(regcx, (getreg16(regcx)) - 1);
  if (getreg16(regcx))
  {
    ip = ip + temp16;
  }
}

void opcode0xE3()
{ /* E3 JCXZ Jb */
  temp16 = signext(getmem8(segregs[regcs], ip));
  StepIP(1);
  if (!(getreg16(regcx)))
  {
    ip = ip + temp16;
  }
}

void opcode0xE4()
{ /* E4 IN regs.byteregs[regal] Ib */
  oper1b = getmem8(segregs[regcs], ip);
  StepIP(1);
  regs.byteregs[regal] = (uint8_t)ports.read(oper1b);
}

void opcode0xE5()
{ /* E5 IN eAX Ib */
  oper1b = getmem8(segregs[regcs], ip);
  StepIP(1);
  putreg16(regax, ports.read16(oper1b));
}

void opcode0xE6()
{ /* E6 OUT Ib regs.byteregs[regal] */
  oper1b = getmem8(segregs[regcs], ip);
  StepIP(1);
  // portWrite (oper1b, regs.byteregs[regal]);
  ports.write(oper1b, regs.byteregs[regal]);
}

void opcode0xE7()
{ /* E7 OUT Ib eAX */
  oper1b = getmem8(segregs[regcs], ip);
  StepIP(1);
  ports.write16(oper1b, (getreg16(regax)));
}

void opcode0xE8()
{ /* E8 CALL Jv */
  oper1 = getmem16(segregs[regcs], ip);
  StepIP(2);
  push(ip);
  ip = ip + oper1;
}

void opcode0xE9()
{ /* E9 JMP Jv */
  oper1 = getmem16(segregs[regcs], ip);
  StepIP(2);
  ip = ip + oper1;
}

void opcode0xEA()
{ /* EA JMP Ap */
  oper1 = getmem16(segregs[regcs], ip);
  StepIP(2);
  oper2 = getmem16(segregs[regcs], ip);
  ip = oper1;
  segregs[regcs] = oper2;
}

void opcode0xEB()
{ /* EB JMP Jb */
  oper1 = signext(getmem8(segregs[regcs], ip));
  StepIP(1);
  ip = ip + oper1;
}

void opcode0xEC()
{ /* EC IN regs.byteregs[regal] regdx */
  oper1 = (getreg16(regdx));
  regs.byteregs[regal] = (uint8_t)ports.read(oper1);
}

void opcode0xED()
{ /* ED IN eAX regdx */
  oper1 = (getreg16(regdx));
  putreg16(regax, ports.read16(oper1));
}

void opcode0xEE()
{ /* EE OUT regdx regs.byteregs[regal] */
  oper1 = (getreg16(regdx));
  // portWrite (oper1, regs.byteregs[regal]);
  ports.write(oper1, regs.byteregs[regal]);
}

void opcode0xEF()
{ /* EF OUT regdx eAX */
  oper1 = (getreg16(regdx));
  ports.write16(oper1, (getreg16(regax)));
}

void opcode0xF0()
{ /* F0 LOCK */
}

void opcode0xF4()
{ /* F4 HLT */
  ip--;
}

void opcode0xF5()
{	//F5 CMC
  cf= (!cf)?1:0;
}

void opcode0xF6()
{ /* F6 GRP3a Eb */
  modregrm();
  oper1b = readrm8(rm);
  op_grp3_8();
  if ((reg > 1) && (reg < 4))
  {
    writerm8(rm, res8);
  }
}

void opcode0xF7()
{ /* F7 GRP3b Ev */
  modregrm();
  oper1 = readrm16(rm);
  op_grp3_16();
  if ((reg > 1) && (reg < 4))
  {
    writerm16(rm, res16);
  }
}

void opcode0xF8()
{ /* F8 CLC */
  cf = 0;
}

void opcode0xF9()
{ /* F9 STC */
  cf = 1;
}

void opcode0xFA()
{ /* FA CLI */
  ifl = 0;
}

void opcode0xFB()
{ /* FB STI */
  ifl = 1;
}

void opcode0xFC()
{ /* FC CLD */
  df = 0;
}

void opcode0xFD()
{ /* FD STD */
  df = 1;
}

void opcode0xFE()
{ /* FE GRP4 Eb */
  modregrm();
  oper1b = readrm8(rm);
  oper2b = 1;
  if (!reg)
  {
    tempcf = cf;
    res8 = oper1b + oper2b;
    flag_add8(oper1b, oper2b);
    cf = tempcf;
    writerm8(rm, res8);
  }
  else
  {
    tempcf = cf;
    res8 = oper1b - oper2b;
    flag_sub8(oper1b, oper2b);
    cf = tempcf;
    writerm8(rm, res8);
  }
}

void opcode0xFF()
{ /* FF GRP5 Ev */
  modregrm();
  oper1 = readrm16(rm);
  op_grp5();
}

void opcodeStub()
{

}

void opcodeInv()
{
#ifdef CPU_V20
  intcall86 (6);  /* trip invalid opcode exception (this occurs on the 80186+, 8086/8088 CPUs treat them as NOPs. */
                  /* technically they aren't exactly like NOPs in most cases, but for our pursoses, that's accurate enough. */
#endif
}