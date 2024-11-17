#pragma once
#include <cstdint>

#define PAGE_SIZE 0x1000
#define GUEST_PHYSICAL_PAGE_FLAG 0x07

struct pagingLevel {
  uint64_t entries[512];
};

void mapPage(uint64_t physical_addr, uint64_t virt_addr, uint16_t flags, uint64_t cr3);
void switch_pageMap(uint64_t cr3);
void save_host_pageMap();
void create_linear_virtual_space(uint64_t size);
uint64_t create_clean_virtual_space();
void identity_map(uint64_t cr3, uint64_t start_addr, uint64_t num_of_pages, uint16_t flags);
