#pragma once

#include "storage_device.h"
#include <cstdint>

class ramDisk : storage_device {
  private:
  uintptr_t disk_addr = 0;
  uint32_t sector_size = 512;
  
  public:

  private:
  void init_ramdisk(uintptr_t disk_addr);

  public:
  uint8_t read_sector(uint8_t* buff, uint64_t start_sector, uint16_t size);
  uint8_t write_sector(uint8_t* buff, uint64_t start_sector, uint16_t size);
  uint8_t read_data(uint8_t* buff, uint64_t offset, uint32_t size);
  uint8_t write_data(uint8_t* buff, uint64_t offset, uint32_t size);
  void commit_transaction(uint8_t* buff, uint64_t start_sector, uint16_t num_of_sectors, bool write);
  uint64_t get_sector_size();
  ramDisk(uintptr_t ramdisk_addr);
};

// void init_ramdisk(uintptr_t disk_addr);
// void read_from_disk(uint8_t* buff, uint64_t start_sector, uint16_t size);
// void write_to_disk(uint8_t* buff, uint64_t start_sector, uint16_t size);
// void commit_transaction(uint8_t* buff, uint64_t start_sector, uint16_t num_of_sectors, bool write);
