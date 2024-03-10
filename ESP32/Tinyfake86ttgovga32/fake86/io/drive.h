#ifndef _IO_DRIVE_H_
#define _IO_DRIVE_H_

#include <esp_vfs_fat.h>
#include <stdint.h>
#include "sdcard.h"

#define RESULT_OK (0x00)
#define RESULT_WRONG_PARAM (0x01)
#define RESULT_WRITE_PROT (0x03)
#define RESULT_SECT_NOT_FOUND (0x04)
#define RESULT_GENERAL_FAILURE (0x20)
#define RESULT_TRACK_NOT_FOUND (0x40)
#define RESULT_NOT_READY (0xAA)

typedef struct Geometry_t
{
  uint32_t heads;
  uint32_t cylinders;
  uint32_t sectors;
  uint32_t capacity;
  Geometry_t(Geometry_t &rvalue) : heads(heads), cylinders(cylinders), sectors(sectors), capacity(capacity){};
  Geometry_t() : heads(0), cylinders(0), sectors(0), capacity(0){};
  Geometry_t & operator= (Geometry_t & rvalue)
  {
    heads = rvalue.heads;
    cylinders = rvalue.cylinders;
    sectors = rvalue.sectors;
    capacity = rvalue.capacity;
    return *this;
  }
} Geometry_t;

typedef struct MEM_ADDR
{
  uint16_t segment;
  uint16_t offset;
  MEM_ADDR(uint16_t seg, uint16_t off) : segment(seg),
                                         offset(off) {}
  uint32_t linear() { return ((uint32_t)segment << 4) + offset; }
} MEM_ADDR;

typedef struct DISK_ADDR
{
  uint32_t drive;
  uint32_t cylinder;
  uint32_t sector;
  uint32_t head;
  uint32_t sectorCount;
  DISK_ADDR(uint32_t drv, uint32_t cyl, uint32_t hd, uint32_t sect, uint32_t cnt) : drive(drv),
                                                                                    cylinder(cyl),
                                                                                    sector(sect),
                                                                                    head(hd),
                                                                                    sectorCount(cnt)
  {
  }
} DISK_ADDR;

class Drive_t
{
  public:
      static SdCard sdCard;
      Drive_t();
      uint8_t read(DISK_ADDR &src, uint8_t *dst);
      uint8_t write(uint8_t *src, DISK_ADDR &dst);
      void getGeometry(Geometry_t *dst);
      bool isReady();
      bool openImage(char const *imgName);

  protected:
      static const uint32_t MAX_NAME_LENGTH = 256;
      static const uint32_t SECTOR_SIZE = 512;
      Geometry_t geometry;
      FILE *pImage;
      int32_t imgIndex;
      uint32_t lba(DISK_ADDR const &a);
      bool isValid(DISK_ADDR const &a);
};

class FloppyDrive_t : public Drive_t
{
  public:
    FloppyDrive_t()
    {
      geometry.heads = 2;
      geometry.cylinders = 80;
      geometry.sectors = 18;
      geometry.capacity = 1474560;
    };
};

class HDD_t : public Drive_t
{
  public:
      HDD_t()
      {
        geometry.heads = 64;
        geometry.cylinders = 512;
        geometry.sectors = 63;
        geometry.capacity = 1056964608;
      };
};

#endif /* _IO_DRIVE_H_ */