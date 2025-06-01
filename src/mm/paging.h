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

uint64_t init_vmm(struct limine_memmap_response* response, struct limine_file* kernel_file, struct limine_kernel_address_response* kernel_addr);
uint64_t init_mappings(uint64_t memmap);
void init_user_mappings(uint64_t memmap);
void allocate_page(uint64_t* virt_addr, uint64_t* phys_addr, uint16_t flags, uint64_t cr3);
void map_kernel(uint64_t cr3);
void map_host_page(uint64_t virt_addr, uint64_t phys_addr, uint16_t flags);
uint64_t walk_host_tables(uint64_t virt_addr);
uint64_t switch_to_host_pagemap();
