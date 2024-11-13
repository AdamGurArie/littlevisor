#include "npaging.h"
#include "../common.h"
#include "../pmm.h"
#include <cstdint>
#include <bit>

static uint64_t cr3_val = 0;
static uint64_t host_cr3 = 0;

void mapPage(uint64_t phys_addr, uint64_t virt_addr, uint16_t flags, uint64_t cr3) {
  if(cr3 == 0) {
    cr3 = cr3_val;
  }

  uint8_t indexes[4] = {0};
  indexes[0] = (virt_addr >> 39) & 0x1FF;
  indexes[1] = (virt_addr >> 30) & 0x1FF;
  indexes[2] = (virt_addr >> 21) & 0x1FF;
  indexes[3] = (virt_addr >> 12) & 0x1FF;

  pagingLevel* table = std::bit_cast<pagingLevel*>(TO_HIGHER_HALF(cr3)); 
  
  for(int i = 0; i < 3; i++) {
    if((table->entries[indexes[i]] & 1) == 0) {
      uint64_t new_page = kpalloc();
      if(new_page == 0) {
        return;
      }
      
      kmemset((uint8_t*)TO_HIGHER_HALF(new_page), 0x0, 0x1000);
      table->entries[indexes[i]] = new_page | flags;
      table = std::bit_cast<pagingLevel*>(TO_HIGHER_HALF((table->entries[indexes[i]] & ~0xFFF)));
    } else {
      table = std::bit_cast<pagingLevel*>(TO_HIGHER_HALF((table->entries[indexes[i]] & ~0xFFF)));
    }
  }

  table->entries[indexes[3]] = phys_addr | flags;
}

void switch_pageMap(uint64_t cr3) {
  asm volatile("movq %%cr3, %0" :: "r"(cr3));
}

void create_linear_virtual_space(uint64_t size) {
  for(uint64_t i = 0; i < size; i+=0x1000) {
    uint64_t page = kpalloc();
    mapPage(page, i, GUEST_PHYSICAL_PAGE_FLAG, 0);
  }
}

void save_host_pageMap() {
  asm volatile("movq %0, %%cr3" : "=r"(host_cr3));
}

uint64_t create_clean_virtual_space() {
  return kpalloc();
}

void identity_map(uint64_t cr3, uint64_t start_addr, uint64_t num_of_pages, uint16_t flags) {
  for(int i = 0; i < num_of_pages; i++) {
    mapPage(
        start_addr + i * 0x1000,
        start_addr + i * 0x1000,
        flags,
        cr3
    );
  }
}
