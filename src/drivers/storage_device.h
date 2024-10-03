#pragma once

#include "../kheap.h"
#include <cstdint>

class storage_device {
  public:
  virtual uint8_t read_sector(uint8_t* buff, uint64_t start_sector, uint16_t size) = 0;
  virtual uint8_t write_sector(uint8_t* buff, uint64_t start_sector, uint16_t size) = 0;
  virtual uint8_t read_data(uint8_t* buff, uint64_t offset, uint32_t size) = 0;
  virtual uint8_t write_data(uint8_t* buff, uint64_t offset, uint32_t size) = 0;
  virtual uint64_t get_sector_size() = 0;
  void* operator new(uint64_t size) {
    return kmalloc(size);
  }; 

  void operator delete(void* p, uint64_t param) {
    (void)param;
    return kfree(p);
  }

  virtual ~storage_device() = default;
};
