#include "extensions.h"
#include <driver/i2c.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp32-hal-log.h>

#define TAG "EXT"

/*
 * 0: W - parameter index    R - length
 * 1: data
 * 
 * 0x00 Device-specific data
 * 0xFD ID[2]
 * 0xFE Device name[16]
 * S A+W A 0xFD A S A+R A 0x?? A 0x?? A P
 */

Scan_t Extensions_t::scan = {{0}, 0, 0};

void Extensions_t::init()
{
  // esp_log_level_set(TAG, ESP_LOG_VERBOSE);
  // i2c_config_t conf = {};
  // conf.mode = I2C_MODE_MASTER;
  // conf.sda_io_num = 18;
  // conf.scl_io_num = 19;
  // conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  // conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  // conf.master.clk_speed = 400000;
  // conf.clk_flags = 0;

  // i2c_param_config(I2C_NUM_0, &conf);
  // i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, ESP_INTR_FLAG_IRAM);
  // const uint32_t coreID = xPortGetCoreID();
  // const uint32_t TARGET_CORE = SOC_CPU_CORES_NUM - coreID - 1;
  // xTaskHandle foo;
  // xTaskCreatePinnedToCore(task, "EXT", 3072, nullptr, tskIDLE_PRIORITY, &foo, TARGET_CORE);
  // assert(foo);
}

void Extensions_t::doScan()
{
  // if(scan.count < Scan_t::MAX_DEV_COUNT)
  // {
  //   scan.nextAbsent();
  //   if(ping(scan.addr))
  //   {
  //     onDeviceAttached(scan.addr);
  //   }
  // }
}

void Extensions_t::onDeviceAttached(uint8_t addr)
{
  ESP_LOGI(TAG, "Device 0x%02X attached", addr);
}

bool Extensions_t::ping(uint8_t addr)
{
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, addr, true);
  i2c_master_stop(cmd);
  const bool result = (i2c_master_cmd_begin(I2C_NUM_0, cmd, 2) == ESP_OK);
  i2c_cmd_link_delete(cmd);
  return result;
}

void Extensions_t::task(void * p)
{
  ESP_LOGI(TAG, "Task created on core #%i", xPortGetCoreID());
  while(true)
  {
    vTaskDelay(1000 / (40 * portTICK_PERIOD_MS));
    doScan();
  }
}