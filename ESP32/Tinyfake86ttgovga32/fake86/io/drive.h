#ifndef _DRIVE_H_
#define _DRIVE_H_

#include "io/sdcard.h"
#include "io/diskio.h"

class Drive_t
{
  public:
    Drive_t();
    virtual void read(DISK_ADDR &src, MEM_ADDR &dst);
    virtual void write(DISK_ADDR & dst, MEM_ADDR & src);
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

#endif /* _DRIVE_H_ */