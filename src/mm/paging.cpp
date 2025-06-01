#include "paging.h"
#include "../common.h"
#include "../pmm.h"
#include "limine.h"
#include "npaging.h"
#include <cstdint>

extern "C" {
  extern const char elf_start_addr[];
  extern const char elf_end_addr[];
}



static uint64_t host_cr3 = 0;
static limine_file* kernel_file_ptr = 0x0;
static limine_kernel_address_response* kernel_addr_ptr = 0x0;
static limine_memmap_response* memmap_response_ptr = 0x0;
static uint64_t kernel_memory_size = 0;

void allocate_page(uint64_t *virt_addr, uint64_t *phys_addr, uint16_t flags, uint64_t cr3) {
  if(cr3 == 0) {
    cr3 = host_cr3;
  }

  *phys_addr = kpalloc();
  *virt_addr = TO_HIGHER_HALF(*phys_addr);
  mapPage(*phys_addr, TO_HIGHER_HALF(*phys_addr), flags, cr3);
}

void map_host_page(uint64_t virt_addr, uint64_t phys_addr, uint16_t flags) {
  mapPage(phys_addr, virt_addr, flags, host_cr3);
}

uint64_t walk_host_tables(uint64_t virt_addr) {
  return walkTable(virt_addr, host_cr3);
}

void map_kernel(uint64_t cr3) {
  if(kernel_file_ptr == 0x0 || kernel_addr_ptr == 0x0) {
    return;
  }

  map_range(cr3, (uint64_t)kernel_addr_ptr->virtual_base,
            (uint64_t)kernel_addr_ptr->physical_base, (kernel_memory_size / PAGE_SIZE) + 1,
            PRESENT_FLAG | RW_FLAG);
}

uint64_t init_vmm(struct limine_memmap_response *memmap_response,
                  struct limine_file *kernel_file, 
                  struct limine_kernel_address_response* kernel_addr) {
  kernel_memory_size = elf_end_addr - elf_start_addr;
  uint64_t memmap = create_clean_virtual_space();
  kernel_file_ptr = kernel_file;
  kernel_addr_ptr = kernel_addr;
  memmap_response_ptr = memmap_response;
  host_cr3 = init_mappings(memmap);
  // Also map rsdt
  switch_pageMap(host_cr3);
  return host_cr3;
}

uint64_t init_mappings(uint64_t memmap) {
  for (uint32_t entry_idx = 0; entry_idx < memmap_response_ptr->entry_count; entry_idx++) {

    struct limine_memmap_entry *entry = memmap_response_ptr->entries[entry_idx];
    map_range(
        memmap, TO_HIGHER_HALF(entry->base), entry->base,
        (entry->length / PAGE_SIZE) + 1,
        PRESENT_FLAG | RW_FLAG
    );

  }

  map_kernel(memmap);
  // map_range(memmap, 0x0, 0x0, 0x100000, 0x03);
  // map_range(memmap, TO_HIGHER_HALF(0), 0x0, 0x100000, 0x03);

  return memmap;
}

void init_user_mappings(uint64_t memmap) {
  kmemcpy((uint8_t*)(TO_HIGHER_HALF(memmap) + 256 * 8), (uint8_t*)(TO_HIGHER_HALF(host_cr3) + 256 * 8), 256 * 8); 
}

uint64_t switch_to_host_pagemap() {
  uint64_t prev_memmap = get_host_pageMap();
  switch_pageMap(host_cr3);
  return prev_memmap;
}
