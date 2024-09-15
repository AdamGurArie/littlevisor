#pragma once


#include <cstdint>
class storage_device {
  public:
  virtual uint8_t read_sector(uint32_t sector_number, uint8_t* buff) = 0;
  virtual uint8_t write_sector(uint32_t sector_number, uint8_t* buff) = 0;
  virtual uint64_t get_sector_size() = 0;
};
