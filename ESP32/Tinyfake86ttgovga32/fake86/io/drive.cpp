#include "drive.h"

#define TAG "drive"

SdCard Drive_t::sdCard;

Drive_t::Drive_t()
{
  pImage = nullptr;
}

bool Drive_t::isReady()
{
  return pImage != nullptr;
}

uint8_t Drive_t::read(DISK_ADDR &src, uint8_t * dst)
{
  if (!isValid(src))
    return RESULT_WRONG_PARAM;
  uint32_t filePos = lba(src) * SECTOR_SIZE;
  const uint32_t size = src.sectorCount * SECTOR_SIZE;
  if (pImage == nullptr)
    return RESULT_NOT_READY;
  if (fseek(pImage, filePos, SEEK_SET) != 0)
    return RESULT_TRACK_NOT_FOUND;
  if (fread(dst, size, 1, pImage) != 1)
    return RESULT_GENERAL_FAILURE;

  return RESULT_OK;
}

uint8_t Drive_t::write(uint8_t * src, DISK_ADDR &dst)
{
  if (!isValid(dst))
    return RESULT_WRONG_PARAM;
  if (pImage == nullptr)
    return RESULT_NOT_READY;
  uint32_t filePos = lba(dst) * SECTOR_SIZE;
  uint32_t size = dst.sectorCount * SECTOR_SIZE;
  uint32_t sector;

  for (sector = 0; sector < dst.sectorCount; sector++)
  {
    {
      if(fseek(pImage, filePos, SEEK_SET) != 0)
        return RESULT_TRACK_NOT_FOUND;
      if(fwrite(src, SECTOR_SIZE, 1, pImage) != 1)
        return RESULT_GENERAL_FAILURE;
    }

    src += SECTOR_SIZE;
    filePos += SECTOR_SIZE;
    if (filePos >= (geometry.capacity - 1))
      break;
  }
  if (fflush(pImage) != 0)
    return (RESULT_GENERAL_FAILURE);
  return RESULT_OK;
}

void Drive_t::getGeometry(Geometry_t *dst)
{
  *dst = geometry;
}

uint32_t Drive_t::lba(DISK_ADDR const & a)
{
  return (a.cylinder * geometry.heads + a.head) * geometry.sectors + a.sector - 1;
}

bool Drive_t::isValid(DISK_ADDR const & a)
{ 
  return (a.sector != 0) && ((lba(a) * SECTOR_SIZE < (geometry.capacity - 1)));
};

bool Drive_t::openImage(char const *imgName)
{
  if (pImage != nullptr)
    fclose(pImage);
  static const uint32_t LENGTH = 256;
  ESP_LOGI(TAG, "Opening image '%s'...", imgName);
  pImage = fopen(imgName, "r+");
  const bool result = (pImage != nullptr);
  if(result)
     ESP_LOGI(TAG, "OK");
  else
     ESP_LOGE(TAG, "FAILED!");
  imgIndex = 0;
  return result;
}
