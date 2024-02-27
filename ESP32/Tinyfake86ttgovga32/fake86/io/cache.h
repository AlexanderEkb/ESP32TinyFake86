#ifndef _IO_CACHE_H_
#define _IO_CACHE_H_

#include <stdint.h>
#include "ll/ll_list.h"

using ll::sortedList;
using ll::listEntry;

static const uint32_t CACHE_ENTRY_SIZE  = 512;
static const uint32_t CACHE_CAPACITY    = 128;

class CacheEntry_t : public listEntry
{
  public:
    uint32_t address;
    uint8_t *buffer;
    CacheEntry_t()
    {
      address = 0xFFFFFFFF;
    }

    virtual bool greaterThan(listEntry * e)
    {
      CacheEntry_t * ce = reinterpret_cast<CacheEntry_t *>(e);
      return address > ce->address;
    }
};

class Cache_t : public sortedList
{
  public:
    Cache_t(uint8_t * location)
    {
      for(uint32_t i=0; i<CACHE_CAPACITY; i++)
      {
        cache[i].buffer = location + i * CACHE_ENTRY_SIZE;
        add(&cache[i]);
      }
    };
    uint8_t * get(uint32_t address)
    {
      CacheEntry_t * ce = reinterpret_cast<CacheEntry_t *>(_root);
    }
  protected:
    CacheEntry_t cache[CACHE_CAPACITY];
};

#endif /* _IO_CACHE_H_ */