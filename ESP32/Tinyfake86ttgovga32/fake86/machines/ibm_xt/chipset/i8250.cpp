#include "i8250.h"

I8250_t::I8250_t(uint16_t baseAddress, uint32_t intr)
{
  this->baseAddress = baseAddress;
  this->intr = intr;

  regDR  = new UartDR_t(baseAddress + UART_DR, this);                   ///< Data register.               r/w
  regIER = new UartIER_t(baseAddress + UART_IER, this);                 ///< Interrupt enable register    w
  regMCR = new UartMCR_t(baseAddress + UART_MCR, this);                 ///< Modem control register.      w
  regLSR = new UartLSR_t(baseAddress + UART_LSR, this);                 ///< Line status register.        r
  regMSR = new UartMSR_t(baseAddress + UART_MSR, this);                 ///< Modem status register.       r

  regIIR = new IOPort(baseAddress + UART_IIR, 0x00, nullptr, nullptr);  ///< Pending interrupt register.  r
  regLCR = new IOPort(baseAddress + UART_LCR, 0x00, nullptr, nullptr);  ///< Line control register.       r/w
  regSR  = new IOPort(baseAddress + UART_SR,  0x00, nullptr, nullptr);  ///< Scratchpad

  rx    = 0;
  tx    = 0;
  DTR   = false;
  RTS   = false;
  OUT1  = false;
  OUT2  = false;
  CTS   = false;
  DSR   = false;
  RI    = false;
  DCD   = false;

  divisorLatch[0] = 0;
  divisorLatch[0] = 1;

  regIIR->value = 0x01;
  regs[UART_LSR] = UART_LSR_TXE | UART_LSR_TXSE;
}

void I8250_t::onRx(uint8_t data)
{
  // IRQ !!!
  rx = data;
  regs[UART_LSR] |= UART_LSR_RXNE;
}

void I8250_t::onTx(uint8_t data)
{
  // IRQ !!!
  tx = data;
  regs[UART_LSR] &= ~(UART_LSR_TXE | UART_LSR_TXSE);
  if(isLoop())
    onRx(data);
}

uint8_t I8250_t::readDR()
{
  if(isDlaEn())
    return divisorLatch[0];
  else
  {
    regs[UART_LSR] &= ~UART_LSR_RXNE;
    return rx;
  }
}

void I8250_t::writeDR(uint8_t data)
{
  if(isDlaEn())
    divisorLatch[0] = data;
  else
    onTx(data);
}

uint8_t I8250_t::readMCR()
{
  return regs[UART_MCR] & UART_MCR_Mask;
}

void I8250_t::writeMCR(uint8_t data)
{
  bool const wasLoop = (regs[UART_MCR] & UART_MCR_LOOPBK);
  bool const isLoop  = (data & UART_MCR_LOOPBK);
  if(wasLoop && !isLoop)
  {
    // restore prev values in rx and msr
  }

  regs[UART_MCR] = data & UART_MCR_Mask;
  DTR   = !(data & UART_MCR_DTR);
  RTS   = !(data & UART_MCR_RTS);
  OUT1  = !(data & UART_MCR_OUT1);
  OUT2  = !(data & UART_MCR_OUT2);
  if(isLoop)
  {
    bool const loopDSR = !(data & UART_MCR_DTR);
    bool const loopCTS = !(data & UART_MCR_RTS);
    bool const loopRI  = !(data & UART_MCR_OUT1);
    bool const loopDCD = !(data & UART_MCR_OUT2);
    regs[UART_MSR] = 
      ((CTS ^ loopCTS) ? 0x01 : 0x00) |
      ((DSR ^ loopDSR) ? 0x02 : 0x00) |
      ((loopRI && !RI) ? 0x04 : 0x00) |
      ((DCD ^ loopDCD) ? 0x08 : 0x00) |
      ((loopCTS)       ? 0x00 : 0x10) |
      ((loopDSR)       ? 0x00 : 0x20) |
      ((loopRI)        ? 0x00 : 0x40) |
      ((loopDCD)       ? 0x00 : 0x80);
  }
}

uint8_t I8250_t::readIER()
{
  return isDlaEn() ? divisorLatch[1] : (regs[UART_IER] & UART_IER_Mask);

}

void I8250_t::writeIER(uint8_t data)
{
  if(regs[UART_LCR] & UART_LCR_DLAB)
    divisorLatch[1] = data;
  else
    regs[UART_IER] = data & UART_IER_Mask;
}

uint8_t I8250_t::readLSR()
{
  uint8_t data = regs[UART_LSR] & UART_LSR_Mask;
  regs[UART_LSR] &= ~(UART_LSR_OE | UART_LSR_PE | UART_LSR_FE | UART_LSR_BI);
  regs[UART_LSR] |= UART_LSR_TXE | UART_LSR_TXSE;
  return data;
}

uint8_t I8250_t::readMSR()
{
  uint8_t data = regs[UART_MSR];
  regs[UART_MSR] &= ~(UART_MSR_DCTS | UART_MSR_DDSR | UART_MSR_TERI | UART_MSR_DDCD);
  return data;
}

bool I8250_t::isLoop()
{
  return regs[UART_MCR] & UART_MCR_LOOPBK;
}

bool I8250_t::isDlaEn()
{
  return (regLCR->value & UART_LCR_DLAB);
}