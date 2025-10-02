#include "i8250.h"

I8250_t::I8250_t(uint16_t baseAddress, uint32_t intr)
{
  this->baseAddress = baseAddress;
  this->intr = intr;

  regDR  = new UartDR_t(baseAddress + UART_DR, this);                   ///< Data register.               r/w
  regDLL = regDR;                                                       ///< Divisor Latch LSB            w
  regIER = new UartIER_t(baseAddress + UART_IER, this);                 ///< Interrupt enable register    w
  regDLH = regIER;                                                      ///< Divisor latch high byte.     w
  regIIR = new IOPort(baseAddress + UART_IIR, 0x00, nullptr, nullptr);  ///< Pending interrupt register.  r
  regLCR = new IOPort(baseAddress + UART_LCR, 0x00, nullptr, nullptr);  ///< Line control register.       r/w
  regMCR = new UartMCR_t(baseAddress + UART_MCR, this);                 ///< Modem control register.      w
  regLSR = new IOPort(baseAddress + UART_LSR, 0x00, nullptr, nullptr);  ///< Line status register.        r
  regMSR = new IOPort(baseAddress + UART_MSR, 0x00, nullptr, nullptr);  ///< Modem status register.       r
  regSR  = new IOPort(baseAddress + UART_SR,  0x00, nullptr, nullptr);  ///< Scratchpad
}

uint8_t I8250_t:: readDR()
{

}

void I8250_t::writeDR(uint8_t data)
{

}
