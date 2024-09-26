#pragma once
#include <cstdint>

#define GUEST_PHYSICAL_PAGE_FLAG 0x07

struct pagingLevel {
  uint64_t entries[512];
};

void mapPage(uint64_t physical_addr, uint64_t virt_addr, uint16_t flags, uint64_t cr3);
void switch_pageMap(uint64_t cr3);
void save_host_pageMap();
void create_linear_virtual_space(uint64_t size);
uint64_t create_clean_virtual_space();
