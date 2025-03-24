#pragma once

#include <cstdint>
enum paging_flags {
  PRESENT_FLAG = 0x1,
  RW_FLAG = 0x2,
  US_FLAG = 0x4,
  WRITE_THROUGH_FLAG = 0x8,
  CACHE_DIS_FLAG = 0x10,
  ACCESSED_FLAG = 0x20,
  DIRTY_FLAG = 0x40,
  PAGE_SIZE_FLAG = 0x80,
  GLOBAL_FLAG = 0x100,
  AVAILABLE_FLAG = 0x200,
  PAT_FLAG = 0x400
};

void init_host_allocator();
void allocate_page(uint64_t* virt_addr, uint64_t* phys_addr, uint16_t flags);
void map_host_page(uint64_t virt_addr, uint64_t phys_addr, uint16_t flags);
uint64_t walk_host_tables(uint64_t virt_addr);
