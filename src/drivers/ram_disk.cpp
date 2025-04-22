#include "ram_disk.h"
#include <cstdint>
#include <sys/types.h>
#include "../common.h"
#include "../kheap.h"

// static uintptr_t disk_addr = 0;
// static uint32_t sector_size = 512;

void ramDisk::init_ramdisk(uintptr_t disk_addr, uint64_t disk_size) {
  this->disk_addr = disk_addr;
  this->disk_size = disk_size;
}

uint8_t ramDisk::read_sector(uint64_t start_sector, uint8_t* buff) {
  if(start_sector * sector_size >= this->disk_size) {
    return 1;
  }

  kmemcpy(
        buff,
        (uint8_t*)(this->disk_addr + start_sector * sector_size),
        sector_size
  );

  return 0;
}

uint8_t ramDisk::write_sector(uint64_t start_sector, uint8_t* buff) {
  if(start_sector * this->sector_size >= this->disk_size) {
    return 1;
  }
  
  kmemcpy(
      (uint8_t*)(this->disk_addr + start_sector * sector_size),
      (uint8_t*)buff,
      sector_size
  );

  return 0;
}

uint8_t ramDisk::read_data(uint8_t* buff, uint64_t offset, uint32_t size) {
  if(offset >= this->disk_size) {
    return 1;
  }

  kmemcpy(
      buff,
      (uint8_t*)(this->disk_addr + offset),
      size
  );

  return 0;
}

uint8_t ramDisk::write_data(uint8_t* buff, uint64_t offset, uint32_t size) {
  if(offset >= this->disk_size) {
    return 1;
  }

  kmemcpy(
      (uint8_t*)(this->disk_addr + offset),
      buff,
      size
  );

  return 0;
}

void ramDisk::commit_transaction(uint8_t* buff, uint64_t start_sector, uint16_t num_of_sectors, bool write) {
  if(write) {
    for(uint32_t i = 0; i < num_of_sectors; i++) {
      
      this->write_sector(
          start_sector + i,
          (uint8_t*)((uint64_t)buff + sector_size * i)
      );

    }

  } else {

    for(uint32_t i = 0; i < num_of_sectors; i++) {
    
      this->read_sector(
          start_sector + i,
          (uint8_t*)((uint64_t)buff + sector_size * i)
      );
    
    }
  
  }

}

uint64_t ramDisk::get_sector_size() {
  return this->sector_size;
}

ramDisk::ramDisk(uintptr_t ramdisk_addr, uint64_t disk_size) {
  init_ramdisk(ramdisk_addr, disk_size);
}
