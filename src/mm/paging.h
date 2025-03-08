#pragma once

#include <cstdint>
enum paging_flags {
  PRESENT_FLAG,
  RW_FLAG,
  US_FLAG,
  WRITE_THROUGH_FLAG,
  CACHE_DIS_FLAG,
  ACCESSED_FLAG,
  DIRTY_FLAG,
  PAGE_SIZE_FLAG,
  GLOBAL_FLAG,
  AVAILABLE_FLAG,
  PAT_FLAG
};

void init_host_allocator();
void allocate_page(uint64_t* virt_addr, uint64_t* phys_addr, uint16_t flags);
