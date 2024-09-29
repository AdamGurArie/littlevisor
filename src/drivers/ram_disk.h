#pragma once

#include <cstdint>

void init_ramdisk();
void read_from_disk(uint8_t* buff, uint64_t start_sector, uint16_t size);
void write_to_disk(uint8_t* buff, uint64_t start_sector, uint16_t size);
void commit_transaction(uint8_t* buff, uint64_t start_sector, uint16_t num_of_sectors, bool write);
