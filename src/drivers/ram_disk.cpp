#include "ram_disk.h"
#include <cstdint>
#include "../common.h"

static uintptr_t disk_addr = 0;
static uint32_t sector_size = 512;

void init_ramdisk(uintptr_t disk_addr) {
  disk_addr = disk_addr;
}

void read_from_disk(uint8_t* buff, uint64_t start_sector, uint16_t size) {
  kmemcpy(
        buff,
        (uint8_t*)(disk_addr + start_sector * sector_size),
        size * sector_size
  );
}

void write_to_disk(uint8_t* buff, uint64_t start_sector, uint16_t size) {
  kmemcpy(
      (uint8_t*)(disk_addr + start_sector * sector_size),
      (uint8_t*)buff,
      size * sector_size
  );
}

void commit_transaction(uint8_t* buff, uint64_t start_sector, uint16_t num_of_sectors, bool write) {
  if(write) {
    write_to_disk(buff, start_sector, num_of_sectors);
  } else {
    read_from_disk(buff, start_sector, num_of_sectors);
  }
}
