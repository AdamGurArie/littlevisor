#pragma once

#include "storage_device.h"
#include "../kheap.h"
#include <cstdint>

class ramDisk {
  private:
  uintptr_t disk_addr = 0;
  uint32_t sector_size = 512;
  uint64_t disk_size = 0;
  
  public:

  private:
  void init_ramdisk(uintptr_t disk_addr, uint64_t disk_size);

  public:
  uint8_t read_sector(uint64_t start_sector, uint8_t* buff);
  uint8_t write_sector(uint64_t start_sector, uint8_t* buff);
  uint8_t read_data(uint8_t* buff, uint64_t offset, uint32_t size);
  uint8_t write_data(uint8_t* buff, uint64_t offset, uint32_t size);
  void commit_transaction(uint8_t* buff, uint64_t start_sector, uint16_t num_of_sectors, bool write);
  uint64_t get_sector_size();

  void* operator new(uint64_t size) {
    return kmalloc(size);
  }

  void operator delete(void* p, uint64_t param) noexcept {
    (void)param;
    kfree(p);
  }

  ramDisk(uintptr_t ramdisk_addr, uint64_t disk_size);
  ~ramDisk() = default;
};

// void init_ramdisk(uintptr_t disk_addr);
// void read_from_disk(uint8_t* buff, uint64_t start_sector, uint16_t size);
// void write_to_disk(uint8_t* buff, uint64_t start_sector, uint16_t size);
// void commit_transaction(uint8_t* buff, uint64_t start_sector, uint16_t num_of_sectors, bool write);
