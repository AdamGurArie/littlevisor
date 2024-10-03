#include "ram_disk.h"
#include <cstdint>
#include <sys/types.h>
#include "../common.h"
#include "../kheap.h"

// static uintptr_t disk_addr = 0;
// static uint32_t sector_size = 512;

void ramDisk::init_ramdisk(uintptr_t disk_addr) {
  this->disk_addr = disk_addr;
}

uint8_t ramDisk::read_sector(uint8_t* buff, uint64_t start_sector, uint16_t size) {
  kmemcpy(
        buff,
        (uint8_t*)(this->disk_addr + start_sector * sector_size),
        size * sector_size
  );

  return 0;
}

uint8_t ramDisk::write_sector(uint8_t* buff, uint64_t start_sector, uint16_t size) {
  kmemcpy(
      (uint8_t*)(this->disk_addr + start_sector * sector_size),
      (uint8_t*)buff,
      size * sector_size
  );

  return 0;
}

uint8_t ramDisk::read_data(uint8_t* buff, uint64_t offset, uint32_t size) {
  kmemcpy(
      buff,
      (uint8_t*)(this->disk_addr + offset),
      size
  );

  return 0;
}

uint8_t ramDisk::write_data(uint8_t* buff, uint64_t offset, uint32_t size) {
  kmemcpy(
      (uint8_t*)(this->disk_addr + offset),
      buff,
      size
      );

  return 0;
}

void ramDisk::commit_transaction(uint8_t* buff, uint64_t start_sector, uint16_t num_of_sectors, bool write) {
  if(write) {
    this->write_sector(buff, start_sector, num_of_sectors);
  } else {
    this->read_sector(buff, start_sector, num_of_sectors);
  }
}

uint64_t ramDisk::get_sector_size() {
  return this->sector_size;
}

ramDisk::ramDisk(uintptr_t ramdisk_addr) {
  init_ramdisk(ramdisk_addr);
}
