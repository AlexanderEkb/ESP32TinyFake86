#ifndef _I8250_H_
#define _I8250_H_

#include "../cpu/ports.h"

class I8250_t 
{
  public:
    I8250_t(uint16_t baseAddress, uint32_t intr);
    // "Equipment" interface
    void onRx(uint8_t data);
    void onTx(uint8_t data);

    uint8_t readDR();
    void writeDR(uint8_t data);
    uint8_t readMCR();
    void writeMCR(uint8_t data);
    uint8_t readIER();
    void writeIER(uint8_t data);
    uint8_t readLSR();
    uint8_t readMSR();

  protected:
    // i8250 UART registers:
    static const uint32_t UART_DR           = 0;  ///< Data register.               r/w
    static const uint32_t UART_DL           = 1;  ///< Divisor latch high byte.     w
    static const uint32_t UART_IER          = 1;  ///< Interrupt enable register.   w
    static const uint32_t UART_IIR          = 2;  ///< Pending interrupt register.  r
    static const uint32_t UART_LCR          = 3;  ///< Line control register.       r/w
    static const uint32_t UART_MCR          = 4;  ///< Modem control register.      w
    static const uint32_t UART_LSR          = 5;  ///< Line status register.        r
    static const uint32_t UART_MSR          = 6;  ///< Modem status register.       r
    static const uint32_t UART_SR           = 7;  ///< Scratchpad
    static const uint32_t _REG_COUNT        = 8;

    // i8250 UART registers' bitfields:
    static const uint8_t UART_IER_RXNE      = 0x01;
    static const uint8_t UART_IER_TXE       = 0x02;
    static const uint8_t UART_IER_LS        = 0x04;
    static const uint8_t UART_IER_MS        = 0x08;
    static const uint8_t UART_IER_Mask      = 0x0F;

    static const uint8_t UART_IIR_NOPI      = 0x01;
    static const uint8_t UART_IIR_IS_Mask   = 0x06;
    static const uint8_t UART_IIR_IS_LS     = 0x00;
    static const uint8_t UART_IIR_IS_RXNE   = 0x02;
    static const uint8_t UART_IIR_IS_TXE    = 0x04;
    static const uint8_t UART_IIR_IS_MS     = 0x06;

    static const uint8_t UART_LCR_WL_Mask   = 0x03;
    static const uint8_t UART_LCR_WL_5BIT   = 0x00;
    static const uint8_t UART_LCR_WL_6BIT   = 0x01;
    static const uint8_t UART_LCR_WL_7BIT   = 0x02;
    static const uint8_t UART_LCR_WL_8BIT   = 0x03;
    static const uint8_t UART_LCR_2STOP     = 0x04;
    static const uint8_t UART_LCR_PEN       = 0x08;
    static const uint8_t UART_LCR_EPS       = 0x10;
    static const uint8_t UART_LCR_SPAR      = 0x20;
    static const uint8_t UART_LCR_BRK       = 0x40;
    static const uint8_t UART_LCR_DLAB      = 0x80;

    static const uint8_t UART_MCR_DTR       = 0x01;
    static const uint8_t UART_MCR_RTS       = 0x02;
    static const uint8_t UART_MCR_OUT1      = 0x04;
    static const uint8_t UART_MCR_OUT2      = 0x08;
    static const uint8_t UART_MCR_LOOPBK    = 0x10;
    static const uint8_t UART_MCR_Mask      = 0x1F;

    static const uint8_t UART_LSR_RXNE      = 0x01;
    static const uint8_t UART_LSR_OE        = 0x02;
    static const uint8_t UART_LSR_PE        = 0x04;
    static const uint8_t UART_LSR_FE        = 0x08;
    static const uint8_t UART_LSR_BI        = 0x10;
    static const uint8_t UART_LSR_TXE       = 0x20;
    static const uint8_t UART_LSR_TXSE      = 0x40;
    static const uint8_t UART_LSR_Mask      = 0x7F;

    static const uint8_t UART_MSR_DCTS      = 0x01;
    static const uint8_t UART_MSR_DDSR      = 0x02;
    static const uint8_t UART_MSR_TERI      = 0x04;
    static const uint8_t UART_MSR_DDCD      = 0x08;
    static const uint8_t UART_MSR_CTS       = 0x10;
    static const uint8_t UART_MSR_DSR       = 0x20;
    static const uint8_t UART_MSR_RI        = 0x40;
    static const uint8_t UART_MSR_DCD       = 0x80;

    IOPort * regDR;
    IOPort * regIER;
    IOPort * regIIR;
    IOPort * regLCR;
    IOPort * regMCR;
    IOPort * regLSR;
    IOPort * regMSR;
    IOPort * regSR;

    uint8_t rx;
    uint8_t tx;

    bool DTR;
    bool RTS;
    bool OUT1;
    bool OUT2;

    bool CTS;
    bool DSR;
    bool RI;
    bool DCD;
    
    uint8_t regs[_REG_COUNT];
    uint8_t divisorLatch[2];
    uint16_t baseAddress;
    uint32_t intr;

    bool isLoop();
    bool isDlaEn();
};

class UartIER_t : public IOPort
{
  public:
    UartIER_t(uint32_t address, I8250_t * owner)
    {
      this->address = address;
      this->owner = owner;

      IOPortSpace::getInstance().insert(this);
    }
    virtual uint8_t in() override {return owner->readIER();}
    virtual void out(uint8_t data) override {owner->writeIER(data);}
  private:
    I8250_t * owner;
};

class UartMCR_t : public IOPort
{
  public:
    UartMCR_t(uint32_t address, I8250_t * owner)
    {
      this->address = address;
      this->owner = owner;

      IOPortSpace::getInstance().insert(this);
    }
    virtual uint8_t in() override {return owner->readMCR();}
    virtual void out(uint8_t data) override {owner->writeMCR(data);}
  private:
    I8250_t * owner;
};

class UartDR_t : public IOPort
{
  public:
    UartDR_t(uint32_t address, I8250_t * owner)
    {
      this->address = address;
      this->owner = owner;

      IOPortSpace::getInstance().insert(this);
    }
    virtual uint8_t in() override {return owner->readDR();}
    virtual void out(uint8_t data) override {owner->writeDR(data);}
  private:
    I8250_t * owner;
};

class UartLSR_t : public IOPort
{
  public:
    UartLSR_t(uint32_t address, I8250_t * owner)
    {
      this->address = address;
      this->owner = owner;

      IOPortSpace::getInstance().insert(this);
    }
    virtual uint8_t in() override {return owner->readLSR();}
  private:
    I8250_t * owner;
};

class UartMSR_t : public IOPort
{
  public:
    UartMSR_t(uint32_t address, I8250_t * owner)
    {
      this->address = address;
      this->owner = owner;

      IOPortSpace::getInstance().insert(this);
    }
    virtual uint8_t in() override {return owner->readMSR();}
  private:
    I8250_t * owner;
};

#endif /* _I8250_H_ */