#include "mouse_sun.h"

#if (MOUSE_DRIVER == 1)
#include "esp32-hal-log.h"
#include "esp32-hal-gpio.h"
#include <strings.h>

#define TAG "SUN"

void MouseSun_t::init()
{
  // Install UART driver using an event queue here
  ESP_ERROR_CHECK(uart_driver_install(uart_num, uart_buffer_size, 0, 0, nullptr, ESP_INTR_FLAG_LEVEL1));
  uart_config_t uart_config = {
      .baud_rate = 1200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
      .rx_flow_ctrl_thresh = 122,
  };
  // Configure UART parameters
  ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
  ESP_ERROR_CHECK(uart_set_rx_full_threshold(uart_num, 10));
  ESP_ERROR_CHECK(uart_set_pin(uart_num, -1, MOUSE_DATA, -1, -1));
  ESP_ERROR_CHECK(uart_set_line_inverse(uart_num, UART_SIGNAL_RXD_INV));
  gpio_set_pull_mode(GPIO_NUM_18, GPIO_PULLDOWN_ONLY);
  // xTaskCreatePinnedToCore(rx_task, "uart_rx_task", 2048, this, tskIDLE_PRIORITY, NULL, 0);
  xTaskCreate(rx_task, "uart_rx_task", 2048, this, tskIDLE_PRIORITY, NULL);
  const uint32_t coreID = xPortGetCoreID();
  ESP_LOGI(TAG, "Sun Mouse initialized: core #%i", coreID);
}

void IRAM_ATTR MouseSun_t::rx_task(void *arg)
{
  MouseSun_t *const instance = reinterpret_cast<MouseSun_t * const>(arg);
  uart_event_t event;
  int8_t bytes[PKT_LENGTH];
  int8_t b;
  uint32_t ptr = 0;

  while(true)
  {
    uart_read_bytes(uart_num, &b, 1, portMAX_DELAY);
    if((b & 0xF8) == 0x80) {
      ptr = 0;
    }
    bytes[ptr] = b;
    if(++ptr >= 5)
    {
      if(instance->sink != nullptr)
      {
        int32_t const dx = bytes[1] + bytes[3];
        int32_t const dy = -bytes[2] - bytes[4];
        uint8_t const btn = 
          ((bytes[0] & 0x04) ? 0x00 : 0x01) |
          ((bytes[0] & 0x01) ? 0x00 : 0x02);
        // ESP_LOGI(TAG, "%i, %i %02Xh", dx, dy, btn);
        instance->sink->onMouseEvent(dx, dy, btn);
      }
      ptr = 0;
    }
  }
}

#endif /* MOUSE_DRIVER */