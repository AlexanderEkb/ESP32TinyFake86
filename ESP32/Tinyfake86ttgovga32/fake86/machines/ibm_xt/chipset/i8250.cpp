#include "i8250.h"
#include "i8259.h"
#include "esp32-hal-log.h"

#define TAG "8250"

I8250_t::I8250_t(uint16_t baseAddress, uint32_t intr)
{
  this->baseAddress = baseAddress;
  this->intr = intr;
  this->periph = nullptr;

  inbound = xQueueCreate(16, sizeof(uint8_t));
  assert(inbound);

  regDR  = new UartReg_t(baseAddress + UART_DR,  this); ///< Data register.               r/w
  regIER = new UartReg_t(baseAddress + UART_IER, this); ///< Interrupt enable register    w
  regMCR = new UartReg_t(baseAddress + UART_MCR, this); ///< Modem control register.      w
  regLSR = new UartReg_t(baseAddress + UART_LSR, this); ///< Line status register.        r
  regMSR = new UartReg_t(baseAddress + UART_MSR, this); ///< Modem status register.       r

  regIIR = new UartReg_t(baseAddress + UART_IIR, this);  ///< Pending interrupt register.  r
  regLCR = new UartReg_t(baseAddress + UART_LCR, this);  ///< Line control register.       r/w
  regSR  = new UartReg_t(baseAddress + UART_SR,  this);  ///< Scratchpad

  rx = 0;
  tx = 0;

  divisorLatch[0] = 0;
  divisorLatch[1] = 0;

  regs[UART_IIR] = UART_IIR_NOPI;
  regs[UART_LSR] = UART_LSR_THRE | UART_LSR_TXSE;
  regs[UART_MSR] = UART_MSR_CTS | UART_MSR_DSR | UART_MSR_RI | UART_MSR_DCD;
}

void I8250_t::bind(SerialPeripheral_t * periph)
{
  assert(periph);
  this->periph = periph;
}

void I8250_t::setModemStatusLine(uint8_t mask, uint8_t code)
{
  if(!isLoop())
  {
    static uint8_t const LINES = UART_MSR_DCD | UART_MSR_RI | UART_MSR_DSR | UART_MSR_CTS;
    uint8_t const strictMask = (mask & LINES);
    uint8_t temp = regs[UART_MSR];
    temp &= strictMask;
    temp |= (code & strictMask);
    uint8_t const deltas = ((regs[UART_MSR] & strictMask) ^ temp) >> 4;
    regs[UART_MSR] = temp | deltas;
    updateIntrStatus();
  }
}

uint8_t I8250_t::onRegRead(uint32_t addr)
{
  uint32_t const reg = addr - baseAddress;
  switch(reg)
  {
    case UART_DR:
      if(isDlaEn())
      {
        // ESP_LOGI(TAG, "DLL r %02Xh", divisorLatch[0]);
        return divisorLatch[0];
      }
      else
        return readDR();
    case UART_IER:
      if(isDlaEn())
      {
        // ESP_LOGI(TAG, "DLH r %02Xh", divisorLatch[1]);
        return divisorLatch[1];
      }
      else
        return readIER();
    case UART_IIR:
      // ESP_LOGI(TAG, "IIR r %02Xh", regs[UART_IIR] & UART_IIR_Mask);
      return regs[UART_IIR] & UART_IIR_Mask;
    case UART_LCR:
      // ESP_LOGI(TAG, "LCR r %02Xh", regs[UART_LCR]);
      return regs[UART_LCR];
    case UART_MCR:
      return readMCR();
    case UART_LSR:
      return readLSR();
    case UART_MSR:
      return readMSR();
    case UART_SR:
      // ESP_LOGI(TAG, "SR r %02Xh", regs[UART_SR]);
      return regs[UART_SR];
    default:
      return 0xFF;
  }
}

/*
Part of CheckIt test sequence:
[ 40593][I][i8250.cpp:191] writeMCR(): [8250]       MCR W 00h
[ 40641][I][i8250.cpp:177] readMCR(): [8250]        MCR r 00h
[ 40648][I][i8250.cpp:191] writeMCR(): [8250]       MCR W 10h
[ 40656][I][i8250.cpp:191] writeMCR(): [8250]       MCR W 1Fh
[ 40664][I][i8250.cpp:131] onRegWrite(): [8250] --- MSR W 00h
[ 40673][I][i8250.cpp:235] readMSR(): [8250]        MSR r F0h
[ 40681][I][i8250.cpp:131] onRegWrite(): [8250] --- MSR W 55h
[ 40690][I][i8250.cpp:235] readMSR(): [8250]        MSR r F0h
*/
void I8250_t::onRegWrite(uint32_t addr, uint8_t data)
{
  uint32_t const reg = addr - baseAddress;
  switch(reg)
  {
    case UART_DR:
      if(isDlaEn())
      {
        divisorLatch[0] = data;
        // ESP_LOGI(TAG, "DLL W %02Xh", data);
      }
      else
      {
        writeDR(data);
        // ESP_LOGI(TAG, "THR W %02Xh", data);
      }
      break;
    case UART_IER:
      if(isDlaEn())
      {
        divisorLatch[1] = data;
        // ESP_LOGI(TAG, "DLH W %02Xh", data);
      }
      else
      {
        writeIER(data);
        // ESP_LOGI(TAG, "IER W %02Xh", data);
      }
      break;
    case UART_IIR:
        // ESP_LOGI(TAG, "---IIR W %02Xh", data);
      break;
    case UART_LCR:
      regs[UART_LCR] = data;
      // ESP_LOGI(TAG, "LCR W %02Xh", data);
      break;
    case UART_MCR:
      writeMCR(data);
      break;
    case UART_LSR:
      // ESP_LOGI(TAG, "---LSR W %02Xh", data);
      break;
    case UART_MSR:
      regs[UART_MSR] &= ~(UART_MSR_DDSR | UART_MSR_DCTS | UART_MSR_TERI | UART_MSR_DDCD);
      // ESP_LOGI(TAG, "---MSR W %02Xh", data);
      break;
    case UART_SR:
      regs[UART_SR] = data;
      // ESP_LOGI(TAG, "SR W %02Xh", data);
      break;
  }
}


void I8250_t::onRx(uint8_t data)
{
  if(regs[UART_LSR] & UART_LSR_RXNE)
  {
    xQueueSend(inbound, &data, 0);
  }
  else
  {
    rx = data;
    regs[UART_LSR] |= UART_LSR_RXNE;
    updateIntrStatus();
  }
}

void I8250_t::onTx(uint8_t data)
{
  tx = data;
  regs[UART_LSR] &= ~(UART_LSR_THRE | UART_LSR_TXSE);
  updateIntrStatus();
  if(isLoop())
    onRx(data);
}

uint8_t I8250_t::readDR()
{
  const uint8_t readByte = rx;
  uint8_t next;
  portBASE_TYPE r = xQueueReceive(inbound, &next, 0);
  if(r == pdPASS)
    rx = next;
  else
    regs[UART_LSR] &= ~UART_LSR_RXNE;
  // ESP_LOGI(TAG, "RBR r %02Xh", readByte);
  updateIntrStatus();
  return readByte;
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
  uint8_t const data = regs[UART_MCR] & UART_MCR_Mask;
  // ESP_LOGI(TAG, "MCR r %02Xh", data);
  return data;
}

void I8250_t::writeMCR(uint8_t data)
{
  bool const wasLoop = isLoop();
  bool const nowLoop  = (data & UART_MCR_LOOPBK);
  if(wasLoop && !nowLoop)
  {
    // restore prev values in rx and msr
  }

  regs[UART_MCR] = data & UART_MCR_Mask;
  if(periph != nullptr)
  {
    periph->onModemControlEvent(data);
  }
  // ESP_LOGI(TAG, "MCR W %02Xh", data & UART_MCR_Mask);
  if(nowLoop)
  {
    uint8_t const DSR = (data & UART_MCR_DTR)   ? UART_MSR_DSR : 0;
    uint8_t const CTS = (data & UART_MCR_RTS)   ? UART_MSR_CTS : 0;
    uint8_t const RI  = (data & UART_MCR_OUT1)  ? UART_MSR_RI  : 0;
    uint8_t const DCD = (data & UART_MCR_OUT2)  ? UART_MSR_DCD : 0;

    uint8_t const inputs = DSR | CTS | RI | DCD;
    uint8_t deltas = (inputs ^ (regs[UART_MSR])) >> 4;
    regs[UART_MSR] = inputs | deltas;

    updateIntrStatus();
  }
}

uint8_t I8250_t::readIER()
{
  // ESP_LOGI(TAG, "IER r %02Xh", regs[UART_IER] & UART_IER_Mask);
  return (regs[UART_IER] & UART_IER_Mask);

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
  // ESP_LOGI(TAG, "LSR r %02Xh", data);
  regs[UART_LSR] &= ~(UART_LSR_OE | UART_LSR_PE | UART_LSR_FE | UART_LSR_BI);
  regs[UART_LSR] |= UART_LSR_THRE | UART_LSR_TXSE;
  updateIntrStatus();
  return data;
}

uint8_t I8250_t::readMSR()
{
  uint8_t data = regs[UART_MSR];
  // ESP_LOGI(TAG, "MSR r %02Xh", regs[UART_MSR]);
  regs[UART_MSR] &= ~(UART_MSR_DCTS | UART_MSR_DDSR | UART_MSR_TERI | UART_MSR_DDCD);
  return data;
}

bool I8250_t::isLoop()
{
  return regs[UART_MCR] & UART_MCR_LOOPBK;
}

bool I8250_t::isDlaEn()
{
  return (regs[UART_LCR] & UART_LCR_DLAB);
}

void I8250_t::updateIntrStatus()
{
  bool const receiverLineStatus_IRQ = (
    (regs[UART_LSR] & UART_LSR_OE) ||
    (regs[UART_LSR] & UART_LSR_PE) ||
    (regs[UART_LSR] & UART_LSR_FE) ||
    (regs[UART_LSR] & UART_LSR_BI)
  ) && (regs[UART_IER] & UART_IER_LS);
  if(receiverLineStatus_IRQ)
  {
    regs[UART_IIR] = UART_IIR_IS_RLS;
    doirq(intr);
    return;
  }

  bool const receivedDataAvailable_IRQ = 
    (regs[UART_LSR] & UART_LSR_RXNE) && 
    (regs[UART_IER] & UART_IER_RXNE);
  if(receivedDataAvailable_IRQ)
  {
    regs[UART_IIR] = UART_IIR_IS_RXNE;
    doirq(intr);
    return;
  }

  bool const txHoldingRegisterEmpty =
    (regs[UART_LSR] & UART_LSR_THRE) &&
    (regs[UART_IER] & UART_IER_THRE);
  if(txHoldingRegisterEmpty)
  {
    regs[UART_IIR] = UART_IIR_IS_THRE;
    doirq(intr);
    return;
  }

  bool const modemStatus_IRQ = (
    (regs[UART_MSR] & UART_MSR_DCTS) ||
    (regs[UART_MSR] & UART_MSR_DDSR) ||
    (regs[UART_MSR] & UART_MSR_TERI) ||
    (regs[UART_MSR] & UART_MSR_DDCD)
  ) && (regs[UART_IER] & UART_IER_MS);
  if(modemStatus_IRQ)
  {
    regs[UART_IIR] = UART_IIR_IS_MS;
    doirq(intr);
    return;
  }

  regs[UART_IIR] = UART_IIR_NOPI;
}


