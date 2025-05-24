#ifndef __IO_EXTENSIONS_H__
#define __IO_EXTENSIONS_H__

#include <stdint.h>

typedef struct Scan_t
{
  static const uint32_t MAX_DEV_COUNT = 128;
  bool map[MAX_DEV_COUNT];
  uint8_t addr;
  uint32_t count;
  bool nextAbsent()
  {
    if(count < MAX_DEV_COUNT)
    {
      {
        if(++addr >= MAX_DEV_COUNT)
          addr = 0;
      } while(map[addr]);
      return true;
    }
    else
      return false;
  }
} Scan_t;

class Extensions_t
{
  public:
    static void init();
  private:
    static Scan_t scan; 
    static void doScan();
    static void onDeviceAttached(uint8_t addr);
    static bool ping(uint8_t addr);
    static void task(void * p);
};

#endif /* __IO_EXTENSIONS_H__ */