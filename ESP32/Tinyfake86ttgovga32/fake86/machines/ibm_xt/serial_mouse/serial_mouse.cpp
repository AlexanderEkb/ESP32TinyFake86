#include "serial_mouse.h"
#include "esp32-hal-log.h"
#define TAG "MSMOUSE"

SerialMouse_t::SerialMouse_t(I8250_t * port)
{
  Host::mouse->bind(this);
  this->port = port;
  port->bind(this);
  RTS = 0;
}

void SerialMouse_t::onMouseEvent(int32_t dx, int32_t dy, uint8_t btn)
{
  if(RTS != 0)
  {
    uint8_t buffer[3];

    int8_t const dx_8 = static_cast<int8_t>(dx);
    int8_t const dy_8 = static_cast<int8_t>(-dy);
    int8_t const lb = (btn & MOUSE_BUTTON_L) ? (1 << 5) : 0;
    int8_t const rb = (btn & MOUSE_BUTTON_R) ? (1 << 4) : 0;

    buffer[0] = (1 << 6) | lb | rb;
    buffer[0] |= (dx_8 >> 6) & 0x03;
    buffer[0] |= (dy_8 >> 4) & 0x0C;
    buffer[1] = (dx_8 & 0x3F);
    buffer[2] = (dy_8 & 0x3F);
    port->onRx(buffer[0]);
    port->onRx(buffer[1]);
    port->onRx(buffer[2]);
  }
}

void SerialMouse_t::onModemControlEvent(uint8_t mcr)
{
  static uint8_t _RTS = 0;
  RTS = mcr & UART_MCR_RTS;

  uint8_t DTR = mcr & UART_MCR_DTR;
  port->setModemStatusLine(UART_MSR_DSR, DTR ? UART_MSR_DSR : 0);

  if((_RTS == 0) && (RTS != 0))
    port->onRx('M');
}
