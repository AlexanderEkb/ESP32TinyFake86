#ifndef _DRIVE_H_
#define _DRIVE_H_

#include "io/sdcard.h"
#include "io/diskio.h"

class Drive_t
{
  public:
    Drive_t() {};
    virtual void read(DISK_ADDR &src, MEM_ADDR &dst) = 0;
    virtual void write(DISK_ADDR & dst, MEM_ADDR & src) = 0;
    void setImage(char * pName);
    void setImage(uint32_t index);
  protected:
    struct
    {
      uint32_t cylinders;
      uint32_t heads;
      uint32_t sectors;
      uint32_t capacity;
    } geometry;
};

class CachedDrive_t : public Drive_t
{
  public:
    CachedDrive_t() {};
  protected:
    static constexpr uint32_t CACHE_SIZE_SECT = 128;
    typedef struct CacheEntry_t
    {
      static const uint32_t NOT_USED = 0xFFFFFFFF;
      CacheEntry_t()
      {
        sector = NOT_USED;
        refCount = 0;
      };
      uint32_t sector;
      uint32_t refCount;
      uint8_t buffer[SECTOR_SIZE];
    } CacheEntry_t;
    CacheEntry_t cache[CACHE_SIZE_SECT];
};

#endif /* _DRIVE_H_ */