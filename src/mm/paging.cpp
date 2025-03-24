#include "npaging.h"
#include "../pmm.h"
#include "../common.h"
#include <cstdint>

static uint64_t host_cr3 = 0;


void init_host_allocator() {
  host_cr3 = get_host_pageMap();
}

void allocate_page(uint64_t* virt_addr, uint64_t* phys_addr, uint16_t flags) {
  *phys_addr = kpalloc();
  *virt_addr = TO_HIGHER_HALF(*phys_addr);
  mapPage(*phys_addr, TO_HIGHER_HALF(*phys_addr), flags, host_cr3);
}

void map_host_page(uint64_t virt_addr, uint64_t phys_addr, uint16_t flags) {
  mapPage(phys_addr, virt_addr, flags, host_cr3);
}

uint64_t walk_host_tables(uint64_t virt_addr) {
  return walkTable(virt_addr, host_cr3);
}
